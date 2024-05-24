find_package(fmt CONFIG REQUIRED)
find_package(HPX)
find_package(stdexec CONFIG)
find_package(benchmark CONFIG)

# Sequential runner
add_executable(
    SolarSim_cli_std
    src/dataset_conversion.hpp
    src/std_main.cpp
)

target_link_libraries(SolarSim_cli_std PUBLIC SolarSim_Library PRIVATE fmt::fmt)

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
endif()

# Benchmark tool
if(HPX_FOUND AND stdexec_FOUND AND benchmark_FOUND)
  add_executable(
      SolarSim_benchmark
      src/dataset_conversion.hpp
      src/benchmark_main.cpp
  )
  target_link_libraries(SolarSim_benchmark
    PUBLIC SolarSim_Library
    PRIVATE fmt::fmt HPX::hpx HPX::wrap_main HPX::iostreams_component STDEXEC::stdexec benchmark::benchmark
  )
endif()
