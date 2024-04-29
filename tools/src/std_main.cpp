#include "dataset_conversion.hpp"

#include <solarsim/types.hpp>
#include <solarsim/body_definition_csv.hpp>
#include <solarsim/sync_simulator.hpp>
#include <solarsim/math.hpp>

#include <fmt/format.h>

#include <span>

SOLARSIM_NS_BEGIN

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

  if constexpr (UseBarnesHut) {
    barnes_hut_sync_simulator simulator(body_positions, body_velocities, body_masses, .05);
    run_simulation(simulator, 60 * 60, year_in_seconds);
  } else {
    naive_sync_simulator simulator(body_positions, body_velocities, body_masses, .05);
    run_simulation(simulator, 60 * 60, year_in_seconds);
  }

  if (need_norm) {
    for (auto& body : dataset)
      DatasetPolicy::denormalize_body_values(body);
  }
  save_to_csv_file(dataset, "dataset_result/" + output_filename);
}

SOLARSIM_NS_END

extern "C" int main(int argc, const char* argv[])
{
  (void)argc;
  (void)argv;
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
