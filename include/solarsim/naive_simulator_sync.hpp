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

#include "solarsim/body_definition.hpp"

#include <vector>
#include <span>

SOLARSIM_NS_BEGIN

/// A very simple and naive N-body simulator
class naive_simulator_sync
{
public:
  naive_simulator_sync(std::span<body_definition> bodies, real softening_factor);

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

/**
 * \brief Run a complete simulation with a fixed time step and a given duration
 * \param simulator simulation state
 * \param time_step Time between simulation ticks
 * \param duration Total runtime of the simulation
 */
void run_simulation(naive_simulator_sync& simulator, real time_step, real duration);

SOLARSIM_NS_END

#endif
