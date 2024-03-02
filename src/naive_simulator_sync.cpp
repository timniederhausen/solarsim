/// @copyright Copyright (c) Tim Niederhausen (tim@rnc-ag.de)
///
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include "solarsim/naive_simulator_sync.hpp"
#include "solarsim/math.hpp"

#include <cassert>

SOLARSIM_NS_BEGIN

// TODO: Add a template argument for this?
inline constexpr bool use_shifted_verlet = false;

naive_simulator_sync::naive_simulator_sync(std::span<body_definition> bodies, real softening_factor)
  : bodies_(bodies)
  , softening_factor_(softening_factor)
  , acceleration_(bodies_.size())
{
  if constexpr (!use_shifted_verlet)
    update_acceleration();
}

void naive_simulator_sync::tick(real dT)
{
  if constexpr (use_shifted_verlet) {
    for (std::size_t i = 0, n = bodies_.size(); i != n; ++i)
      integrate_leapfrog_phase1(bodies_[i].position, bodies_[i].velocity, dT);

    update_acceleration();

    for (std::size_t i = 0, n = bodies_.size(); i != n; ++i)
      integrate_leapfrog_phase2(bodies_[i].position, bodies_[i].velocity, acceleration_[i], dT);
  } else {
    // needs previous acceleration!
    for (std::size_t i = 0, n = bodies_.size(); i != n; ++i)
      integrate_velocity_verlet_phase1(bodies_[i].position, bodies_[i].velocity, acceleration_[i], dT);

    update_acceleration();

    for (std::size_t i = 0, n = bodies_.size(); i != n; ++i)
      integrate_velocity_verlet_phase2(bodies_[i].velocity, acceleration_[i], dT);
  }
}

void naive_simulator_sync::update_acceleration()
{
  for (std::size_t i = 0, n = bodies_.size(); i != n; ++i) {
    // Obtain a_i by summing all pairwise acceleration values (i \ne j)
    triple acceleration_sum = {};
    for (std::size_t j = 0; j != n; ++j) {
      if (i != j) {
        calculate_acceleration(bodies_[i].position, bodies_[j].position, bodies_[j].mass, softening_factor_,
                               acceleration_sum);
      }
    }
    debug_validate_acceleration(acceleration_sum);
    acceleration_[i] = acceleration_sum;
  }
}

void run_simulation(naive_simulator_sync& simulator, real time_step, real duration)
{
  assert(time_step <= duration);

  // Start simulating at |time_step|
  for (real elapsed = time_step; elapsed < duration;) {
    simulator.tick(time_step);
    elapsed += time_step;
  }
}

SOLARSIM_NS_END
