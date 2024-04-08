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
#include "solarsim/barnes_hut_octree.hpp"
#include "solarsim/body_definition.hpp"
#include "solarsim/math.hpp"

#include <span>
#include <cassert>

SOLARSIM_NS_BEGIN

namespace {

axis_aligned_bounding_box build_bounding_box(std::span<const body_definition> bodies)
{
  axis_aligned_bounding_box aabb = axis_aligned_bounding_box::infinity();
  for (const auto& body : bodies) {
    aabb.min[0] = std::min(aabb.min[0], body.position[0]);
    aabb.min[1] = std::min(aabb.min[1], body.position[1]);
    aabb.min[2] = std::min(aabb.min[2], body.position[2]);

    aabb.max[0] = std::max(aabb.max[0], body.position[0]);
    aabb.max[1] = std::max(aabb.max[1], body.position[1]);
    aabb.max[2] = std::max(aabb.max[2], body.position[2]);
  }
  return aabb;
}

barnes_hut_octree_node& get_child_node(const barnes_hut_octree_node& node, const triple& pos)
{
  // Make really sure we're not called with a position outside this node's bounds!
  // Use some hardcoded epsilon to account for inaccuracies.
  assert(pos[0] >= node.position[0] - 0.00001);
  assert(pos[1] >= node.position[1] - 0.00001);
  assert(pos[2] >= node.position[2] - 0.00001);
  assert(pos[0] < node.position[0] + node.length + 0.00001);
  assert(pos[1] < node.position[1] + node.length + 0.00001);
  assert(pos[2] < node.position[2] + node.length + 0.00001);

  const triple center        = node.position + node.length / 2;
  const std::size_t offset_x = 4 * static_cast<std::size_t>(pos[0] >= center[0]);
  const std::size_t offset_y = 2 * static_cast<std::size_t>(pos[1] >= center[1]);
  const std::size_t offset_z = 1 * static_cast<std::size_t>(pos[2] >= center[2]);
  return *node.children[offset_x + offset_y + offset_z];
}

void subdivide_node(barnes_hut_octree_node& node)
{
  assert(node.is_leaf());      // can't divided a non-leaf
  assert(node.contained_body); // why else would be sub-divide?

  // This order is special
  // 1) it alternates between +0 / +half on the Z-axis
  // 2) these groups of two alternate between +0 / +half on the Y-axis
  // 3) these groups of four alternate between +0 / +half on the X-axis
  // see get_child_node()
  const real half_length        = node.length / 2;
  const triple child_offsets[8] = {
      triple{          0,           0,           0},
      triple{          0,           0, half_length},

      triple{          0, half_length,           0},
      triple{          0, half_length, half_length},

      triple{half_length,           0,           0},
      triple{half_length,           0, half_length},

      triple{half_length, half_length,           0},
      triple{half_length, half_length, half_length},
  };

  for (std::size_t index = 0; index != 8; ++index) {
    node.children[index] = std::make_unique<barnes_hut_octree_node>(node.position + child_offsets[index], half_length);
  }
}

void insert_body(barnes_hut_octree_node& node, const body_definition& body)
{
  if (node.is_leaf()) {
    if (node.contained_body) {
      subdivide_node(node);

      // We had a body in the node we just sub-divided? place that first!
      insert_body(get_child_node(node, node.contained_body->position), *node.contained_body);
      node.contained_body = nullptr;

      // Now place what we've been asked to place
      insert_body(get_child_node(node, body.position), body);
    } else {
      node.contained_body = &body;
    }
  } else {
    insert_body(get_child_node(node, body.position), body);
  }
  node.total_mass += body.mass;
}

barnes_hut_octree_node setup_root_node_with_bounds(const axis_aligned_bounding_box& aabb)
{
  // Find (position, length) that encompasses this AABB
  // We don't really care about wasting space (i.e. making a larger bounding area than necessary)
  const triple center = (aabb.min + aabb.max) * 0.5;
  const triple d      = aabb.max - aabb.min;
  const real length   = std::max({d[0], d[1], d[2]});

  return barnes_hut_octree_node{center - (length * 0.5), length};
}

void apply_node_gravity_to_body(const barnes_hut_octree_node& node, const body_definition& body, real softening,
                                triple& acceleration)
{
  constexpr real theta          = 0.5;
  const real distance_to_center = length(node.center_of_mass - body.position) + softening;

  if (node.length / distance_to_center < theta) {
    // It's far enough away that our approximation suffices!
    return calculate_acceleration(body.position, node.center_of_mass, node.total_mass, softening, acceleration);
  }

  // Leafs apply their bodies' force
  if (node.is_leaf()) {
    if (node.contained_body) {
      calculate_acceleration(body.position, node.contained_body->position, node.contained_body->mass, softening,
                             acceleration);
    }
    return;
  }

  // Otherwise, descend into our children
  for (auto& child : node.children) {
    assert(!!child);
    apply_node_gravity_to_body(*child, body, softening, acceleration);
  }
}

} // namespace

barnes_hut_octree::barnes_hut_octree(std::span<const body_definition> bodies)
  : bounds_(build_bounding_box(bodies))
  , root_(setup_root_node_with_bounds(bounds_))
{
  for (const auto& body : bodies)
    insert_body(root_, body);
}

void barnes_hut_octree::apply_forces_to(const body_definition& body, real softening, triple& acceleration) const
{
  apply_node_gravity_to_body(root_, body, softening, acceleration);
}

SOLARSIM_NS_END
