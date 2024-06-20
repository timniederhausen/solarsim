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
#ifndef SOLARSIM_HPX_NAMESPACES_HPP
#define SOLARSIM_HPX_NAMESPACES_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

// Don't #include HPX headers here unnecessarily!
namespace hpx::execution::experimental::detail {
}
namespace hpx::this_thread::experimental {
}

SOLARSIM_NS_BEGIN

namespace impl_hpx {

// Use common aliases. These names are used by HPX itself (e.g. in test code)
// see e.g. //libs/core/algorithms/tests/unit/algorithms/rotate_sender.cpp
namespace ex = hpx::execution::experimental;
namespace tt = hpx::this_thread::experimental;

} // namespace impl_hpx

SOLARSIM_NS_END

#endif
