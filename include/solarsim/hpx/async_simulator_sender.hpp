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
#ifndef SOLARSIM_HPX_ASYNCSIMULATORSENDER_HPP
#define SOLARSIM_HPX_ASYNCSIMULATORSENDER_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include "solarsim/hpx/execution_concepts.hpp"
#include "solarsim/simulation_state.hpp"
// Logic fragments come from the sync simulators:
#include "solarsim/barnes_hut_octree.hpp"
#include "solarsim/sync_simulator.hpp"

#include <hpx/execution/algorithms/bulk.hpp>
#include <hpx/execution/algorithms/let_value.hpp>

SOLARSIM_NS_BEGIN

namespace impl_hpx {
// These algorithms follow the rules for pipeable sender adaptors
// see: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2300r9.html#spec-execution.senders.adaptor.objects
//
// Possible futher work: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3300r0.html

// TODO: These need to be combined to reduce boilerplate code!

#if defined(HPX_HAVE_STDEXEC)
#  define CONSTEXPR_FOR_HPX_SR
#else
#  define CONSTEXPR_FOR_HPX_SR constexpr
#endif

inline constexpr struct async_tick_naive_t
{
  CONSTEXPR_FOR_HPX_SR auto operator()() const
  {
    return ex::then([](any_simulation_state auto&& state) {
      hpx::scoped_annotation annotation("async_tick_naive");
      naive_sync_simulator_impl().tick(state.body_positions, state.body_masses, state.softening_factor,
                                       state.acceleration);
      return std::move(state);
    });
  }

  template <sender Sender>
  CONSTEXPR_FOR_HPX_SR auto operator()(Sender&& sender) const
  {
    return ex::then(std::forward<Sender>(sender), [](any_simulation_state auto&& state) {
      hpx::scoped_annotation annotation("async_tick_naive");
      naive_sync_simulator_impl().tick(state.body_positions, state.body_masses, state.softening_factor,
                                       state.acceleration);
      return std::move(state);
    });
  }
} async_tick_naive{};

inline constexpr struct async_tick_barnes_hut_t
{
  CONSTEXPR_FOR_HPX_SR auto operator()(auto sch) const
  {
    return ex::let_value([sch](any_simulation_state auto&& state) {
      hpx::scoped_annotation annotation("async_tick_barnes_hut");
      std::fill(state.acceleration.begin(), state.acceleration.end(), triple{});

      barnes_hut_octree octree(state.body_positions, state.body_masses);
      const auto n = get_dataset_size(state);

      return ex::transfer_just(sch, std::move(state), std::move(octree)) |
             ex::bulk(n,
                      [](std::size_t i, any_simulation_state auto& state, const barnes_hut_octree& octree) {
                        hpx::scoped_annotation annotation("async_tick_barnes_hut::apply_forces_to");
                        octree.apply_forces_to(state.body_positions[i], state.softening_factor, state.acceleration[i]);
                      }) |
             ex::then([=](any_simulation_state auto&& state, const barnes_hut_octree&) {
               return std::move(state);
             });
    });
  }

  template <sender Sender>
  CONSTEXPR_FOR_HPX_SR auto operator()(Sender&& sender, auto sch, const std::size_t& num_bodies) const
  {
    return ex::let_value(std::forward<Sender>(sender), [sch](any_simulation_state auto&& state) {
      hpx::scoped_annotation annotation("async_tick_barnes_hut");
      std::fill(state.acceleration.begin(), state.acceleration.end(), triple{});

      barnes_hut_octree octree(state.body_positions, state.body_masses);
      const auto n = get_dataset_size(state);

      return ex::transfer_just(sch, std::move(state), std::move(octree)) |
             ex::bulk(n,
                      [](std::size_t i, any_simulation_state auto& state, const barnes_hut_octree& octree) {
                        hpx::scoped_annotation annotation("async_tick_barnes_hut::apply_forces_to");
                        octree.apply_forces_to(state.body_positions[i], state.softening_factor, state.acceleration[i]);
                      }) |
             ex::then([=](any_simulation_state auto&& state, const barnes_hut_octree&) {
               return std::move(state);
             });
    });
  }
} async_tick_barnes_hut{};

inline constexpr struct async_tick_simulation_phase1_t
{
  CONSTEXPR_FOR_HPX_SR auto operator()(const std::size_t& num_bodies, real dT) const
  {
    return ex::bulk(num_bodies, [=](std::size_t i, any_simulation_state auto& state) {
      hpx::scoped_annotation annotation("async_tick_simulation_phase1");
      if constexpr (true) {
        integrate_leapfrog_phase1(state.body_positions[i], state.body_velocities[i], dT);
      } else {
        // needs previous acceleration!
        integrate_velocity_verlet_phase1(state.body_positions[i], state.body_velocities[i], state.acceleration[i], dT);
      }
    });
  }

  template <sender Sender>
  CONSTEXPR_FOR_HPX_SR auto operator()(Sender&& sender, const std::size_t& num_bodies, real dT) const
  {
    return ex::bulk(std::forward<Sender>(sender), num_bodies, [=](std::size_t i, any_simulation_state auto& state) {
      hpx::scoped_annotation annotation("async_tick_simulation_phase1");
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
  CONSTEXPR_FOR_HPX_SR auto operator()(const std::size_t& num_bodies, real dT) const
  {
    return ex::bulk(num_bodies, [=](std::size_t i, any_simulation_state auto& state) {
      hpx::scoped_annotation annotation("async_tick_simulation_phase2");
      if constexpr (true) {
        integrate_leapfrog_phase2(state.body_positions[i], state.body_velocities[i], state.acceleration[i], dT);
      } else {
        integrate_velocity_verlet_phase2(state.body_velocities[i], state.acceleration[i], dT);
      }
    });
  }

  template <sender Sender>
  CONSTEXPR_FOR_HPX_SR auto operator()(Sender&& sender, const std::size_t& num_bodies, real dT) const
  {
    return ex::bulk(std::forward<Sender>(sender), num_bodies, [=](std::size_t i, any_simulation_state auto& state) {
      hpx::scoped_annotation annotation("async_tick_simulation_phase2");
      if constexpr (true) {
        integrate_leapfrog_phase2(state.body_positions[i], state.body_velocities[i], state.acceleration[i], dT);
      } else {
        integrate_velocity_verlet_phase2(state.body_velocities[i], state.acceleration[i], dT);
      }
    });
  }
} async_tick_simulation_phase2{};

} // namespace impl_hpx

#undef CONSTEXPR_FOR_HPX_SR

SOLARSIM_NS_END

#endif
