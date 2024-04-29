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
#pragma once

#include "solarsim/body_definition.hpp"
#include "solarsim/math.hpp"

#include <span>
#include <vector>

SOLARSIM_NS_BEGIN

// TODO: do we need this for every dataset?
inline void adjust_initial_velocities(std::span<body_definition> bodies)
{
  std::vector<triple> u_i(bodies.size());

  for (std::size_t i = 0, n = bodies.size(); i != n; ++i) {
    triple adjustment = {};
    real mass_sum     = 0;
    for (std::size_t j = 0; j != n; ++j) {
      adjustment = adjustment + bodies[j].velocity[0] * bodies[j].mass;
      mass_sum += bodies[j].mass;
    }
    u_i[i] = adjustment / mass_sum;
  }

  for (std::size_t i = 0, n = bodies.size(); i != n; ++i) {
    bodies[i].velocity[0] -= u_i[i][0];
    bodies[i].velocity[1] -= u_i[i][1];
    bodies[i].velocity[2] -= u_i[i][2];
  }
}

// Depending on the units we want to use, some conversion becomes necessary.

// Our input files have the following units:
// kg for mass, pc / year for velocity and pc for positions
struct ipvs_dataset
{
  // XXX: only used for some tests!
  static constexpr bool use_common_gravity_constant = false;

  static void normalize_body_values(body_definition& body)
  {
    if (use_common_gravity_constant) {
      // pc => m
      body.position[0] *= parsec_in_m;
      body.position[1] *= parsec_in_m;
      body.position[2] *= parsec_in_m;

      // pc / y => m / s
      body.velocity[0] *= parsec_in_m / year_in_seconds;
      body.velocity[1] *= parsec_in_m / year_in_seconds;
      body.velocity[2] *= parsec_in_m / year_in_seconds;
    } else {
      body.mass = body.mass / solar_mass_in_kg;

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

  static void denormalize_body_values(body_definition& body)
  {
    if (use_common_gravity_constant) {
      // m => pc
      body.position[0] /= parsec_in_m;
      body.position[1] /= parsec_in_m;
      body.position[2] /= parsec_in_m;

      // m / s => pc / y
      body.velocity[0] /= parsec_in_m * year_in_seconds;
      body.velocity[1] /= parsec_in_m * year_in_seconds;
      body.velocity[2] /= parsec_in_m * year_in_seconds;
    } else {
      body.mass = body.mass * solar_mass_in_kg;

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
};

// Our input files have the following units:
// kg for mass, m / s for velocity and m for positions
struct historic_1970_dataset
{
  static void normalize_body_values_1970(body_definition& body)
  {
    body.mass = body.mass / solar_mass_in_kg;

    // m => km
    body.position[0] /= 1000;
    body.position[1] /= 1000;
    body.position[2] /= 1000;

    // m / s => km / s
    body.velocity[0] /= 1000 * 1000;
    body.velocity[1] /= 1000 * 1000;
    body.velocity[2] /= 1000 * 1000;
  }

  static void denormalize_body_values_1970(body_definition& body)
  {
    body.mass = body.mass * solar_mass_in_kg;

    // km => m
    body.position[0] *= 1000;
    body.position[1] *= 1000;
    body.position[2] *= 1000;

    // km / s => m / s
    body.velocity[0] *= 1000 * 1000;
    body.velocity[1] *= 1000 * 1000;
    body.velocity[2] *= 1000 * 1000;
  }
};

SOLARSIM_NS_END
