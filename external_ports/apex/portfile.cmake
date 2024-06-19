vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO UO-OACISS/apex
    REF "v${VERSION}"
    SHA512 df08512e333f1fec6605a0e2e3e4a7399692b6f5ff084a6cc453a1ea928dd9377ef5ee8c9af6686f7e63ba3bba17a96de9fa2de66c9692b138105f32e2be5843
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    DISABLE_PARALLEL_CONFIGURE
    OPTIONS
        -DAPEX_WITH_KOKKOS=OFF
        -DVCPKG_HOST_TRIPLET=${_HOST_TRIPLET}
)
vcpkg_cmake_install()

vcpkg_fixup_pkgconfig()
