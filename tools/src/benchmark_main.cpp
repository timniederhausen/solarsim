#include "dataset_conversion.hpp"
#include "solarsim/sync_simulator.hpp"
#include "solarsim/hpx/async_simulator.hpp"
#include "solarsim/hpx/async_simulator_sender.hpp"

#include <solarsim/body_definition_csv.hpp>
#include <solarsim/simulation_state.hpp>

#include <hpx/algorithm.hpp>
#include <hpx/execution.hpp>
#include <hpx/init.hpp>

// Enable optional spirit debugging
// #define BOOST_SPIRIT_DEBUG
#include <boost/spirit/home/x3.hpp>

#include <benchmark/benchmark.h>
#include <gflags/gflags.h>

inline constexpr boost::spirit::x3::rule<class number_list_tag, std::vector<int>> thread_list = "thread_list";
inline constexpr auto thread_list_def = -(boost::spirit::x3::int_ % ',');
BOOST_SPIRIT_DEFINE(thread_list);

static std::vector<int> threads_to_benchmark;
bool parse_threads(const char* flagname, const std::string& value)
{
  (void)flagname;
  auto iter = value.begin();
  return phrase_parse(iter, value.end(), thread_list_def, boost::spirit::x3::space, threads_to_benchmark);
}

DEFINE_double(time_step, 60 * 60, "Time between simulation steps (in s)");
DEFINE_double(duration, (60 * 60) * 15, "Total duration of the simulation (in s)");
DEFINE_string(dataset, "dataset/scenario1_state_vectors.csv", "Path to the input state vectors");
DEFINE_string(threads, "2,4,8,16", "Number of threads to test");
DEFINE_validator(threads, &parse_threads);

SOLARSIM_NS_BEGIN

// Helper type to own our simulation dataset
template <typename DatasetPolicy>
struct benchmark_simulator_data
{
  benchmark_simulator_data(const std::string& filename, bool need_norm)
  {
    auto dataset = load_from_csv_file(filename);

    if (need_norm) {
      for (auto& body : dataset)
        DatasetPolicy::normalize_body_values(body);
    }
    adjust_initial_velocities(dataset);

    // Next, decompose the bodies into what we need!
    // Our algorithms are decoupled from the body_definition type.
    state.body_positions.resize(dataset.size());
    state.body_velocities.resize(dataset.size());
    state.body_masses.resize(dataset.size());
    state.acceleration.resize(dataset.size());
    state.softening_factor = .05;

    for (std::size_t i = 0, n = dataset.size(); i != n; ++i) {
      state.body_positions[i]  = dataset[i].position;
      state.body_velocities[i] = dataset[i].velocity;
      state.body_masses[i]     = dataset[i].mass;
    }
  }

  simulation_state state;
};

static const simulation_state& get_problem()
{
  // <static const> gives us "free" on-demand thread safe init for our static dataset
  // Dataset selection:
  // static const benchmark_simulator_data<ipvs_dataset> d("planets_and_moons_state_vectors.csv", true);
  static const benchmark_simulator_data<ipvs_dataset> d(FLAGS_dataset, true);
  return d.state;
}

SOLARSIM_NS_END

//
// Benchmark harnesses
//

// Singlethreaded:
static void BM_Naive_ST(benchmark::State& state)
{
  auto data = solarsim::get_problem();
  for (auto _ : state) {
    solarsim::naive_sync_simulator simulator(data.body_positions, data.body_velocities, data.body_masses, .05);
    run_simulation(simulator, FLAGS_time_step, FLAGS_duration);
  }
}
BENCHMARK(BM_Naive_ST);

static void BM_BH_ST(benchmark::State& state)
{
  auto data = solarsim::get_problem();
  for (auto _ : state) {
    solarsim::barnes_hut_sync_simulator simulator(data.body_positions, data.body_velocities, data.body_masses, .05);
    run_simulation(simulator, FLAGS_time_step, FLAGS_duration);
  }
}
BENCHMARK(BM_BH_ST);

// Multithreaded:
enum class Scaling
{
  // i.e. fixed work per thread
  Weak,
  // i.e. fixed work over all threads
  Strong
};

