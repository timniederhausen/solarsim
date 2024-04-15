add_library(
    SolarSim_Library
    barnes_hut_octree.cpp
    log.cpp
    body_definition_csv.cpp
    math.cpp
    naive_simulator_sync.cpp
)

# Add them as PRIVATE sources here so they show up in project files
# Can't use PUBLIC etc., see: https://stackoverflow.com/a/62465051
file(GLOB_RECURSE header_files CONFIGURE_DEPENDS ../include/solarsim/*.hpp)
target_sources(SolarSim_Library PRIVATE ${header_files})

add_library(SolarSim::SolarSim ALIAS SolarSim_Library)

target_compile_definitions(SolarSim_Library PRIVATE SOLARSIM_SOURCE)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(SolarSim_Library PUBLIC SOLARSIM_DYN_LINK)
endif()

set_target_properties(
    SolarSim_Library PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME solarsim
    OUTPUT_NAME solarsim
)

target_include_directories(
    SolarSim_Library ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(SolarSim_Library PUBLIC cxx_std_20)

find_package(fmt REQUIRED)
target_link_libraries(SolarSim_Library PRIVATE fmt::fmt)

find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(SolarSim_Library PUBLIC Boost::boost Boost::system)

# TODO: seperate library for HPX executor?
find_package(HPX)
if(HPX_FOUND)
  target_link_libraries(SolarSim_Library PUBLIC HPX::hpx)
endif()