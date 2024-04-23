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
#ifndef SOLARSIM_ASYNCSIMULATOR_HPP
#define SOLARSIM_ASYNCSIMULATOR_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include "solarsim/sync_simulator.hpp"
#include "solarsim/execution_concepts.hpp"

#include <hpx/execution/algorithms/bulk.hpp>
#include <hpx/execution/algorithms/let_value.hpp>

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

  // conversion from e.g. owned to _view
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

// These algorithms follow the rules for pipeable sender adaptors
// see: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2300r9.html#spec-execution.senders.adaptor.objects
//
// Possible futher work: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3300r0.html

inline constexpr struct async_tick_naive_t
{
  constexpr auto operator()() const
  {
    return ex::then([](any_simulation_state auto&& state) {
      naive_sync_simulator_impl().tick(state.body_positions, state.body_masses, state.softening_factor,
                                       state.acceleration);
      return std::move(state);
    });
  }

  template <sender Sender>
  constexpr auto operator()(Sender&& sender) const
  {
    return ex::then(std::forward<Sender>(sender), [](any_simulation_state auto&& state) {
      naive_sync_simulator_impl().tick(state.body_positions, state.body_masses, state.softening_factor,
                                       state.acceleration);
      return std::move(state);
    });
  }
} async_tick_naive{};

// TODO: introduce parallel versions of these algorithms
// Parallel construction of the tree might be possible, might also be out of scope?
inline constexpr struct async_tick_barnes_hut_t
{
  constexpr auto operator()() const
  {
    return ex::then([](any_simulation_state auto&& state) {
      barnes_hut_sync_simulator_impl().tick(state.body_positions, state.body_masses, state.softening_factor,
                                            state.acceleration);
      return std::move(state);
    });
  }

  template <sender Sender>
  constexpr auto operator()(Sender&& sender) const
  {
    return ex::then(std::forward<Sender>(sender), [](any_simulation_state auto&& state) {
      barnes_hut_sync_simulator_impl().tick(state.body_positions, state.body_masses, state.softening_factor,
                                            state.acceleration);
      return std::move(state);
    });
  }
} async_tick_barnes_hut{};

inline constexpr struct async_tick_simulation_phase1_t
{
  constexpr auto operator()(std::size_t num_bodies, real dT) const
  {
    return ex::bulk(num_bodies, [=](std::size_t i, any_simulation_state auto& state) {
      if constexpr (true) {
        integrate_leapfrog_phase1(state.body_positions[i], state.body_velocities[i], dT);
      } else {
        // needs previous acceleration!
        integrate_velocity_verlet_phase1(state.body_positions[i], state.body_velocities[i], state.acceleration[i], dT);
      }
    });
  }

  template <sender Sender>
  constexpr auto operator()(Sender&& sender, std::size_t num_bodies, real dT) const
  {
    return ex::bulk(std::forward<Sender>(sender), num_bodies, [=](std::size_t i, any_simulation_state auto& state) {
      if constexpr (true) {
        integrate_leapfrog_phase1(state.body_positions[i], state.body_velocities[i], dT);
      } else {
        // needs previous acceleration!
        integrate_velocity_verlet_phase1(state.body_positions[i], state.body_velocities[i], state.acceleration[i], dT);
      }
    });
  }
} async_tick_simulation_phase1{};

inline constexpr struct async_tick_simulation_phase2_t
{
  constexpr auto operator()(std::size_t num_bodies, real dT) const
  {
    return ex::bulk(num_bodies, [=](std::size_t i, any_simulation_state auto& state) {
      if constexpr (true) {
        integrate_leapfrog_phase2(state.body_positions[i], state.body_velocities[i], state.acceleration[i], dT);
      } else {
        integrate_velocity_verlet_phase2(state.body_velocities[i], state.acceleration[i], dT);
      }
    });
  }

  template <sender Sender>
  constexpr auto operator()(Sender&& sender, std::size_t num_bodies, real dT) const
  {
    return ex::bulk(std::forward<Sender>(sender), num_bodies, [=](std::size_t i, any_simulation_state auto& state) {
      if constexpr (true) {
        integrate_leapfrog_phase2(state.body_positions[i], state.body_velocities[i], state.acceleration[i], dT);
      } else {
        integrate_velocity_verlet_phase2(state.body_velocities[i], state.acceleration[i], dT);
      }
    });
  }
} async_tick_simulation_phase2{};

SOLARSIM_NS_END

#endif
