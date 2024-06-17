find_package(fmt CONFIG REQUIRED)
find_package(HPX)
find_package(stdexec CONFIG)
find_package(benchmark CONFIG)
set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG)

# Sequential runner
add_executable(
    SolarSim_cli_std
    src/dataset_conversion.hpp
    src/std_main.cpp
)
target_link_libraries(SolarSim_cli_std PUBLIC SolarSim_Library PRIVATE fmt::fmt)

if(NOT CMAKE_SKIP_INSTALL_RULES)
  install (TARGETS SolarSim_cli_std)
endif()

# HPX runner
if(HPX_FOUND AND stdexec_FOUND)
  add_executable(
      SolarSim_cli_hpx
      src/dataset_conversion.hpp
      src/hpx_main.cpp
  )
  target_link_libraries(SolarSim_cli_hpx
    PUBLIC SolarSim_Library
    PRIVATE fmt::fmt HPX::hpx HPX::wrap_main HPX::iostreams_component
  )

  if(NOT CMAKE_SKIP_INSTALL_RULES)
    install (TARGETS SolarSim_cli_hpx)
  endif()
endif()

# Benchmark tool
if(HPX_FOUND AND stdexec_FOUND AND benchmark_FOUND AND gflags_FOUND)

  add_executable(
      SolarSim_benchmark
      src/dataset_conversion.hpp
      src/benchmark_main.cpp
  )
  target_link_libraries(SolarSim_benchmark
    PUBLIC SolarSim_Library
    PRIVATE fmt::fmt HPX::hpx HPX::wrap_main HPX::iostreams_component STDEXEC::stdexec benchmark::benchmark gflags::gflags
  )

  if(NOT CMAKE_SKIP_INSTALL_RULES)
    install (TARGETS SolarSim_benchmark)
  endif()
endif()
