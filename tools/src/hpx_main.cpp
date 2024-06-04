#include "dataset_conversion.hpp"

#include <solarsim/hpx/async_simulator.hpp>
#include <solarsim/hpx/async_simulator_sender.hpp>
#include <solarsim/body_definition_csv.hpp>

#include <hpx/algorithm.hpp>
#include <hpx/execution.hpp>
#include <hpx/iostream.hpp>

// Including 'hpx/hpx_main.hpp' instead of the usual 'hpx/hpx_init.hpp' enables
// to use the plain C-main below as the direct main HPX entry point.
#include <hpx/hpx_main.hpp>

#include <fmt/format.h>

#include <cassert>

SOLARSIM_NS_BEGIN

void test_hpx_execution_ops()
{
  const auto [v] = (ex::just(1010) | tt::sync_wait()).value(); // NOLINT(bugprone-unchecked-optional-access)
  assert(v == 1010);
}

void run_with_senders(const simulation_state_view& state)
{
  // Could use different executors here
  ex::thread_pool_scheduler exec{};

  constexpr real time_step = 60 * 60;
  for (real elapsed = time_step; elapsed < year_in_seconds; elapsed += time_step) {
    // Very basic way of chaining these algorithms together to end up with:
    // [parallel] integration step phase 1
    // <barnes hut or naive acceleration update>
    // [parallel] integration step phase 2

    auto snd = ex::transfer_just(exec, state) |                                   //
               async_tick_simulation_phase1(get_dataset_size(state), time_step) | //
               async_tick_barnes_hut(exec) |                                          //
               async_tick_simulation_phase2(get_dataset_size(state), time_step);

    tt::sync_wait(std::move(snd)); // wait on this thread to finish
  }
}

void run_with_parallel_algorithms(const simulation_state_view& state)
{
  // We have quite a few executors to choose from:
  hpx::execution::parallel_executor par;
  hpx::execution::parallel_executor par_nostack(hpx::threads::thread_priority::default_,
                                                hpx::threads::thread_stacksize::nostack);
  hpx::execution::experimental::scheduler_executor<hpx::execution::experimental::thread_pool_scheduler> sched_exec_tps;

  constexpr real time_step = 60 * 60;
  for (real elapsed = time_step; elapsed < year_in_seconds; elapsed += time_step) {
    // Task-based parallel execution on our chosen Executor.
    auto our_policy = hpx::execution::par(hpx::execution::task).on(sched_exec_tps);
    auto future1    = impl_hpx::tick_simulation_phase1(our_policy, state, time_step);
    auto future2    = future1.then([=](hpx::future<void>) {
      barnes_hut_sync_simulator_impl().tick(state.body_positions, state.body_masses, state.softening_factor,
                                               state.acceleration);
    });
    auto future3    = future2.then([=](hpx::future<void>) {
      return impl_hpx::tick_simulation_phase2(our_policy, state, time_step);
    });
    future3.get(); // wait on this thread to finish
  }
}

template <bool UseBarnesHut, typename DatasetPolicy>
void run_for_file(const std::string& filename, auto&& impl, bool need_norm)
{
  fmt::print("Running on {} using {} {} normalization\n", filename, UseBarnesHut ? "barnes-hut" : "naive-sim",
             need_norm ? "with" : "without");

  const auto output_filename = fmt::format("{}{}", UseBarnesHut ? "BH_" : "", filename);
  auto dataset               = load_from_csv_file("dataset/" + filename);

  if (need_norm) {
    for (auto& body : dataset)
      DatasetPolicy::normalize_body_values(body);
  }
  adjust_initial_velocities(dataset);
  save_to_csv_file(dataset, "dataset_debug/" + output_filename);

  // Next, decompose the bodies into what we need!
  // Our algorithms are decoupled from the body_definition type.
  std::vector<triple> body_positions(dataset.size());
  std::vector<triple> body_velocities(dataset.size());
  std::vector<real> body_masses(dataset.size());
  for (std::size_t i = 0, n = dataset.size(); i != n; ++i) {
    body_positions[i]  = dataset[i].position;
    body_velocities[i] = dataset[i].velocity;
    body_masses[i]     = dataset[i].mass;
  }

  // per-step acceleration
  std::vector<triple> acceleration(dataset.size());

  simulation_state_view state;
  state.body_positions   = body_positions;
  state.body_velocities  = body_velocities;
  state.body_masses      = body_masses;
  state.acceleration     = acceleration;
  state.softening_factor = .05;
  impl(state);

  for (std::size_t i = 0, n = dataset.size(); i != n; ++i) {
    dataset[i].position = body_positions[i];
    dataset[i].velocity = body_velocities[i];
    dataset[i].mass     = body_masses[i];
  }

  if (need_norm) {
    for (auto& body : dataset)
      DatasetPolicy::denormalize_body_values(body);
  }
  save_to_csv_file(dataset, "dataset_result/" + output_filename);
}

SOLARSIM_NS_END

int main()
{
  solarsim::test_hpx_execution_ops();
  try {
    // vectors from the internet
    // solarsim::run_for_file("sol_1970_state_vectors.csv", false);

    // simple test vectors from the institute
    solarsim::run_for_file<false, solarsim::ipvs_dataset>("planets_and_moons_state_vectors.csv",
                                                          solarsim::run_with_senders, /*need_norm=*/true);
    solarsim::run_for_file<true, solarsim::ipvs_dataset>("planets_and_moons_state_vectors.csv",
                                                         solarsim::run_with_parallel_algorithms, /*need_norm=*/true);
  } catch (std::exception& e) {
    fmt::print("std::exception caught: {}\n", e.what());
  }
  return 0;
}
