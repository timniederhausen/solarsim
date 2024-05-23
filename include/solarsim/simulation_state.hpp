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
#ifndef SOLARSIM_SIMULATIONSTATE_HPP
#define SOLARSIM_SIMULATIONSTATE_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include "solarsim/types.hpp"

#include <vector>
#include <span>

SOLARSIM_NS_BEGIN

// TODO: Perhaps there's a better way to express this...
// I don't like introducing this type just for the async algorithms
struct simulation_state
{
  std::vector<triple> body_positions;
  std::vector<triple> body_velocities;
  std::vector<real> body_masses;
  real softening_factor;
  std::vector<triple> acceleration;
};

template <typename T>
concept any_simulation_state = requires(T a) {
  // Let the span<> constructors do the heavy lifting here!
  {
    a.body_positions
  } -> std::convertible_to<std::span<triple>>;
  {
    a.body_masses
  } -> std::convertible_to<std::span<const real>>;
  {
    a.softening_factor
  } -> std::convertible_to<real>;
  {
    a.acceleration
  } -> std::convertible_to<std::span<triple>>;
};

struct simulation_state_view
{
  constexpr simulation_state_view()                             = default;
  constexpr simulation_state_view(const simulation_state_view&) = default;

  // conversion from e.g. owned to a view
  constexpr simulation_state_view(const any_simulation_state auto& other)
    : body_positions(other.body_positions)
    , body_velocities(other.body_velocities)
    , body_masses(other.body_masses)
    , softening_factor(other.softening_factor)
    , acceleration(other.acceleration)
  {
  }

  constexpr simulation_state_view& operator=(const simulation_state_view&) = default;

  std::span<triple> body_positions;
  std::span<triple> body_velocities;
  std::span<const real> body_masses;
  real softening_factor = 0.0;
  std::span<triple> acceleration;
};

constexpr std::size_t get_dataset_size(const any_simulation_state auto& state)
{
  return state.body_positions.size();
}

SOLARSIM_NS_END

#endif
