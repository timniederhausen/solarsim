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
#ifndef SOLARSIM_TYPES_INLINES_HPP
#define SOLARSIM_TYPES_INLINES_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include <limits>

SOLARSIM_NS_BEGIN

constexpr triple operator*(const triple& lhs, real rhs)
{
  return triple{lhs.v[0] * rhs, lhs.v[1] * rhs, lhs.v[2] * rhs};
}
constexpr triple operator/(const triple& lhs, real rhs)
{
  return triple{lhs.v[0] / rhs, lhs.v[1] / rhs, lhs.v[2] / rhs};
}

constexpr triple operator+(const triple& lhs, real rhs)
{
  return triple{lhs.v[0] + rhs, lhs.v[1] + rhs, lhs.v[2] + rhs};
}
constexpr triple operator-(const triple& lhs, real rhs)
{
  return triple{lhs.v[0] - rhs, lhs.v[1] - rhs, lhs.v[2] - rhs};
}

constexpr triple operator*(const triple& lhs, const triple& rhs)
{
  return triple{lhs.v[0] * rhs.v[0], lhs.v[1] * rhs.v[1], lhs.v[2] * rhs.v[2]};
}
constexpr triple operator/(const triple& lhs, const triple& rhs)
{
  return triple{lhs.v[0] / rhs.v[0], lhs.v[1] / rhs.v[1], lhs.v[2] / rhs.v[2]};
}

constexpr triple operator+(const triple& lhs, const triple& rhs)
{
  return triple{lhs.v[0] + rhs.v[0], lhs.v[1] + rhs.v[1], lhs.v[2] + rhs.v[2]};
}
constexpr triple operator-(const triple& lhs, const triple& rhs)
{
  return triple{lhs.v[0] - rhs.v[0], lhs.v[1] - rhs.v[1], lhs.v[2] - rhs.v[2]};
}

constexpr triple operator-(const triple& lhs)
{
  return triple{-lhs.v[0], -lhs.v[1], -lhs.v[2]};
}

constexpr triple& operator*=(triple& lhs, real rhs)
{
  lhs[0] *= rhs;
  lhs[1] *= rhs;
  lhs[2] *= rhs;
  return lhs;
}
constexpr triple& operator/=(triple& lhs, real rhs)
{
  lhs[0] /= rhs;
  lhs[1] /= rhs;
  lhs[2] /= rhs;
  return lhs;
}
constexpr triple& operator+=(triple& lhs, const triple& rhs)
{
  lhs[0] += rhs[0];
  lhs[1] += rhs[1];
  lhs[2] += rhs[2];
  return lhs;
}
constexpr triple& operator-=(triple& lhs, const triple& rhs)
{
  lhs[0] -= rhs[0];
  lhs[1] -= rhs[1];
  lhs[2] -= rhs[2];
  return lhs;
}
constexpr triple& operator*=(triple& lhs, const triple& rhs)
{
  lhs[0] *= rhs[0];
  lhs[1] *= rhs[1];
  lhs[2] *= rhs[2];
  return lhs;
}
constexpr triple& operator/=(triple& lhs, const triple& rhs)
{
  lhs[0] /= rhs[0];
  lhs[1] /= rhs[1];
  lhs[2] /= rhs[2];
  return lhs;
}

constexpr axis_aligned_bounding_box axis_aligned_bounding_box::infinity()
{
  // min() gives us smallest *normalized* value
  // see: https://en.cppreference.com/w/cpp/types/numeric_limits/lowest
  constexpr real min = std::numeric_limits<real>::lowest();
  constexpr real max = std::numeric_limits<real>::max();
  // important: max & min are swapped here
  return {
      {max, max, max},
      {min, min, min},
  };
}

SOLARSIM_NS_END

#endif
