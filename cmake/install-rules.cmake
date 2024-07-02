if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/SolarSim-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package SolarSim)

# We need transitive runtime-deps as well!
# see: https://stackoverflow.com/a/75065206
set(exe_targets "")
if(TARGET SolarSim_cli_std)
  list(APPEND exe_targets SolarSim_cli_std)
  install(FILES $<TARGET_RUNTIME_DLLS:SolarSim_cli_std> TYPE BIN)
endif()
if(TARGET SolarSim_cli_hpx)
  list(APPEND exe_targets SolarSim_cli_hpx)
  install(FILES $<TARGET_RUNTIME_DLLS:SolarSim_cli_hpx> TYPE BIN)
endif()
if(TARGET SolarSim_benchmark)
  list(APPEND exe_targets SolarSim_benchmark)
  install(FILES $<TARGET_RUNTIME_DLLS:SolarSim_benchmark> TYPE BIN)
endif()
if(TARGET SolarSim_benchmark_std)
  list(APPEND exe_targets SolarSim_benchmark_std)
  install(FILES $<TARGET_RUNTIME_DLLS:SolarSim_benchmark_std> TYPE BIN)
endif()

install(
    TARGETS SolarSim_Library ${exe_targets}
    EXPORT SolarSimTargets
    RUNTIME #
    COMPONENT SolarSim_Runtime
    LIBRARY #
    COMPONENT SolarSim_Runtime
    NAMELINK_COMPONENT SolarSim_Development
    ARCHIVE #
    COMPONENT SolarSim_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

if (NOT WIN32)
  install(IMPORTED_RUNTIME_ARTIFACTS ${exe_targets} RUNTIME_DEPENDENCY_SET _dependency_set)
  # FIXME: CONFLICTING_DEPENDENCIES_PREFIX  _conflicts
  install(
      RUNTIME_DEPENDENCY_SET _dependency_set
      PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
      POST_EXCLUDE_REGEXES "${CMAKE_INSTALL_PREFIX}/lib"
      RUNTIME DESTINATION lib)
endif()

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    SolarSim_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE SolarSim_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(SolarSim_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${SolarSim_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT SolarSim_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${SolarSim_INSTALL_CMAKEDIR}"
    COMPONENT SolarSim_Development
)

install(
    EXPORT SolarSimTargets
    NAMESPACE SolarSim::
    DESTINATION "${SolarSim_INSTALL_CMAKEDIR}"
    COMPONENT SolarSim_Development
)

include(InstallRequiredSystemLibraries)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
