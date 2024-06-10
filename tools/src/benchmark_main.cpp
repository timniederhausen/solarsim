#include "dataset_conversion.hpp"
#include "solarsim/sync_simulator.hpp"
#include "solarsim/hpx/async_simulator.hpp"
#include "solarsim/hpx/async_simulator_sender.hpp"

#include <solarsim/body_definition_csv.hpp>
#include <solarsim/simulation_state.hpp>

#include <hpx/algorithm.hpp>
#include <hpx/execution.hpp>
#include <hpx/init.hpp>

#include <benchmark/benchmark.h>

SOLARSIM_NS_BEGIN

// Helper type to own our simulation dataset
template <typename DatasetPolicy>
struct benchmark_simulator_data
{
  benchmark_simulator_data(const std::string& filename, bool need_norm)
  {
    auto dataset = load_from_csv_file("dataset/" + filename);

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

const simulation_state& load_benchmark_data()
{
  // <static const> gives us "free" on-demand thread safe init for our static dataset
  // Dataset selection:
  // static const benchmark_simulator_data<ipvs_dataset> d("planets_and_moons_state_vectors.csv", true);
  static const benchmark_simulator_data<ipvs_dataset> d("scenario1_state_vectors.csv", true);
  return d.state;
}

SOLARSIM_NS_END

inline constexpr solarsim::real time_step = 60 * 60;
inline constexpr solarsim::real duration  = time_step * 15;
// yeah, no way on this computer!
// inline constexpr solarsim::real duration  = solarsim::year_in_seconds;

//
// Benchmark harnesses
//

static void BM_Naive_ST(benchmark::State& state)
{
  auto data = solarsim::load_benchmark_data();
  for (auto _ : state) {
    solarsim::naive_sync_simulator simulator(data.body_positions, data.body_velocities, data.body_masses, .05);
    run_simulation(simulator, time_step, duration);
  }
}
// BENCHMARK(BM_Naive_ST);

static void BM_BH_ST(benchmark::State& state)
{
  auto data = solarsim::load_benchmark_data();
  for (auto _ : state) {
    solarsim::barnes_hut_sync_simulator simulator(data.body_positions, data.body_velocities, data.body_masses, .05);
    run_simulation(simulator, time_step, duration);
  }
}
BENCHMARK(BM_BH_ST);

enum class Scaling
{
  // i.e. fixed work per thread
  Weak,
  // i.e. fixed work over all threads
  Strong
};

template <Scaling S>
inline solarsim::simulation_state get_data_for(std::size_t threads = 1)
{
  (void)threads;
  switch (S) {
    case Scaling::Strong: return solarsim::load_benchmark_data();
    case Scaling::Weak:   // TODO
    default:              throw std::runtime_error("not implemented");
  }
}

template <Scaling S>
static void BM_BH_MT_HPXSenders(benchmark::State& state)
{
  hpx::execution::experimental::thread_pool_scheduler sched;

  auto data = get_data_for<S>(state.range(0));
  for (auto _ : state) {
    for (solarsim::real elapsed = time_step; elapsed < duration; elapsed += time_step) {
      // Very basic way of chaining these algorithms together to end up with:
      // [parallel] integration step phase 1
      // <barnes hut or naive acceleration update>
      // [parallel] integration step phase 2

      auto snd = solarsim::ex::transfer_just(sched, solarsim::simulation_state_view(data)) |           //
                 solarsim::async_tick_simulation_phase1(solarsim::get_dataset_size(data), time_step) | //
                 solarsim::async_tick_barnes_hut(sched) |                                                   //
                 solarsim::async_tick_simulation_phase2(solarsim::get_dataset_size(data), time_step);

      static_assert(solarsim::ex::is_sender_v<decltype(snd)>);
      // std::cerr << "DOING BM_BH_MT_HPXSenders" << std::endl;
      solarsim::tt::sync_wait(std::move(snd)); // wait on this thread to finish
    }
  }
}

template <Scaling S>
static void BM_BH_MT_HPXFutures(benchmark::State& state)
{
  hpx::execution::experimental::scheduler_executor<hpx::execution::experimental::thread_pool_scheduler> exec;

  auto data = get_data_for<S>(state.range(0));
  for (auto _ : state) {
    auto view = solarsim::simulation_state_view(data);
    for (solarsim::real elapsed = time_step; elapsed < duration; elapsed += time_step) {
      // Task-based parallel execution on our chosen Executor.
      auto our_policy = hpx::execution::par(hpx::execution::task).on(exec);
      auto future1    = solarsim::impl_hpx::tick_simulation_phase1(our_policy, view, time_step);
      auto future2    = future1.then([=](hpx::future<void>) {
        return solarsim::impl_hpx::tick_barnes_hut(our_policy, view);
      });
      auto future3    = future2.then([=](hpx::future<void>) {
        return solarsim::impl_hpx::tick_simulation_phase2(our_policy, view, time_step);
      });
      // std::cerr << "DOING BM_BH_MT_HPXFutures" << std::endl;
      future3.get(); // wait on this thread to finish
    }
  }
}

// XXX: All other ways to specify that at configure time are worse... :(
#define SETUP_MT_BENCHMARK ->Arg(2)->Arg(4)->Arg(8)->Arg(16)->Arg(32)->MeasureProcessCPUTime()->UseRealTime();

// strong scaling first
BENCHMARK(BM_BH_MT_HPXSenders<Scaling::Strong>) SETUP_MT_BENCHMARK;
BENCHMARK(BM_BH_MT_HPXFutures<Scaling::Strong>) SETUP_MT_BENCHMARK;

// then weak scaling
/*BENCHMARK(BM_BH_MT_HPXFutures<Scaling::Weak>) SETUP_MT_BENCHMARK;
BENCHMARK(BM_BH_MT_HPXSenders<Scaling::Weak>) SETUP_MT_BENCHMARK;*/

#undef SETUP_MT_BENCHMARK

int hpx_main(int argc, char** argv)
{
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();
  ::benchmark::Shutdown();
  return hpx::finalize();
}

extern "C" int main(int argc, char* argv[])
{
  ::benchmark::Initialize(&argc, argv);

  const std::vector<std::string> cfg = {
      // "all" for HT cores as well
      "hpx.os_threads=cores",

      // Needed if we wanted to initialize Google benchmark later
      //"hpx.commandline.allow_unknown=1",
      //"hpx.commandline.aliasing=0"
  };
  hpx::local::init_params init_args;
  init_args.cfg = cfg;
  return hpx::local::init(&hpx_main, argc, argv, init_args);
}
