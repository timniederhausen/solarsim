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
#ifndef SOLARSIM_BODYDEFINITION_HPP
#define SOLARSIM_BODYDEFINITION_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include "solarsim/types.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

#include <string>

SOLARSIM_NS_BEGIN

struct body_definition
{
  // Identifier & assigned name (optional)
  std::string id;
  std::string name;
  // TODO: Add enum for this!
  std::string type;

  real mass;
  triple position;
  triple velocity;
};

SOLARSIM_NS_END

// We can't have those in a namespace

BOOST_FUSION_ADAPT_STRUCT(
  SOLARSIM_NS::body_definition,
  (std::string, id)
  (std::string, name)
  (std::string, type)
  (SOLARSIM_NS::real, mass)
  // Triples are really just three separate values
  (SOLARSIM_NS::real, position[0])
  (SOLARSIM_NS::real, position[1])
  (SOLARSIM_NS::real, position[2])
  // ...
  (SOLARSIM_NS::real, velocity[0])
  (SOLARSIM_NS::real, velocity[1])
  (SOLARSIM_NS::real, velocity[2])
)

#endif
