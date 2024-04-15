# -- std runner
add_executable(
    SolarSim_cli_std
    src/std_main.cpp
)

target_link_libraries(SolarSim_cli_std PUBLIC SolarSim_Library)

find_package(HPX)
if(HPX_FOUND)
  # -- HPX runner
  add_executable(
      SolarSim_cli_hpx
      src/hpx_main.cpp
  )
  target_link_libraries(SolarSim_cli_hpx
    PUBLIC SolarSim_Library
    PRIVATE HPX::hpx HPX::wrap_main HPX::iostreams_component
  )
endif()