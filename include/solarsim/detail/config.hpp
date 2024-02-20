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
#ifndef SOLARSIM_DETAIL_CONFIG_HPP
#define SOLARSIM_DETAIL_CONFIG_HPP

// clang-format off
#define SOLARSIM_NS solarsim::v1
#define SOLARSIM_NS_BEGIN namespace solarsim { inline namespace v1 {
#define SOLARSIM_NS_END } }
// clang-format on

#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__CODEGEARC__)
#  if defined(SOLARSIM_DYN_LINK)
#    if defined(SOLARSIM_SOURCE)
#      define SOLARSIM_DECL __declspec(dllexport)
#    else
#      define SOLARSIM_DECL __declspec(dllimport)
#    endif
#  endif
#endif

#if !defined(SOLARSIM_DECL)
#  define SOLARSIM_DECL
#endif

// TODO: every supported compiler has that?
#define SOLARSIM_HAS_PRAGMA_ONCE 1

#endif
