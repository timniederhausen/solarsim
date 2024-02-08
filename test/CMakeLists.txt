cmake_minimum_required(VERSION 3.14)

project(SolarSimTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(SolarSim REQUIRED)
  enable_testing()
endif()

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(SolarSim_test src/body_definition_csv.cpp)
target_link_libraries(
    SolarSim_test PRIVATE
    SolarSim::SolarSim
    Catch2::Catch2WithMain
)
target_compile_features(SolarSim_test PRIVATE cxx_std_20)

catch_discover_tests(SolarSim_test)

# ---- End-of-file commands ----

add_folders(Test)
