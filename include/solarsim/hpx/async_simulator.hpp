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
// Logic fragments come from the sync simulators!
#include "solarsim/sync_simulator.hpp"

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
