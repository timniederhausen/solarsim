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
#include "solarsim/math.hpp"

#include <algorithm>
#include <span>
#include <cassert>

SOLARSIM_NS_BEGIN

barnes_hut_octree_node& barnes_hut_octree_node::get_child_for_position(const triple& pos) const
{
  // Make really sure we're not called with a position outside this node's bounds!
  constexpr real epsilon = 0.00001; // Use some hardcoded epsilon to account for inaccuracies.
  assert(pos[0] >= position[0] - epsilon);
  assert(pos[1] >= position[1] - epsilon);
  assert(pos[2] >= position[2] - epsilon);
  assert(pos[0] <= position[0] + length + epsilon);
  assert(pos[1] <= position[1] + length + epsilon);
  assert(pos[2] <= position[2] + length + epsilon);

  const triple center        = position + length / 2;
  const std::size_t offset_x = 4 * static_cast<std::size_t>(pos[0] >= center[0]);
  const std::size_t offset_y = 2 * static_cast<std::size_t>(pos[1] >= center[1]);
  const std::size_t offset_z = 1 * static_cast<std::size_t>(pos[2] >= center[2]);
  return *children[offset_x + offset_y + offset_z];
}

void barnes_hut_octree_node::insert_body(const triple& body_position, real body_mass)
{
  if (is_leaf()) {
    if (has_contained_body) {
      subdivide_node();

      // We had a body in the node we just subdivided? place that first!
      get_child_for_position(contained_body_position).insert_body(contained_body_position, contained_body_mass);
      has_contained_body = false;

      // Now place what we've been asked to place
      get_child_for_position(body_position).insert_body(body_position, body_mass);
    } else {
      has_contained_body      = true;
      contained_body_position = body_position;
      contained_body_mass     = body_mass;
    }
  } else {
    get_child_for_position(body_position).insert_body(body_position, body_mass);
  }
  total_mass += body_mass;
}

void barnes_hut_octree_node::merge_from(barnes_hut_octree_node&& other)
{
  assert(almost_equal_ulps(other.position, position));
  assert(almost_equal_ulps(other.length, length));

  if (!is_leaf()) {
    // Simple case: We're both branches - just merge our children
    if (!other.is_leaf()) {
      real new_total_mass = 0;
      for (int i = 0; i < 8; ++i) {
        children[i]->merge_from(std::move(*other.children[i]));
        new_total_mass += children[i]->total_mass;
      }
      total_mass = new_total_mass;
      return;
    }

    // Still the easiest path - just get the correct child and insert there.
    insert_body(other.contained_body_position, other.contained_body_mass);
    return;
  }

  if (other.is_leaf()) {
    insert_body(other.contained_body_position, other.contained_body_mass);
    return;
  }

  children = std::move(other.children);

  // We had a body in the node we just subdivided? place that first!
  get_child_for_position(contained_body_position).insert_body(contained_body_position, contained_body_mass);
  has_contained_body = false;

  total_mass += other.total_mass;
}

template <typename F>
void barnes_hut_octree_node::recursively_apply_node_gravity(const triple& body_position, real softening,
                                                            F&& apply_gravity) const
{
  constexpr real theta = 0.5;

  const real distance_to_center = ::solarsim::length(center_of_mass - body_position) + softening;
  if (length / distance_to_center < theta) {
    // It's far enough away that our approximation is sufficient.
    apply_gravity(center_of_mass, total_mass);
    return;
  }

  // Leaf nodes apply their body's force
  if (is_leaf()) {
    if (has_contained_body) {
      apply_gravity(contained_body_position, contained_body_mass);
    }
    return;
  }

  // Otherwise, descend into our children
  for (auto& child : children) {
    if (!child->is_leaf() || child->has_contained_body)
      child->recursively_apply_node_gravity(body_position, softening, std::forward<F>(apply_gravity));
  }
}

