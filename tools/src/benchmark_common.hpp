/// @copyright Copyright (c) Tim Niederhausen (tim@rnc-ag.de)
///
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#pragma once

#include "dataset_conversion.hpp"

#include <solarsim/simulation_state.hpp>
#include <solarsim/body_definition_csv.hpp>

// Enable optional spirit debugging
// #define BOOST_SPIRIT_DEBUG
#include <boost/spirit/home/x3.hpp>

#include <gflags/gflags.h>
#include <benchmark/benchmark.h>

SOLARSIM_NS_BEGIN

enum class Scaling
{
  // i.e. fixed work per thread
  Weak,
  // i.e. fixed work over all threads
  Strong
};

//
// Benchmark parameters
//

bool parse_threads(const char* flagname, const std::string& value);

DEFINE_double(time_step, 60 * 60, "Time between simulation steps (in s)");
DEFINE_double(duration, (60 * 60) * 15, "Total duration of the simulation (in s)");
DEFINE_string(dataset, "dataset/scenario1_state_vectors.csv", "Path to the input state vectors");
DEFINE_string(threads, "2,4,8,16", "Number of threads to test");
DEFINE_validator(threads, &parse_threads);
static std::vector<int> FLAGS_threads_v;

bool parse_threads(const char* flagname, const std::string& value)
{
  (void)flagname;
  auto iter = value.begin();
  return phrase_parse(iter, value.end(), -(boost::spirit::x3::int_ % ','), boost::spirit::x3::space, FLAGS_threads_v);
}

// Helper type to own our simulation dataset
template <typename DatasetPolicy>
struct benchmark_simulator_data
{
  benchmark_simulator_data(const std::string& filename, bool need_norm)
  {
    auto dataset = load_from_csv_file(filename);

    if (need_norm) {
      for (auto& body : dataset)
        DatasetPolicy::normalize_body_values(body);
    }
    adjust_initial_velocities(dataset);

    // Next, decompose the bodies into what we need!
    // Our algorithms are decoupled from the body_definition type.
    state.body_positions.resize(dataset.size());
    state.body_velocities.resize(dataset.size());
    state.body_masses.resize(dataset.size());
    state.acceleration.resize(dataset.size());
    state.softening_factor = .05;

    for (std::size_t i = 0, n = dataset.size(); i != n; ++i) {
      state.body_positions[i]  = dataset[i].position;
      state.body_velocities[i] = dataset[i].velocity;
      state.body_masses[i]     = dataset[i].mass;
    }
  }

  simulation_state state;
};

static const simulation_state& get_problem()
{
  // <static const> gives us "free" on-demand thread safe init for our static dataset
  // Dataset selection:
  // static const benchmark_simulator_data<ipvs_dataset> d("planets_and_moons_state_vectors.csv", true);
  static const benchmark_simulator_data<ipvs_dataset> d(FLAGS_dataset, true);
  return d.state;
}

// Benchmark helpers

using benchmark_function_type = void(benchmark::State&);

inline void register_solarsim_benchmark(const std::string& name, benchmark_function_type function)
{
  const auto registration = benchmark::RegisterBenchmark(name, function)->UseRealTime()->MeasureProcessCPUTime();
  for (auto num_threads : FLAGS_threads_v)
    registration->Arg(num_threads);
}

SOLARSIM_NS_END
