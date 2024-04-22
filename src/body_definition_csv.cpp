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
#include "solarsim/body_definition_csv.hpp"

// Enable optional spirit debugging
// #define BOOST_SPIRIT_DEBUG
#include <boost/spirit/include/qi.hpp>

#include <fmt/format.h>
#include <fmt/os.h>

#include <fstream>

SOLARSIM_NS_BEGIN

namespace px = boost::phoenix;
namespace qi = boost::spirit::qi;

// A possibly more-complex-than-needed CSV parser for our celestial bodies
// see: https://stackoverflow.com/a/18366335
// see: https://stackoverflow.com/a/7462539
template <typename Iterator = std::string_view::const_iterator, char ColSep = ','>
struct bodydef_csv_grammar : qi::grammar<Iterator, body_definition()>
{
  bodydef_csv_grammar()
    : bodydef_csv_grammar::base_type(start)
  {
    using namespace qi;

    // TODO: I'm sure there's a way to build this from the adapted struct
    // might no be worth implementing though :(
    // clang-format off
    start  = column >> ColSep >> column >> ColSep >> column >> ColSep >>
             double_ >> ColSep >> // mass
             double_ >> ColSep >> double_ >> ColSep >> double_ >> ColSep >> // position (x y z)
             double_ >> ColSep >> double_ >> ColSep >> double_; // velocity (x y z)
    column = quoted | *~char_(ColSep);
    quoted = '"' >> *("\"\"" | ~char_('"')) >> '"';
    // clang-format on

    BOOST_SPIRIT_DEBUG_NODES((start)(column)(quoted))
  }

private:
  qi::rule<Iterator, body_definition()> start;
  qi::rule<Iterator, std::string()> column;
  qi::rule<Iterator, std::string()> quoted;
};

template <typename Parser, typename... Args>
void parse_or_die(std::string_view input, const Parser& p, Args&&... args)
{
  std::string_view::const_iterator begin = input.begin(), end = input.end();
  bool ok = qi::parse(begin, end, p, std::forward<Args>(args)...);
  if (!ok || begin != end)
    throw std::runtime_error(fmt::format("Parse error: remaining {}", std::string_view(begin, end)));
}

std::vector<body_definition> load_from_csv_file(std::istream& input)
{
  std::string line;
  if (!std::getline(input, line))
    throw std::runtime_error("failed to read headers");
  // TODO: validate header

  std::vector<body_definition> bodies;
  for (bodydef_csv_grammar<> grammar; std::getline(input, line);) {
    body_definition body;
    parse_or_die(line, grammar, body);
    bodies.push_back(std::move(body));
  }
  return bodies;
}

std::vector<body_definition> load_from_csv_file(zstring_view filename)
{
  std::ifstream istrm(filename.c_str(), std::ios::binary);
  if (!istrm.is_open())
    throw std::runtime_error(fmt::format("failed to read {}", filename.c_str()));

  return load_from_csv_file(istrm);
}

void save_to_csv_file(std::span<const body_definition> bodies, zstring_view filename)
{
  auto out = fmt::output_file(filename.c_str());
  out.print("id,name,class,mass,pos_x,pos_y,pos_z,vel_x,vel_y,vel_z\n");
  for (const auto& body : bodies) {
    // This doesn't look very maintainable. Maybe generate it with crapi?
    out.print("{},{},{},{},{},{},{},{},{},{}\n", body.id, body.name, body.type, body.mass, body.position[0],
              body.position[1], body.position[2], body.velocity[0], body.velocity[1], body.velocity[2]);
  }
}

SOLARSIM_NS_END
