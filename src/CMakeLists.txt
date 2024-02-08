add_library(
    SolarSim_Library
    log.cpp
    body_definition_csv.cpp
)
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
target_link_libraries(SolarSim_Library PRIVATE Boost::boost Boost::system)
