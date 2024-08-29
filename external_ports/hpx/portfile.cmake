if(VCPKG_TARGET_IS_WINDOWS)
    vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)
endif()
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" HPX_WITH_STATIC_LINKING)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO STEllAR-GROUP/hpx
    REF "e43a4ec0a218640b267e357ea2f0c04f2b24d84e"
    SHA512 335ab1311030520e7e2b51543b1c2f5b4fabc27deb27dd9b990fa9f4f6163f207e9408ba692ac928ac2e9942b7a36af73d47cef9afeccfa8b557d13d07da23c7
    HEAD_REF master
    PATCHES
        fix-find-apex-anywhere.patch
        use-my-apex-fork.patch
)

vcpkg_check_features(
    OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
    "zlib"              HPX_WITH_COMPRESSION_ZLIB
    "snappy"            HPX_WITH_COMPRESSION_SNAPPY
    "bzip2"             HPX_WITH_COMPRESSION_BZIP2
    "cuda"              HPX_WITH_CUDA
    "mpi"               HPX_WITH_PARCELPORT_MPI
    "mpi"               HPX_WITH_PARCELPORT_MPI_MULTITHREADED
)

if(NOT VCPKG_TARGET_ARCHITECTURE MATCHES "(x64|x86)")
    list(APPEND FEATURE_OPTIONS "-DHPX_WITH_GENERIC_CONTEXT_COROUTINES=ON")
endif()

# Enable APEX on non-Windows
if(NOT VCPKG_TARGET_IS_WINDOWS)
    list(APPEND FEATURE_OPTIONS
        "-DHPX_WITH_APEX=ON"
        "-DHPX_WITH_FETCH_APEX=ON"
        "-DHPX_WITH_APEX_TAG=a81cc9fc1b0169aafe4fe3fd12b81dd31afe896a")
else()
    list(APPEND FEATURE_OPTIONS
        "-DCMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS=1"
        "-DCMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS=1"
        "-DCMAKE_C_RESPONSE_FILE_LINK_FLAG=@"
        "-DCMAKE_CXX_RESPONSE_FILE_LINK_FLAG=@"
        "-DCMAKE_NINJA_FORCE_RESPONSE_FILE=1")
endif()

file(REMOVE "${SOURCE_PATH}/cmake/FindBZip2.cmake") # Outdated

# see: https://hpx-docs.stellar-group.org/latest/html/manual/cmake_variables.html
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    DISABLE_PARALLEL_CONFIGURE
    OPTIONS
        -DHPX_WITH_VCPKG=ON
        -DHPX_WITH_TESTS=OFF
        -DHPX_WITH_EXAMPLES=OFF
        -DHPX_WITH_TOOLS=OFF
        -DHPX_WITH_RUNTIME=OFF
        -DHPX_USE_CMAKE_CXX_STANDARD=ON
        ${FEATURE_OPTIONS}
        -DHPX_WITH_PKGCONFIG=OFF
        #-DHPX_WITH_STATIC_LINKING=${HPX_WITH_STATIC_LINKING}
        -DHPX_WITH_PARCELPORT_TCP=ON
        -DHPX_WITH_THREAD_TARGET_ADDRESS=ON
        -DHPX_WITH_CHECK_MODULE_DEPENDENCIES=ON
        -DHPX_WITH_THREAD_IDLE_RATES=ON
        # Otherwise the checks are pretty useless
        -DHPX_WITH_VERIFY_LOCKS_BACKTRACE=ON
        # IPVS has big systems
        -DHPX_WITH_MAX_CPU_COUNT=256
        # We are usually starved, especially if actually running in other runtimes (e.g. TBB, stdexec)
        -DHPX_THREAD_BACKOFF_ON_IDLE=ON
        # Use new stdexec impl.
        -DHPX_WITH_CXX_STANDARD=20
        -DHPX_WITH_STDEXEC=ON
        -DVCPKG_HOST_TRIPLET=${_HOST_TRIPLET}
        # Have this last so it overrides the previous ...=ON
        # HPX uses FetchContent for some optional features (APEX, ...)
        # see: https://learn.microsoft.com/en-us/vcpkg/troubleshoot/build-failures#fetchcontent-dependency-is-not-found-during-build-process
        -DFETCHCONTENT_FULLY_DISCONNECTED=OFF
)
vcpkg_cmake_install()

# post build cleanup
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/HPX)

file(GLOB_RECURSE CMAKE_FILES "${CURRENT_PACKAGES_DIR}/share/hpx/*.cmake")
foreach(CMAKE_FILE IN LISTS CMAKE_FILES)
    file(READ ${CMAKE_FILE} _contents)
    string(REGEX REPLACE
        "lib/([A-Za-z0-9_.-]+\\.dll)"
        "bin/\\1"
        _contents "${_contents}")
    string(REGEX REPLACE
        "lib/hpx/([A-Za-z0-9_.-]+\\.dll)"
        "bin/hpx/\\1"
        _contents "${_contents}")
    file(WRITE ${CMAKE_FILE} "${_contents}")
endforeach()

vcpkg_replace_string(
    "${CURRENT_PACKAGES_DIR}/share/${PORT}/HPXConfig.cmake"
    "set(HPX_BUILD_TYPE \"Release\")"
    "set(HPX_BUILD_TYPE \"\${CMAKE_BUILD_TYPE}\")")

vcpkg_replace_string(
    "${CURRENT_PACKAGES_DIR}/share/${PORT}/HPXMacros.cmake"
    "set(CMAKE_MODULE_PATH \${CMAKE_MODULE_PATH}"
    "list(APPEND CMAKE_MODULE_PATH")

file(INSTALL
    "${SOURCE_PATH}/LICENSE_1_0.txt"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_fixup_pkgconfig()

file(REMOVE "${CURRENT_PACKAGES_DIR}/bin/hpxcxx" "${CURRENT_PACKAGES_DIR}/debug/bin/hpxcxx")

if(EXISTS "${CURRENT_PACKAGES_DIR}/bin/hpxrun.py")
    file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/tools/${PORT}")
    file(RENAME "${CURRENT_PACKAGES_DIR}/bin/hpxrun.py" "${CURRENT_PACKAGES_DIR}/tools/${PORT}/hpxrun.py")
    vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/tools/${PORT}/hpxrun.py" "'${CURRENT_INSTALLED_DIR}/tools/openmpi/bin/mpiexec'" "'mpiexec'")
endif()

if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/bin" "${CURRENT_PACKAGES_DIR}/debug/bin")
endif()

configure_file("${CMAKE_CURRENT_LIST_DIR}/usage" "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" COPYONLY)
