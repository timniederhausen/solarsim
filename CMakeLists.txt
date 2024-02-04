cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    SolarSim
    VERSION 0.1.0
    DESCRIPTION "Solar system simulation with C++20 and HPX"
    HOMEPAGE_URL "https://github.com/timniederhausen/SolarSim"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

add_subdirectory(src)

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT SolarSim_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of SolarSim"
  )
endif()

include(cmake/dev-mode.cmake)
