#include "solarsim/body_definition_csv.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <sstream>

SOLARSIM_NS_BEGIN

// This is much better for testing!
extern std::vector<body_definition> load_from_csv_file(std::istream& input);

TEST_CASE("parse_headers_and_empty", "body_definition_csv")
{
  const std::string empty_csv = R"(id,name,class,mass,pos_x,pos_y,pos_z,vel_x,vel_y,vel_z)";
  std::istringstream tmp_stream(empty_csv);
  REQUIRE(load_from_csv_file(tmp_stream).size() == 0);
}

TEST_CASE("parse_single", "body_definition_csv")
{
  const std::string single_csv = R"(id,name,class,mass,pos_x,pos_y,pos_z,vel_x,vel_y,vel_z
0,Sun,STA,1.988469999999999977e+30,0,0,0,0,0,0)";
  std::istringstream tmp_stream(single_csv);

  const auto bodies = load_from_csv_file(tmp_stream);
  REQUIRE(bodies.size() == 1);
  REQUIRE(bodies[0].id == "0");
  REQUIRE(bodies[0].name == "Sun");
  REQUIRE(bodies[0].type == "STA");
  REQUIRE_THAT(bodies[0].mass, Catch::Matchers::WithinAbs(1.988469999999999977e+30, 0.0001));
  REQUIRE_THAT(bodies[0].position[0], Catch::Matchers::WithinAbs(0.0, 0.0001));
  REQUIRE_THAT(bodies[0].position[1], Catch::Matchers::WithinAbs(0.0, 0.0001));
  REQUIRE_THAT(bodies[0].position[2], Catch::Matchers::WithinAbs(0.0, 0.0001));
  REQUIRE_THAT(bodies[0].velocity[0], Catch::Matchers::WithinAbs(0.0, 0.0001));
  REQUIRE_THAT(bodies[0].velocity[1], Catch::Matchers::WithinAbs(0.0, 0.0001));
  REQUIRE_THAT(bodies[0].velocity[2], Catch::Matchers::WithinAbs(0.0, 0.0001));
}

SOLARSIM_NS_END