template <Scaling S>
static void BM_BH_MT_HPXSenders(benchmark::State& state)
{
  const solarsim::real duration = FLAGS_duration * (S == Scaling::Weak ? state.range(0) : 1.0);
  auto data = solarsim::get_problem();
  
  hpx::execution::experimental::thread_pool_scheduler sched;
  for (auto _ : state) {
    for (solarsim::real elapsed = FLAGS_time_step; elapsed < duration; elapsed += FLAGS_time_step) {
      // Very basic way of chaining these algorithms together to end up with:
      // [parallel] integration step phase 1
      // <barnes hut or naive acceleration update>
      // [parallel] integration step phase 2

      auto snd = solarsim::ex::transfer_just(sched, solarsim::simulation_state_view(data)) |                 //
                 solarsim::async_tick_simulation_phase1(solarsim::get_dataset_size(data), FLAGS_time_step) | //
                 solarsim::async_tick_barnes_hut(sched) |                                                    //
                 solarsim::async_tick_simulation_phase2(solarsim::get_dataset_size(data), FLAGS_time_step);

      solarsim::tt::sync_wait(std::move(snd)); // wait on this thread to finish
    }
  }
}

template <Scaling S>
static void BM_BH_MT_HPXFutures(benchmark::State& state)
{
  const solarsim::real duration = FLAGS_duration * (S == Scaling::Weak ? state.range(0) : 1.0);
  auto data = solarsim::get_problem();
  
  hpx::execution::experimental::scheduler_executor<hpx::execution::experimental::thread_pool_scheduler> exec;
  for (auto _ : state) {
    auto view = solarsim::simulation_state_view(data);
    for (solarsim::real elapsed = FLAGS_time_step; elapsed < duration; elapsed += FLAGS_time_step) {
      // Task-based parallel execution on our chosen Executor.
      auto our_policy = hpx::execution::par(hpx::execution::task).on(exec);
      auto future1    = solarsim::impl_hpx::tick_simulation_phase1(our_policy, view, FLAGS_time_step);
      auto future2    = future1.then([=](hpx::future<void>) {
        return solarsim::impl_hpx::tick_barnes_hut(our_policy, view);
      });
      auto future3    = future2.then([=](hpx::future<void>) {
        return solarsim::impl_hpx::tick_simulation_phase2(our_policy, view, FLAGS_time_step);
      });
      future3.get(); // wait on this thread to finish
    }
  }
}

using benchmark_function_type = void(benchmark::State&);

void register_solarsim_benchmark(const std::string& name, benchmark_function_type function)
{
  const auto registration = benchmark::RegisterBenchmark(name, function)->UseRealTime()->MeasureProcessCPUTime();
  for (auto num_threads : threads_to_benchmark)
    registration->Arg(num_threads);
}

int hpx_main(int argc, char** argv)
{
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;

#define SOLARSIM_BENCHMARK(function_name) register_solarsim_benchmark(#function_name, &function_name)

  // strong scaling first
  SOLARSIM_BENCHMARK(BM_BH_MT_HPXSenders<Scaling::Strong>);
  SOLARSIM_BENCHMARK(BM_BH_MT_HPXSenders<Scaling::Strong>);

  // then weak scaling
  SOLARSIM_BENCHMARK(BM_BH_MT_HPXFutures<Scaling::Weak>);
  SOLARSIM_BENCHMARK(BM_BH_MT_HPXSenders<Scaling::Weak>);

#undef SOLARSIM_BENCHMARK

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return hpx::finalize();
}

extern "C" int main(int argc, char* argv[])
{
  benchmark::Initialize(&argc, argv, []() {
    benchmark::PrintDefaultHelp();
    gflags::SetUsageMessage("see above");
    gflags::ShowUsageWithFlags("SolarSim_benchmark");
  });

  const std::vector<std::string> cfg = {// "all" for HT cores as well
                                        "hpx.os_threads=cores",

                                        // Needed if we wanted to initialize Google benchmark later
                                        "hpx.commandline.allow_unknown=1", "hpx.commandline.aliasing=0"};
  hpx::local::init_params init_args;
  init_args.cfg = cfg;
  return hpx::local::init(&hpx_main, argc, argv, init_args);
}
