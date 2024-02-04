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

install(
    TARGETS SolarSim_Library
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

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
