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
#ifndef SOLARSIM_HPX_ASYNCSIMULATOR_HPP
#define SOLARSIM_HPX_ASYNCSIMULATOR_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include "solarsim/hpx/namespaces.hpp"
#include "solarsim/simulation_state.hpp"
#include "solarsim/barnes_hut_octree.hpp"

#include <hpx/execution/traits/is_execution_policy.hpp>
#include <hpx/parallel/algorithms/for_loop.hpp>

SOLARSIM_NS_BEGIN

namespace impl_hpx {

// XXX: This could live elsewhere should we use those in more files
template <typename ExPolicy>
concept execution_policy = hpx::is_execution_policy_v<ExPolicy>;

template <execution_policy ExPolicy>
auto tick_simulation_phase1(ExPolicy&& policy, any_simulation_state auto&& state, real time_step)
{
  return hpx::experimental::for_loop_n(
      std::forward<ExPolicy>(policy), std::size_t(), get_dataset_size(state), [=](std::size_t i) {
        integrate_leapfrog_phase1(state.body_positions[i], state.body_velocities[i], time_step);
      });
}

template <execution_policy ExPolicy>
auto tick_barnes_hut(ExPolicy&& policy, any_simulation_state auto&& state)
{
  std::fill(state.acceleration.begin(), state.acceleration.end(), triple{});

  // Ugh, our function needs to be copyable. Just make it a shared ptr then!
  // Compared to the work we're performing, the cost is negligible.
  auto shared_octree = std::make_shared<barnes_hut_octree>(state.body_positions, state.body_masses);
  return hpx::experimental::for_loop_n(
      std::forward<ExPolicy>(policy), std::size_t(), get_dataset_size(state), [=](std::size_t i) {
        shared_octree->apply_forces_to(state.body_positions[i], state.softening_factor, state.acceleration[i]);
      });
}

template <execution_policy ExPolicy>
auto tick_simulation_phase2(ExPolicy&& policy, any_simulation_state auto&& state, real time_step)
{
  return hpx::experimental::for_loop_n(
      std::forward<ExPolicy>(policy), std::size_t(), get_dataset_size(state), [=](std::size_t i) {
        integrate_leapfrog_phase2(state.body_positions[i], state.body_velocities[i], state.acceleration[i], time_step);
      });
}

} // namespace impl_hpx

SOLARSIM_NS_END

#endif
