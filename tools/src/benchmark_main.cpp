#include "benchmark_common.hpp"
#include "solarsim/sync_simulator.hpp"
#include "solarsim/hpx/async_simulator.hpp"
#include "solarsim/hpx/async_simulator_sender.hpp"

#include <solarsim/simulation_state.hpp>

#include <hpx/algorithm.hpp>
#include <hpx/execution.hpp>
#include <hpx/init.hpp>

#include <benchmark/benchmark.h>

SOLARSIM_NS_BEGIN

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

template <Scaling S>
static void BM_BH_MT_HPXSenders(benchmark::State& state)
{
  using namespace solarsim::impl_hpx;

  const solarsim::real duration = FLAGS_duration * (S == Scaling::Weak ? state.range(0) : 1.0);
  auto data                     = solarsim::get_problem();

  auto sched = hpx::parallel::execution::with_processing_units_count(
      hpx::execution::experimental::thread_pool_scheduler{}, state.range(0));

  for (auto _ : state) {
    for (solarsim::real elapsed = FLAGS_time_step; elapsed < duration; elapsed += FLAGS_time_step) {
      // Very basic way of chaining these algorithms together to end up with:
      // [parallel] integration step phase 1
      // <barnes hut or naive acceleration update>
      // [parallel] integration step phase 2

      auto snd = ex::transfer_just(sched, solarsim::simulation_state_view(data)) |                 //
                 async_tick_simulation_phase1(solarsim::get_dataset_size(data), FLAGS_time_step) | //
                 async_tick_barnes_hut(sched) |                                                    //
                 async_tick_simulation_phase2(solarsim::get_dataset_size(data), FLAGS_time_step);

      tt::sync_wait(std::move(snd)); // wait on this thread to finish
    }
  }
}

template <Scaling S>
static void BM_BH_MT_HPXFutures(benchmark::State& state)
{
  using namespace solarsim::impl_hpx;

  const solarsim::real duration = FLAGS_duration * (S == Scaling::Weak ? state.range(0) : 1.0);
  auto data                     = solarsim::get_problem();

  auto exec = hpx::parallel::execution::with_processing_units_count(
      hpx::execution::experimental::scheduler_executor<hpx::execution::experimental::thread_pool_scheduler>{},
      state.range(0));

  for (auto _ : state) {
    auto view = solarsim::simulation_state_view(data);
    for (solarsim::real elapsed = FLAGS_time_step; elapsed < duration; elapsed += FLAGS_time_step) {
      // Task-based parallel execution on our chosen Executor.
      auto our_policy = hpx::execution::par(hpx::execution::task).on(exec);
      auto future1    = tick_simulation_phase1(our_policy, view, FLAGS_time_step);
      auto future2    = future1.then([=](hpx::future<void>) {
        return tick_barnes_hut(our_policy, view);
      });
      auto future3    = future2.then([=](hpx::future<void>) {
        return tick_simulation_phase2(our_policy, view, FLAGS_time_step);
      });
      future3.get(); // wait on this thread to finish
    }
  }
}

int hpx_main(int argc, char** argv)
{
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;

#define SOLARSIM_BENCHMARK(function_name) register_solarsim_benchmark(#function_name, &function_name)

  // strong scaling first
  SOLARSIM_BENCHMARK(BM_BH_MT_HPXFutures<Scaling::Strong>);
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

  const std::vector<std::string> cfg = {
      // "all" for HT cores as well
      "hpx.os_threads=cores",

      // Needed if we want to parse arguments later.
      "hpx.commandline.allow_unknown=1",
      "hpx.commandline.aliasing=0",
  };
  hpx::local::init_params init_args;
  init_args.cfg = cfg;
  return hpx::local::init(&hpx_main, argc, argv, init_args);
}

SOLARSIM_NS_END
