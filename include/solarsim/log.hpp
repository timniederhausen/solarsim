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
#ifndef SOLARSIM_LOG_HPP
#define SOLARSIM_LOG_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include <fmt/format.h>

SOLARSIM_NS_BEGIN

enum class log_severity
{
  debug,
  verbose,
  info,
  warning,
  error,
  disabled
};

using log_callback = void (*)(log_severity severity, const char* filename, int line, std::string_view format,
                              fmt::format_args args);

void set_min_severity(log_severity severity) noexcept;
void set_log_callback(log_callback cb) noexcept;

bool is_severity_enabled(log_severity severity) noexcept;

void vlog(log_severity severity, const char* filename, int line, fmt::string_view format, fmt::format_args args);

template <typename... T>
void log(log_severity severity, const char* filename, int line, fmt::format_string<T...> fmt, T&&... args)
{
  return vlog(severity, filename, line, fmt, fmt::make_format_args(args...));
}

#if !defined(_DEBUG) && !defined(SOLARSIM_LOG_SOURCE_LOCATION)
#  define SOLARSIM_LOG(severity, format, ...)                                                \
    do {                                                                                     \
      if (SOLARSIM_NS::is_severity_enabled(SOLARSIM_NS::log_severity::severity)) {           \
        SOLARSIM_NS::log(SOLARSIM_NS::log_severity::severity, {}, 0, format, ##__VA_ARGS__); \
      }                                                                                      \
    } while (false)
#else
#  define SOLARSIM_LOG(severity, format, ...)                                                             \
    do {                                                                                                  \
      if (SOLARSIM_NS::is_severity_enabled(SOLARSIM_NS::log_severity::severity)) {                        \
        SOLARSIM_NS::log(SOLARSIM_NS::log_severity::severity, __FILE__, __LINE__, format, ##__VA_ARGS__); \
      }                                                                                                   \
    } while (false)
#endif

#define SOLARSIM_LOG_DEBUG(format, ...) SOLARSIM_LOG(debug, format, ##__VA_ARGS__)
#define SOLARSIM_LOG_VERBOSE(format, ...) SOLARSIM_LOG(verbose, format, ##__VA_ARGS__)
#define SOLARSIM_LOG_INFO(format, ...) SOLARSIM_LOG(info, format, ##__VA_ARGS__)
#define SOLARSIM_LOG_WARNING(format, ...) SOLARSIM_LOG(warning, format, ##__VA_ARGS__)
#define SOLARSIM_LOG_ERROR(format, ...) SOLARSIM_LOG(error, format, ##__VA_ARGS__)

#if defined(_DEBUG) || defined(SOLARSIM_ENABLE_DLOG)
#  define SOLARSIM_DLOG_DEBUG(format, ...) SOLARSIM_LOG(debug, format, ##__VA_ARGS__)
#  define SOLARSIM_DLOG_VERBOSE(format, ...) SOLARSIM_LOG(verbose, format, ##__VA_ARGS__)
#  define SOLARSIM_DLOG_INFO(format, ...) SOLARSIM_LOG(info, format, ##__VA_ARGS__)
#  define SOLARSIM_DLOG_WARNING(format, ...) SOLARSIM_LOG(warning, format, ##__VA_ARGS__)
#  define SOLARSIM_DLOG_ERROR(format, ...) SOLARSIM_LOG(error, format, ##__VA_ARGS__)
#else
#  define SOLARSIM_DLOG_DEBUG(format, ...) (void)0
#  define SOLARSIM_DLOG_VERBOSE(format, ...) (void)0
#  define SOLARSIM_DLOG_INFO(format, ...) (void)0
#  define SOLARSIM_DLOG_WARNING(format, ...) (void)0
#  define SOLARSIM_DLOG_ERROR(format, ...) (void)0
#endif

SOLARSIM_NS_END

#endif
