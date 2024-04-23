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
#ifndef SOLARSIM_EXECUTIONCONCEPTS_HPP
#define SOLARSIM_EXECUTIONCONCEPTS_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include "solarsim/namespaces.hpp"

// needed for is_sender<> etc.
#include <hpx/execution_base/completion_signatures.hpp>

SOLARSIM_NS_BEGIN

// HPX as of v1.9.1 does not ship with the C++20 concepts for Senders / Receivers.
// Thankfully we can easily define them ourselves via the provided traits!
//
// These closely match what is defined by https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2300r9.html#spec

// Sender concepts
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2300r9.html#spec-execution.snd.concepts
template <typename Sender, typename Env = ex::empty_env>
concept sender_in = ex::is_sender<Sender, ex::empty_env>::value;

template <typename Sender>
concept sender = ex::is_sender<Sender, ex::empty_env>::value;

template <typename Sender, typename Env, typename... Values>
concept sender_of_in = ex::is_sender_of<Sender, Env, Values...>::value;

template <typename Sender, typename... Values>
concept sender_of = ex::is_sender_of<Sender, ex::empty_env, Values...>::value;

SOLARSIM_NS_END

#endif
