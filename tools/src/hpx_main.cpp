#include "dataset_conversion.hpp"

#include <solarsim/hpx/async_simulator.hpp>
#include <solarsim/body_definition_csv.hpp>

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

template <bool UseBarnesHut, typename DatasetPolicy>
void run_for_file(const std::string& filename, bool need_norm)
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

  // Could use different executors here
  ex::thread_pool_scheduler exec{};

  constexpr real time_step = 60 * 60;
  for (real elapsed = time_step; elapsed < year_in_seconds; elapsed += time_step) {
    simulation_state_view s;
    s.body_positions  = body_positions;
    s.body_velocities = body_velocities;
    s.body_masses     = body_masses;
    s.acceleration    = acceleration;

    // Very basic way of chaining these algorithms together to end up with:
    // [parallel] integration step phase 1
    // <barnes hut or naive acceleration update>
    // [parallel] integration step phase 2

    // TODO: make these easier to use
    tt::sync_wait(ex::transfer_just(exec, s) |                              //
                  async_tick_simulation_phase1(dataset.size(), time_step) | //
                  async_tick_barnes_hut() |                                 //
                  async_tick_simulation_phase2(dataset.size(), time_step));
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

    // vectors from the institute
    // solarsim::run_for_file<false, solarsim::ipvs_dataset>("planets_and_moons_state_vectors.csv", /*need_norm=*/true);
    solarsim::run_for_file<true, solarsim::ipvs_dataset>("planets_and_moons_state_vectors.csv", /*need_norm=*/true);
  } catch (std::exception& e) {
    fmt::print("std::exception caught: {}\n", e.what());
  }
  return 0;
}
