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
#ifndef SOLARSIM_BARNESHUTOCTREE_HPP
#define SOLARSIM_BARNESHUTOCTREE_HPP

#include "solarsim/detail/config.hpp"

#if SOLARSIM_HAS_PRAGMA_ONCE
#  pragma once
#endif

#include "solarsim/types.hpp"

#include <array>
#include <span>
#include <memory>

SOLARSIM_NS_BEGIN

struct barnes_hut_octree_node
{
  barnes_hut_octree_node() = default;

  constexpr barnes_hut_octree_node(const triple& position, real length)
    : position(position)
    , length(length)
  {
  }

  [[nodiscard]] bool is_leaf() const noexcept { return !children[0]; }

  barnes_hut_octree_node& get_child_for_position(const triple& pos) const;
  void insert_body(const triple& body_position, real body_mass);

  void merge_from(barnes_hut_octree_node&& other);

  template <typename F>
  void recursively_apply_node_gravity(const triple& body_position, real softening, F&& apply_gravity) const;

  void finalize();

private:
  void subdivide_node();

  // Octree data
  triple position = {}; // top-left corner
  real length     = 0.0;
  std::array<std::unique_ptr<barnes_hut_octree_node>, 8> children;

  // Barnes-Hut bounds for this node
  real total_mass       = 0.0;
  triple center_of_mass = {};

  // Contained object (if any)
  bool has_contained_body        = false;
  triple contained_body_position = {};
  real contained_body_mass       = 0.0;
};

axis_aligned_bounding_box build_bounding_box(std::span<const triple> positions);

class partial_barnes_hut_octree
{
public:
  partial_barnes_hut_octree() = default;
  partial_barnes_hut_octree(const axis_aligned_bounding_box& bounds, std::span<const triple> body_positions,
                            std::span<const real> body_masses);

protected:
  partial_barnes_hut_octree(const axis_aligned_bounding_box& bounds);

  barnes_hut_octree_node root_ = {};
};

class barnes_hut_octree : public partial_barnes_hut_octree
{
public:
  barnes_hut_octree() = default;
  barnes_hut_octree(const axis_aligned_bounding_box& bounds, std::span<const triple> body_positions,
                    std::span<const real> body_masses);

  /**
   * Create a new octree from a sequence of partial trees with the same bounds.
   * @param bounds Bounding box of this tree and all partial trees.
   * @param partial_trees Sequence of partial trees to merge.
   */
  barnes_hut_octree(const axis_aligned_bounding_box& bounds, std::span<barnes_hut_octree> partial_trees);

  // simple interface for single-threaded usage
  barnes_hut_octree(std::span<const triple> body_positions, std::span<const real> body_masses);

  void apply_forces_to(const triple& body_position, real softening, triple& acceleration) const;
};

SOLARSIM_NS_END

#endif
