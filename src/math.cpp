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

#if defined(_DEBUG)
void debug_validate_acceleration(const triple& acceleration)
{
  // Make sure we don't end up with NaNs everywhere!
  assert(!std::isnan(acceleration[0]));
  assert(!std::isnan(acceleration[1]));
  assert(!std::isnan(acceleration[2]));
  // TODO: further validation? max/min values?
}
#else
constexpr void debug_validate_acceleration(const triple& acceleration)
{
  // no op
}
#endif

// In general, the formula is:
//
//   a_i = \sum_{i \ne j} G * m_j * (x_j - x_i) / pow(norm(x_j - x_i) + softening_factor, 3)
//
// Below function calculates it for a single object
void calculate_acceleration(const triple& x_i, const triple& x_j, real unadjusted_mass, real softening,
                            triple& acceleration)
{
  const triple displacement = x_j - x_i;

  const real distance = length(displacement) + softening;
  const real divisor  = distance * distance * distance;

  unadjusted_mass *= gravitational_constant;

  acceleration[0] += unadjusted_mass * displacement[0] / divisor;
  acceleration[1] += unadjusted_mass * displacement[1] / divisor;
  acceleration[2] += unadjusted_mass * displacement[2] / divisor;
  debug_validate_acceleration(acceleration);
}

// calculate acceleration pairwise for (i, j) and (j, i)
// This allows us to cut down on the more expensive calculations (e.g. sqrt)
void calculate_acceleration(const triple& x_i, const triple& x_j, real unadjusted_mass_i, real unadjusted_mass_j,
                            real softening, triple& acceleration_i, triple& acceleration_j)
{
  const triple displacement = x_j - x_i;

  const real distance = length(displacement) + softening;
  const real divisor  = distance * distance * distance;

  unadjusted_mass_j *= gravitational_constant;
  unadjusted_mass_i *= gravitational_constant;

  acceleration_i[0] += unadjusted_mass_j * displacement[0] / divisor;
  acceleration_i[1] += unadjusted_mass_j * displacement[1] / divisor;
  acceleration_i[2] += unadjusted_mass_j * displacement[2] / divisor;
  debug_validate_acceleration(acceleration_i);

  acceleration_j[0] -= unadjusted_mass_i * displacement[0] / divisor;
  acceleration_j[1] -= unadjusted_mass_i * displacement[1] / divisor;
  acceleration_j[2] -= unadjusted_mass_i * displacement[2] / divisor;
  debug_validate_acceleration(acceleration_j);
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

// Energy calculation

real calculate_kinetic_energy(real unadjusted_mass, const triple& velocity)
{
  return 0.5 * unadjusted_mass * squared_length(velocity);
}

real calculate_potential_energy(real unadjusted_mass_i, real unadjusted_mass_j, const triple& x_i, const triple& x_j)
{
  return gravitational_constant * unadjusted_mass_i * unadjusted_mass_j / length(x_j - x_i);
}

SOLARSIM_NS_END
