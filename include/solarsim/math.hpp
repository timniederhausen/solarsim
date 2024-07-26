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
#ifndef SOLARSIM_MATH_HPP
#define SOLARSIM_MATH_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include "solarsim/types.hpp"

#include <cmath>

SOLARSIM_NS_BEGIN

// Unit conversions
inline constexpr real parsec_in_m      = 3.08567758129e16;
inline constexpr real parsec_in_km     = parsec_in_m / 1000.0;
inline constexpr real solar_mass_in_kg = 1.988435e30;
inline constexpr real year_in_seconds  = 365.25 * 86400;

// TODO: maybe switch to https://github.com/mpusz/mp-units?

// We are dealing with astrophysics here so our units of measurement are:
//  * Kilometers (km) for distances
//  * km/s for velocities
//  * Solar mass (Mo) for entity masses
//
// Commonly, parsec (pc) is used for distances, however that would complicate the
// position update code.

// Unit is m \times (m / s)^2 \times kg ^ (-1)
inline constexpr real gravitational_constant_common = 6.67428e-11;

// Unit is km \times (km / s)^2 \times M ^ (-1)
inline constexpr real gravitational_constant = gravitational_constant_common / 1000 / (1000 * 1000) * solar_mass_in_kg;

// TODO: Maybe have a sort of dataset_math<> policy template that contains the necessary conversion functions?

constexpr real squared_length(const triple& v)
{
  return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

/*constexpr?*/ inline real length(const triple& v)
{
  return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void calculate_acceleration(const triple& x_i, const triple& x_j, real unadjusted_mass, real softening,
                            triple& acceleration);
void calculate_acceleration(const triple& x_i, const triple& x_j, real unadjusted_mass_i, real unadjusted_mass_j,
                            real softening, triple& acceleration_i, triple& acceleration_j);

void integrate_velocity_verlet_phase1(triple& position, triple& velocity, const triple& acceleration, real dT);
void integrate_velocity_verlet_phase2(triple& velocity, const triple& acceleration, real dT);

void integrate_leapfrog_phase1(triple& position, const triple& velocity, real dT);
void integrate_leapfrog_phase2(triple& position, triple& velocity, const triple& acceleration, real dT);

// System energy
real calculate_kinetic_energy(real unadjusted_mass, const triple& velocity);
real calculate_potential_energy(real unadjusted_mass_i, real unadjusted_mass_j, const triple& x_i, const triple& x_j);

// Data validation
#if defined(_DEBUG)
bool almost_equal_ulps(real a, real b, int max_ulps_diff = 4);
bool almost_equal_ulps(const triple& a, const triple& b, int max_ulps_diff = 4);
void debug_validate_finite(const triple& v);
#else
constexpr void debug_validate_finite(const triple&)
{
  // no op
}
#endif

SOLARSIM_NS_END

#endif
