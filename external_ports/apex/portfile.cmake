vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO UO-OACISS/apex
    # we need something newer than 2.6.5 that includes:
    # see https://github.com/UO-OACISS/apex/commit/e3bcade6d1b9b6de91ad09b2443a115bc05e5700
    REF "615886778ee84a62f3f4dd30e591128b1ae89652"
    SHA512 c16faba2987dafa49ef2fa0f9b343c7c0ba78678e705cc03485a875b157b6075b33c5ccb1024325b63f66ddd81851fb1c45fc5534e200e9b7e4d74cec655ded7
    HEAD_REF master
    PATCHES
        fix-add-cstdint.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    DISABLE_PARALLEL_CONFIGURE
    OPTIONS
        -DAPEX_WITH_KOKKOS=OFF
        -DVCPKG_HOST_TRIPLET=${_HOST_TRIPLET}
)
vcpkg_cmake_install()

# post build cleanup
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/APEX)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_fixup_pkgconfig()
