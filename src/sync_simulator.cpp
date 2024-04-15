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
#include "solarsim/sync_simulator.hpp"

#include "solarsim/barnes_hut_octree.hpp"
#include "solarsim/math.hpp"

SOLARSIM_NS_BEGIN

void naive_sync_simulator_impl::tick(std::span<const body_definition> bodies, real softening_factor,
                                     std::span<triple> acceleration) const
{
  // TODO: Add a template argument for this?
  constexpr bool use_fused_acceleration_calculation = true;

  std::fill(acceleration.begin(), acceleration.end(), triple{});

  for (std::size_t i = 0, n = bodies.size(); i != n; ++i) {
    // Obtain a_i by summing all pairwise acceleration values (i \ne j)
    if constexpr (use_fused_acceleration_calculation) {
      // do it for (i, j) & (j, i) at the same time
      for (std::size_t j = i + 1; j < n; ++j) {
        calculate_acceleration(bodies[i].position, bodies[j].position, bodies[i].mass, bodies[j].mass, softening_factor,
                               acceleration[i], acceleration[j]);
      }
    } else {
      // very naive version, calculate it per body
      for (std::size_t j = 0; j != n; ++j) {
        if (i != j) {
          calculate_acceleration(bodies[i].position, bodies[j].position, bodies[j].mass, softening_factor,
                                 acceleration[i]);
        }
      }
    }
  }
}

void barnes_hut_sync_simulator_impl::tick(std::span<const body_definition> bodies, real softening_factor,
                                          std::span<triple> acceleration) const
{
  std::fill(acceleration.begin(), acceleration.end(), triple{});
  barnes_hut_octree octree(bodies);
  for (std::size_t i = 0, n = bodies.size(); i != n; ++i) {
    octree.apply_forces_to(bodies[i], softening_factor, acceleration[i]);
  }
}

SOLARSIM_NS_END
