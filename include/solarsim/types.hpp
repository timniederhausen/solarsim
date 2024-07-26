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
#include <cstring>

SOLARSIM_NS_BEGIN

// Floating point values - can switch between 32bits (float) and 64bits (double)
// Default to double as our test data needs this precision
using real = double;

// see https://randomascii.wordpress.com/2012/01/11/tricks-with-the-floating-point-format/
template <typename Real>
struct representation_type;

template <>
struct representation_type<float>
{
  struct type
  {
    type(float num = 0.0f)
    {
      // The easiest & cleanest way w/o violating strict aliasing etc.
      std::memcpy(&i, &num, sizeof(i));
    }

    // Portable extraction of components.
    bool negative() const { return (i >> 31) != 0; }
    int32_t mantissa() const { return i & ((1 << 23) - 1); }
    int32_t exponent() const { return (i >> 23) & 0xFF; }

    int32_t i;
  };
};

template <>
struct representation_type<double>
{
  struct type
  {
    type(double num = 0.0)
    {
      // The easiest & cleanest way w/o violating strict aliasing etc.
      std::memcpy(&i, &num, sizeof(i));
    }

    // Portable extraction of components.
    bool negative() const { return (i >> 63) != 0; }
    int64_t mantissa() const { return i & ((1ll << 52) - 1); }
    int64_t exponent() const { return (i >> 52) & 0x7FF; }

    int64_t i;
  };
};

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

// TODO: move this somewhere else. it's not a "basic" type per-se and requires an include
template <typename Char>
class basic_zstring_view
{
public:
  constexpr basic_zstring_view(const Char* s)
    : data_(s)
  {
  }

  template <typename Traits, typename Alloc>
  constexpr basic_zstring_view(const std::basic_string<Char, Traits, Alloc>& s)
    : data_(s.c_str())
  {
  }

  constexpr const Char* c_str() const { return data_; }

private:
  const Char* data_;
};

using zstring_view = basic_zstring_view<char>;

SOLARSIM_NS_END

#include "solarsim/types_inlines.hpp"

#endif
