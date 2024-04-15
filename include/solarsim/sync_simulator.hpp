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
#ifndef SOLARSIM_NAIVESIMULATORSYNC_HPP
#define SOLARSIM_NAIVESIMULATORSYNC_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include <cassert>

#include "solarsim/body_definition.hpp"

#include <vector>
#include <span>

#include "math.hpp"

SOLARSIM_NS_BEGIN

template <typename T>
concept simulation_algorithm = requires(T a) {
  a.tick(std::declval<std::span<const body_definition>>(), real(), std::declval<std::span<triple>>());
};

/// Boilerplate for a simple synchronous simulator
template <simulation_algorithm S, bool UseShiftedVerlet = true>
class basic_sync_simulator
{
public:
  basic_sync_simulator(std::span<body_definition> bodies, real softening_factor)
    : bodies_(bodies)
    , softening_factor_(softening_factor)
    , acceleration_(bodies_.size())
  {
    if (!UseShiftedVerlet)
      update_acceleration();
  }

  /**
   * \brief Advance the simulation by \c dT seconds
   * \param dT Elapsed time in seconds
   */
  void tick(real dT);

private:
  void update_acceleration();

  std::span<body_definition> bodies_;
  real softening_factor_;

  // Temporary cache for acceleration values during ticking
  std::vector<triple> acceleration_;
};

template <simulation_algorithm S, bool UseShiftedVerlet>
void basic_sync_simulator<S, UseShiftedVerlet>::tick(real dT)
{
  if constexpr (UseShiftedVerlet) {
    for (std::size_t i = 0, n = bodies_.size(); i != n; ++i)
      integrate_leapfrog_phase1(bodies_[i].position, bodies_[i].velocity, dT);

    update_acceleration();

    for (std::size_t i = 0, n = bodies_.size(); i != n; ++i)
      integrate_leapfrog_phase2(bodies_[i].position, bodies_[i].velocity, acceleration_[i], dT);
  } else {
    // needs previous acceleration!
    for (std::size_t i = 0, n = bodies_.size(); i != n; ++i)
      integrate_velocity_verlet_phase1(bodies_[i].position, bodies_[i].velocity, acceleration_[i], dT);

    update_acceleration();

    for (std::size_t i = 0, n = bodies_.size(); i != n; ++i)
      integrate_velocity_verlet_phase2(bodies_[i].velocity, acceleration_[i], dT);
  }
}

template <simulation_algorithm S, bool UseShiftedVerlet>
void basic_sync_simulator<S, UseShiftedVerlet>::update_acceleration()
{
  S worker;
  worker.tick(bodies_, softening_factor_, acceleration_);
}

// Simulation algorithm implementations:

struct naive_sync_simulator_impl
{
  void tick(std::span<const body_definition> bodies, real softening_factor, std::span<triple> acceleration) const;
};

struct barnes_hut_sync_simulator_impl
{
  void tick(std::span<const body_definition> bodies, real softening_factor, std::span<triple> acceleration) const;
};

// Easy-to-use simulator types:

using naive_sync_simulator      = basic_sync_simulator<naive_sync_simulator_impl>;
using barnes_hut_sync_simulator = basic_sync_simulator<barnes_hut_sync_simulator_impl>;

/**
 * \brief Run a complete simulation with a fixed time step and a given duration
 * \param simulator simulation state
 * \param time_step Time between simulation ticks
 * \param duration Total runtime of the simulation
 */
template <typename Simulator>
void run_simulation(Simulator& simulator, real time_step, real duration)
{
  assert(time_step <= duration);

  // Start simulating at |time_step|
  for (real elapsed = time_step; elapsed < duration;) {
    simulator.tick(time_step);
    elapsed += time_step;
  }
}

SOLARSIM_NS_END

#endif
