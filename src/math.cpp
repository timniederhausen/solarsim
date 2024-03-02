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
#include "solarsim/math.hpp"
#include "solarsim/body_definition.hpp"

#include <cassert>

SOLARSIM_NS_BEGIN

// In general, the formula is:
//
//   a_i = \sum_{i \ne j} G * m_j * (x_j - x_i) / pow(norm(x_j - x_i) + softening_factor, 3)
//
// Below function calculates it for a single (i x j) pair
// Note: |adjusted_mass| is already adjusted by the system's gravity!
void calculate_acceleration(const triple& xi, const triple& xj, real adjusted_mass, real softening,
                            triple& acceleration)
{
  triple displacement;
  displacement[0] = xj[0] - xi[0];
  displacement[1] = xj[1] - xi[1];
  displacement[2] = xj[2] - xi[2];

  // const real divisor  = std::pow(squared_length(displacement) + softening * softening, 3.0 / 2.0);
  const real divisor = std::pow(length(displacement) + softening, 3.0);

  acceleration[0] += adjusted_mass * displacement[0] / divisor;
  acceleration[1] += adjusted_mass * displacement[1] / divisor;
  acceleration[2] += adjusted_mass * displacement[2] / divisor;
}

void debug_validate_acceleration(const triple& acceleration)
{
  // Make sure we don't end up with NaNs everywhere!
  assert(!std::isnan(acceleration[0]));
  assert(!std::isnan(acceleration[1]));
  assert(!std::isnan(acceleration[2]));
  // TODO: further validation?
}

// Velocity verlet
//
// This is supposed to be used in two phases, between which the |acceleration|
// is re-calculated based on the new body positions.
//
// Steps:
//   calculate acceleration(...)
//   integrate_velocity_verlet_phase1(...)
//   calculate acceleration(...)
//   integrate_velocity_verlet_phase2(...)

void integrate_velocity_verlet_phase1(triple& position, triple& velocity, const triple& acceleration, real dT)
{
  // v_{i+1/2} = v_i + 0.5 \times a[i] \times \Delta t
  velocity[0] += acceleration[0] * 0.5 * dT;
  velocity[1] += acceleration[1] * 0.5 * dT;
  velocity[2] += acceleration[2] * 0.5 * dT;

  // x_{i+1} = x_i + v_{i+1/2} \times \Delta t
  position[0] += velocity[0] * dT;
  position[1] += velocity[1] * dT;
  position[2] += velocity[2] * dT;
}

void integrate_velocity_verlet_phase2(triple& velocity, const triple& acceleration, real dT)
{
  // v_{i+1} = v_{i+1/2} + 0.5 \times a_{i+1} \times \Delta t
  velocity[0] += acceleration[0] * 0.5 * dT;
  velocity[1] += acceleration[1] * 0.5 * dT;
  velocity[2] += acceleration[2] * 0.5 * dT;
}

// Leapfrog
//
// This is the velocity verlet with a time shift of one half of a step.
// Has the benefit of not needing a preserved acceleration vector!
//
// Steps:
//   integrate_leapfrog_phase1(...)
//   calculate acceleration(...)
//   integrate_leapfrog_phase2(...)

void integrate_leapfrog_phase1(triple& position, const triple& velocity, real dT)
{
  // x_{i+1/2} = x_i + 0.5 \times v_{i} \times \Delta t
  position[0] += velocity[0] * 0.5 * dT;
  position[1] += velocity[1] * 0.5 * dT;
  position[2] += velocity[2] * 0.5 * dT;
}

void integrate_leapfrog_phase2(triple& position, triple& velocity, const triple& acceleration, real dT)
{
  // v_{i+1} = v_i + a_{i+1/2} \times \Delta t
  velocity[0] += acceleration[0] * dT;
  velocity[1] += acceleration[1] * dT;
  velocity[2] += acceleration[2] * dT;

  // x_{i+1} = x_{i+1/2} + 0.5 \times v_{i+1} \times \Delta t
  position[0] += velocity[0] * 0.5 * dT;
  position[1] += velocity[1] * 0.5 * dT;
  position[2] += velocity[2] * 0.5 * dT;
}

SOLARSIM_NS_END
