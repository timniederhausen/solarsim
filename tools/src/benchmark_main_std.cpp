#include "benchmark_common.hpp"
#include "solarsim/sync_simulator.hpp"
#include "solarsim/stdexec/async_simulator_sender.hpp"

#include <solarsim/simulation_state.hpp>

#include <exec/static_thread_pool.hpp>

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
static void BM_BH_MT_STDSenders(benchmark::State& state)
{
  using namespace solarsim::impl_std;

  const solarsim::real duration = FLAGS_duration * (S == Scaling::Weak ? state.range(0) : 1.0);
  auto data                     = solarsim::get_problem();

  // Create a thread pool and get a scheduler from it
  exec::static_thread_pool pool(state.range(0));
  ex::scheduler auto sched = pool.get_scheduler();

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

  pool.request_stop();
}

extern "C" int main(int argc, char* argv[])
{
  benchmark::Initialize(&argc, argv, []() {
    benchmark::PrintDefaultHelp();
    gflags::SetUsageMessage("see above");
    gflags::ShowUsageWithFlags("SolarSim_benchmark");
  });

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;

#define SOLARSIM_BENCHMARK(function_name) register_solarsim_benchmark(#function_name, &function_name)

  // strong scaling first
  SOLARSIM_BENCHMARK(BM_BH_MT_STDSenders<Scaling::Strong>);

  // then weak scaling
  SOLARSIM_BENCHMARK(BM_BH_MT_STDSenders<Scaling::Weak>);

#undef SOLARSIM_BENCHMARK

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}

SOLARSIM_NS_END
