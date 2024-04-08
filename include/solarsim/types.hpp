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
#ifndef SOLARSIM_TYPES_HPP
#define SOLARSIM_TYPES_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include <string>

SOLARSIM_NS_BEGIN

// Floating point values - can switch between 32bits (float) and 64bits (double)
// Default to double as our test data needs this precision
using real = double;

// TODO: get these from a math library (w. proper alignment + SIMD support)
// We need one that supports double precision though (so DirectXMath etc. are out for now)

// A triple is generally used to represent a column point or vector
struct triple
{
  // subscription operators
  [[nodiscard]] constexpr real& operator[](std::size_t i) noexcept { return v[i]; }
  [[nodiscard]] constexpr const real& operator[](std::size_t i) const noexcept { return v[i]; }

  // data() + size() for span<> etc. support
  [[nodiscard]] constexpr std::size_t size() const noexcept { return 3; }
  [[nodiscard]] constexpr real* data() noexcept { return v; }
  [[nodiscard]] constexpr const real* data() const noexcept { return v; }

  real v[3];
};

// AABBs are very basic axis-aligned collision primitives
struct axis_aligned_bounding_box
{
  static constexpr axis_aligned_bounding_box infinity();

  triple min;
  triple max;
};

SOLARSIM_NS_END

#include "solarsim/types_inlines.hpp"

#endif