void barnes_hut_octree_node::finalize()
{
  if (is_leaf()) {
    if (has_contained_body)
      center_of_mass = contained_body_position * contained_body_mass;
  } else {
    triple mass_centers_sum = {};
    for (auto& child : children) {
      if (!child->is_leaf() || child->has_contained_body) {
        child->finalize();
        mass_centers_sum += child->center_of_mass * child->total_mass;
      }
    }
    center_of_mass = mass_centers_sum / total_mass;
    debug_validate_finite(center_of_mass);
  }
}

void barnes_hut_octree_node::subdivide_node()
{
  assert(is_leaf());          // can't divide a non-leaf
  assert(has_contained_body); // why else would subdivide?

  // This order is special
  // 1) it alternates between +0 / +half on the Z-axis
  // 2) these groups of two alternate between +0 / +half on the Y-axis
  // 3) these groups of four alternate between +0 / +half on the X-axis
  // see get_child_node()
  const real half_length        = length / 2;
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
    children[index] = std::make_unique<barnes_hut_octree_node>(position + child_offsets[index], half_length);
  }
}

namespace {

barnes_hut_octree_node setup_root_node_with_bounds(const axis_aligned_bounding_box& aabb)
{
  debug_validate_finite(aabb.min);
  debug_validate_finite(aabb.max);

  // Find (position, length) that encompasses this AABB
  // We don't really care about wasting space (i.e. making a larger bounding area than necessary)
  const triple center = (aabb.min + aabb.max) * 0.5;
  const triple d      = aabb.max - aabb.min;
  const real length   = std::max({d[0], d[1], d[2]});
  assert(std::isfinite(length));

  return barnes_hut_octree_node{center - (length * 0.5), length};
}

} // namespace

axis_aligned_bounding_box build_bounding_box(std::span<const triple> positions)
{
  axis_aligned_bounding_box aabb = axis_aligned_bounding_box::infinity();
  for (const auto& position : positions) {
    aabb.min[0] = std::min(aabb.min[0], position[0]);
    aabb.min[1] = std::min(aabb.min[1], position[1]);
    aabb.min[2] = std::min(aabb.min[2], position[2]);

    aabb.max[0] = std::max(aabb.max[0], position[0]);
    aabb.max[1] = std::max(aabb.max[1], position[1]);
    aabb.max[2] = std::max(aabb.max[2], position[2]);
  }
  return aabb;
}

partial_barnes_hut_octree::partial_barnes_hut_octree(const axis_aligned_bounding_box& bounds,
                                                     std::span<const triple> body_positions,
                                                     std::span<const real> body_masses)
  : root_(setup_root_node_with_bounds(bounds))
{
  assert(body_positions.size() == body_masses.size());
  for (std::size_t i = 0, n = body_positions.size(); i < n; ++i)
    root_.insert_body(body_positions[i], body_masses[i]);
}

partial_barnes_hut_octree::partial_barnes_hut_octree(const axis_aligned_bounding_box& bounds)
  : root_(setup_root_node_with_bounds(bounds))
{
  // real setup happens in child classes
}

barnes_hut_octree::barnes_hut_octree(const axis_aligned_bounding_box& bounds, std::span<const triple> body_positions,
                                     std::span<const real> body_masses)
  : partial_barnes_hut_octree(bounds, body_positions, body_masses)
{
  root_.finalize();
}

barnes_hut_octree::barnes_hut_octree(const axis_aligned_bounding_box& bounds,
                                     std::span<barnes_hut_octree> partial_trees)
  : partial_barnes_hut_octree(bounds)
{
  // Destructively merge all other trees into this one.
  for (auto& tree : partial_trees)
    root_.merge_from(std::move(tree.root_));

  root_.finalize();
}

barnes_hut_octree::barnes_hut_octree(std::span<const triple> body_positions, std::span<const real> body_masses)
  : barnes_hut_octree(build_bounding_box(body_positions), body_positions, body_masses)
{
}

void barnes_hut_octree::apply_forces_to(const triple& body_position, real softening, triple& acceleration) const
{
  auto apply_gravity = [&](const triple& node_position, real node_mass) {
    calculate_acceleration(body_position, node_position, node_mass, softening, acceleration);
  };
  root_.recursively_apply_node_gravity(body_position, softening, apply_gravity);
}

SOLARSIM_NS_END
