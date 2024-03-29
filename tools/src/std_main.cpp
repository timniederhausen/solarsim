#include <iostream>
#include <solarsim/types.hpp>
#include <solarsim/body_definition_csv.hpp>
#include <solarsim/naive_simulator_sync.hpp>
#include <solarsim/math.hpp>

#include <span>

SOLARSIM_NS_BEGIN

inline constexpr bool use_common_gravity_constant = false;

void adjust_initial_velocities(std::span<body_definition> bodies)
{
  std::vector<triple> u_i(bodies.size());

  for (std::size_t i = 0, n = bodies.size(); i != n; ++i) {
    // Obtain a_i by summing all pairwise acceleration values (i \ne j)
    triple adjustment = {};
    real mass_sum     = 0;
    for (std::size_t j = 0; j != n; ++j) {
      adjustment[0] += bodies[j].mass * bodies[j].velocity[0];
      adjustment[1] += bodies[j].mass * bodies[j].velocity[1];
      adjustment[2] += bodies[j].mass * bodies[j].velocity[2];
      mass_sum += bodies[j].mass;
    }
    adjustment[0] /= mass_sum;
    adjustment[1] /= mass_sum;
    adjustment[2] /= mass_sum;
    u_i[i] = adjustment;
  }

  for (std::size_t i = 0, n = bodies.size(); i != n; ++i) {
    bodies[i].velocity[0] -= u_i[i][0];
    bodies[i].velocity[1] -= u_i[i][1];
    bodies[i].velocity[2] -= u_i[i][2];
  }
}

// Our input files have the following units:
// kg for mass, pc / year for velocity and pc for positions
// Depending on the units we want to use, some conversion becomes necessary.

void normalize_body_values(body_definition& body)
{
  if (use_common_gravity_constant) {
    body.mass = body.mass * gravitational_constant_common;

    // pc => m
    body.position[0] *= parsec_in_m;
    body.position[1] *= parsec_in_m;
    body.position[2] *= parsec_in_m;

    // pc / y => m / s
    body.velocity[0] *= parsec_in_m / year_in_seconds;
    body.velocity[1] *= parsec_in_m / year_in_seconds;
    body.velocity[2] *= parsec_in_m / year_in_seconds;
  } else {
    body.mass = body.mass / solar_mass_in_kg * gravitational_constant;

    // pc => km
    body.position[0] *= parsec_in_km;
    body.position[1] *= parsec_in_km;
    body.position[2] *= parsec_in_km;

    // pc / y => km / s
    body.velocity[0] *= parsec_in_km / year_in_seconds;
    body.velocity[1] *= parsec_in_km / year_in_seconds;
    body.velocity[2] *= parsec_in_km / year_in_seconds;
  }
}

void denormalize_body_values(body_definition& body)
{
  if (use_common_gravity_constant) {
    body.mass = body.mass / gravitational_constant_common;

    // m => pc
    body.position[0] /= parsec_in_m;
    body.position[1] /= parsec_in_m;
    body.position[2] /= parsec_in_m;

    // m / s => pc / y
    body.velocity[0] /= parsec_in_m * year_in_seconds;
    body.velocity[1] /= parsec_in_m * year_in_seconds;
    body.velocity[2] /= parsec_in_m * year_in_seconds;
  } else {
    body.mass = body.mass * solar_mass_in_kg / gravitational_constant;

    // km => pc
    body.position[0] /= parsec_in_km;
    body.position[1] /= parsec_in_km;
    body.position[2] /= parsec_in_km;

    // km / s => pc / y
    body.velocity[0] /= parsec_in_km * year_in_seconds;
    body.velocity[1] /= parsec_in_km * year_in_seconds;
    body.velocity[2] /= parsec_in_km * year_in_seconds;
  }
}

/*
void normalize_body_values_1970(body_definition& body)
{
  body.mass = body.mass / solar_mass_in_kg;
  body.position[0] /= parsec_in_m;
  body.position[1] /= parsec_in_m;
  body.position[2] /= parsec_in_m;

  body.velocity[0] *= 1000;
  body.velocity[1] *= 1000;
  body.velocity[2] *= 1000;
}

void denormalize_body_values_1970(body_definition& body)
{
  body.mass = body.mass * solar_mass_in_kg;
  body.position[0] *= parsec_in_m;
  body.position[1] *= parsec_in_m;
  body.position[2] *= parsec_in_m;

  body.velocity[0] /= 1000;
  body.velocity[1] /= 1000;
  body.velocity[2] /= 1000;
}
*/
SOLARSIM_NS_END

void run_for_file(const std::string& filename, bool need_norm)
{
  using namespace solarsim;

  auto dataset = load_from_csv_file(("dataset/" + filename).c_str());

  if (need_norm) {
    for (auto& body : dataset)
      normalize_body_values(body);
  }
  adjust_initial_velocities(dataset);
  save_to_csv_file(dataset, ("dataset_debug/" + filename).c_str());

  naive_simulator_sync simulator(dataset, .05);
  run_simulation(simulator, 60 * 60, year_in_seconds);

  if (need_norm) {
    for (auto& body : dataset)
      denormalize_body_values(body);
  }
  save_to_csv_file(dataset, ("dataset_result/" + filename).c_str());
}

extern "C" int main(int argc, const char* argv[])
{
  try {
    // run_for_file("sol_1970_state_vectors.csv", false);
    run_for_file("planets_and_moons_state_vectors.csv", true);
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
