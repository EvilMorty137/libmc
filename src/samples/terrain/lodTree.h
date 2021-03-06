/*
 * Copyright (c) 2016 Jonathan Glines
 * Jonathan Glines <jonathan@glines.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MC_SAMPLES_TERRAIN_LOD_TREE_H_
#define MC_SAMPLES_TERRAIN_LOD_TREE_H_

#include <memory>
#include <glm/glm.hpp>

namespace mc { namespace samples { namespace terrain {
  class Terrain;
  class TerrainMesh;
  /**
   * Class implementing an octree structure that keeps track of the level of
   * detail of the terrain meshes generated.
   */
  class LodTree {
    public:
      /**
       * Integer coordinates used to describe node and voxel block positions
       * within the level of detail tree.
       */
      typedef struct Coordinates {
        int x, y, z;
      } Coordinates;
      /**
       * A node within a level of detail tree. This node represents a single
       * octant at some level of detail in the tree, and, at levels of detail
       * other than 0, contains up to eight children. This node resides at a
       * specific voxel block position, which is represented by integer
       * coordinates. These coordinates align to the level of detail for this
       * node.
       */
      class Node {
        friend LodTree;
        public:
          enum class MeshState {
            /** A pseudo state that indicates that that this node exists
             * outside of the terrain that we are generating. This state is not
             * used at the moment, but it might be useful in the future. */
            VOID,
            /** The initial state of the terrain mesh held by this node, which
             * indicates that we have not yet requested a terrain mesh to be
             * generated for this node. */
            INITIAL,
            /** This state indicates that this node has already requested that
             * its terrain mesh be generated by the terrain generation thread,
             * and a reply from the terrain generation thread is pending. */
            REQUESTED,
            /** This state indicates that the terrain mesh was generated and
             * received by this node. */
            GENERATED,
            /** This state is for nodes whose terrain mesh was generated prior
             * to some recent change in the scalar field that defines the
             * terrain surface. The terrain mesh should be re-generated in
             * order to correct the appearance of the terrain represented by
             * this node. */
            DIRTY,
          };
          /**
           * Enumeration of the different possible states for a given LOD node.
           * These states are used in the node state machine that determines
           * when to add meshes from the scene and when to remove meshes from
           * the scene.
           *
           * The mesh state transitions with the following state machine:
           * \dot
           * digraph meshstate {
           *   INITIAL -> REQUESTED;
           *   REQUESTED -> GENERATED;
           *   REQUESTED -> EMPTY;
           *   GENERATED -> CACHED;
           * };
           * \enddot
           *
           * \todo DIRTY mesh states should be considered.
           * \todo Some mechanism for discarding meshes that are made dirty
           * before they even have a chance to finish generating.
           *
           * The scene state can transition with the following state machine:
           * \dot
           * digraph scenestate {
           *   INITIAL -> REQUESTED
           *   REQUESTED -> DRAWABLE;
           *   REQUESTED -> SPLIT;
           *   REQUESTED -> EMPTY;
           *   EMPTY -> EMPTY_POPPED;
           *   EMPTY -> DRAWABLE [style=dashed];
           *   EMPTY -> SPLIT;
           *   EMPTY -> EMPTY;
           *   EMPTY_POPPED -> SPLIT_POPPED;
           *   EMPTY_POPPED -> POPPED [style=dashed];
           *   DRAWABLE -> EMPTY [style=dashed];
           *   DRAWABLE -> SPLIT;
           *   DRAWABLE -> POPPED;
           *   DRAWABLE -> DRAWABLE;
           *   POPPED -> SPLIT_POPPED;
           *   POPPED -> EMPTY_POPPED [style=dashed];
           *   SPLIT -> SPLIT_POPPED;
           * }
           * \enddot
           * The dashed lines indicate a transition that is only possible if
           * the terrain is being edited on the fly.
           */
          enum class State {
            /** This is a pseudo state used to indicate a node that exists
             * outside of the terrain that we are generating. This might be
             * useful to account for nodes that have siblings outside of the
             * terrain, but for now this case is impossible because all terrain
             * is generated in groups of eight siblings.
             */
            VOID,
            /** Node state that indicates that there are no meshes with which
             * to draw this node in the scene, and the terrain mesh for this
             * node has not been requested for generation.  */
            INITIAL,
            /** A request for generating the terrain mesh for this node has
             * been submitted, but the node has not yet received a response
             * with the generated mesh. */
            REQUESTED,
            /** This node holds a mesh that represents the terrain at this
             * node's level of detail. This mesh has not yet been "popped" onto
             * the scene. */
            DRAWABLE,
            /** Similar to drawable, except the mesh representing the terrain
             * for this mesh at this level of detail is empty, so no mesh is
             * held by the node. */
            EMPTY,
            /** The mesh that this node holds is currently part of the graphics
             * scene. This state implies that the node is being drawn at this
             * level of detail and no higher; there are not yet enough meshes
             * held by children to draw at a higher level of detail. */
            EMPTY_POPPED,
            /**
             *
             */
            POPPED,
            /** This state implies that all of the children of this mesh are
             * each in a drawable state, which can be any of DRAWABLE, EMPTY,
             * SPLIT, or VOID. This state also implies that none of the meshes
             * at or below this node have been popped onto the scene. */
            SPLIT,
            /** This state is the same as split except that some of the meshes
             * below this node are currently in the scene. This means that
             * terrain at this node is being drawn at a greater level of detail
             * than the level of detail of this node itself. */
            SPLIT_POPPED,
          };

          /**
           * Iterator for a LOD node that can iterate over the octree.
           */
          class Iterator {
            friend Node;
            private:
              const Node *m_current;

              /**
               * Constructs an iterator pointing to the given node. If current
               * is a null pointer, this signifies the parent of the root node
               * of the octree.
               *
               * \param current Pointer to the node that this iterator refers
               * to, which may be null.
               */
              Iterator(const Node *current) : m_current(current) {}
            public:
              /**
               * Overloaded pre-increment operator so that this iterator class
               * can be used as an iterator. This method moves the iterator to
               * the next node in the LOD octree. Node ordering is carried out
               * depth-first.
               */
              Iterator &operator++();

              /**
               * Overloaded dereference operator for accessing the node that
               * this iterator references.
               *
               * Note that it is possible that this iterator contains a null
               * pointer to a node if this iterator is past the octree. In
               * order to avoid dereferencing a null iterator, it is important
               * to check that this iterator does not equal the iterator
               * returned by Node::end().
               *
               * \return Reference to the LOD tree node that this iterator is
               * currently at. 
               */
              const Node &operator*() {
                return *m_current;
              }

              /**
               * Overloaded equality operator for comparing this iterator with
               * other iterators. Iterators are equal if they reference the
               * same LOD tree node.
               *
               * \param other The iterator with which to compare this iterator.
               * \return True if these iterators reference the same node, false
               * otherwise.
               */
              bool operator==(const Iterator &other) {
                return this->m_current == other.m_current;
              }

              /**
               * Overloaded inequality operator for comparing this iterator
               * with other iterators. This is implemented with the overloaded
               * equality operator.
               *
               * \param other The iterator with which to compare this iterator.
               * \return True if these iterators reference different nodes,
               * false otherwise.
               *
               * \sa operator==()
               */
              bool operator!=(const Iterator &other) {
                return !(*this == other);
              }
          };
        private:
          Node *m_parent;
          std::shared_ptr<Node> m_children[8];
          std::shared_ptr<TerrainMesh> m_mesh;
          State m_state;
          Coordinates m_block;
          int m_lod, m_index, m_drawableChildrenCount;

          int m_childIndexContainingBlock(const Coordinates &block);
          void m_childIndexBlock(int index, Coordinates *block);
          std::shared_ptr<Node> m_createChild(int index);
        public:
          /**
           * Consturcts a LOD node at the given position and level of detail.
           */
          Node(const Coordinates &block, int lod, Node *parent, int childIndex);

          /**
           * Returns the current state of the node state machine that
           * determines when to add and remove terrain meshes from the scene.
           *
           * \return The state of the node scene state machine as an enum
           * value.
           */
          State state() const { return m_state; }
          void setState(State state) { m_state = state; }

          /**
           * \return Shared pointer to the terrain mesh this node represents,
           * or nullptr if the mesh is empty or no such mesh has been
           * generated.
           */
          std::shared_ptr<TerrainMesh> mesh() { return m_mesh; }

          /**
           * Returns the voxel block at which this node resides, in integer
           * coordinates. The node's position is measured at corner of the node
           * with the lowest coordinate value.
           *
           * \return Integer coordinates of the voxel block where this node is
           * located.
           */
          const Coordinates &block() const { return m_block; }

          /**
           * Returns the level of detail at this node. Level of detail is
           * highest at level 0, and greater values of the level of detail
           * indicate a lower level of detail. The level of detail determines
           * the size of the node, with higher levels of detail being smaller
           * nodes further down in the octree.
           *
           * \return The level of detail at this node.
           */
          int lod() const { return m_lod; }

          /**
           * Returns in world coordinates the position of the block that this
           * node is located at. Block positions are given with respect to the
           * lowest value corner of the block. These positions are computed
           * from the block coordinates, the TerrainMesh::BLOCK_SIZE, and the
           * TerrainMesh::VOXEL_DELTA values.
           *
           * \return Position in world coordinates of the block that this node
           * is located at.
           */
          glm::vec3 pos();

          /**
           * Returns the size of a side of this node in terms of world space
           * unit length. The nodes are cube volumes, so each side has the same
           * length.
           *
           * Since nodes double in size as the level of detail goes down, this
           * size depends on the level of detail of the node. This size is
           * calculated from the node level of detail, the
           * TerrainMesh::BLOCK_SIZE, and the TerrainMesh::VOXEL_DELTA values.
           *
           * \return Size of one side of this node in terms of world space
           * units.
           */
          float size() const;

          /**
           * Returns an iterator starting at this LOD tree node for iterating
           * over this node and all of its children.
           *
           * If this node is a null pointer, which signifies an empty octree,
           * then this method returns an iterator referencing a null pointer as
           * well.
           *
           * \return An iterator refering to this node.
           */
          Iterator begin() const {
            return Iterator(this);
          }

          /**
           * Returns an iterator representing the end of the subtree defined by
           * this node.
           *
           * The iterator returned by this method actually points to the parent
           * of this node, but since this node might be the root of the octree
           * it is important not to iterate past the iterator returned by this
           * method.  Iterating past this iterator is undefined behavior.
           */
          Iterator end() const {
            return Iterator(this->parent());
          }

          /**
           * \return A pointer to the parent node of this node.
           */
          Node *parent() const { return m_parent; }

          /**
           * Finds the sibling of this node considered the "next" sibling.
           * Siblings are ordered based on their child index within the parent
           * node. If there is no next sibling (i.e. this node is the last
           * sibling), then a null pointer is returned.
           *
           * \return Shared pointer to the next sibling node, if such a sibling
           * exists.
           */
          std::shared_ptr<Node> nextSibling() const;

          /**
           * This method accesses the child of this node at the given index.
           * Since it is not required for the octree to be complete, it is
           * possible that this method will return a null pointer indicating no
           * child nodes at the given index.
           *
           * \param index Index of the child node to access.
           * \return Shared pointer to the child of this node at the given
           * index.
           */
          std::shared_ptr<Node> child(int index) const {
            return m_children[index];
          }

          /**
           * Determine if the node with the given block and level of detail
           * would be contained within this node.
           *
           * \param block The coordinates of the block to check.
           * \param lod The level of detail.
           * \return True if the node with the given block coordinates and
           * level of detail wolud be contained within this node.
           */
          bool contains(const Coordinates &block, int lod) const;

          /**
           * \return True if a child node exists at the given index, false
           * otherwise.
           */
          bool hasChild(int index) const {
            return m_children[index].get() != nullptr;
          }

          /**
           * Returns the child node at the given child index. If no child
           * exists at the given index yet, then a child is created at that
           * index before it is returned.
           *
           * \param index Index of the child node to return.
           * \return Shared pointer to the child at the given index.
           */
          std::shared_ptr<Node> getChild(int index);

          /**
           * Return the child node at the given block coordinates with the
           * given level of detail. If such a node does not already exist, then
           * it is created.
           *
           * \param block The coordinates of the block to look for.
           * \param lod The level of detail to look at.
           * \return Pointer to the node at the given block coordinates and
           * level of detail.
           */
          std::shared_ptr<Node> getDescendant(
              const Coordinates &block, int lod);

          /**
           * This method is used to check whether or not this node's voxel
           * block position is aligned to the lattice grid for this node's
           * level of detail. All nodes (other than the root node, which
           * straddles the origin) should be aligned in order for the LOD tree
           * to work correctly.
           *
           * \return True if this node is aligned to the lattice grid for this
           * node's level of detail, false otherwise.
           */
          bool isAligned() const;

          /**
           * Sets the terrain mesh represented by this node. A null pointer can
           * be passed to free the terrain mesh represented by this node. This
           * method also maintains the mesh counter on the parent node of this
           * node.
           *
           * A pointer to the terrain object is also passed to this method so
           * that terrain meshes can be added to the scene as children of the
           * terrain object as appropriate.
           *
           * \param mesh Shared pointer of the terrain mesh to set.
           * \param terrain The terrain object to which any new meshes should
           * be added as children as appropriate.
           */
          void setMesh(std::shared_ptr<TerrainMesh> mesh, Terrain *terrain);

        private:
          void m_childStateChanged(
              State oldChildState,
              int childIndex,
              Terrain *terrain);

          void m_popTerrain(Terrain *terrain);

          bool m_isValidState(Terrain *terrain);
      };
    private:
      std::shared_ptr<Node> m_root;

      void m_grow(const Coordinates &block, int lod);
      void m_alignBlockToLod(const Coordinates &block, int lod,
          Coordinates *alignedBlock);
    public:
      /**
       * Finds the coordinates of the voxel block that the point at the
       * given world space coordinates would reside in.
       *
       * \param pos The world position of the point in world space coordinates.
       * \param block Set to the coordinates of the voxel block that
       * contain the given point.
       */
      static void posToBlock(const glm::vec3 &pos, Coordinates *block);

      /**
       * Constructs a LOD tree which contains no terrain.
       */
      LodTree();

      /**
       * Returns the LOD node located at the given block for the given level of
       * detail. If that node does not exist, it and its parent nodes are
       * created when this method is called. If the given block coordinates do
       * not align to the given level of detail, then they are made to align to
       * the given level of detail; the returned node might not have the given
       * block coordinates.
       *
       * \param block The integer coordinates of the node to retrieve.
       * \param lod The level of detail at which the node resides, with the
       * default being the highest level of detail.
       */
      std::shared_ptr<Node> getNode(
          const Coordinates &block, int lod = 0);

      /**
       * Returns the node at the same level of detail as the given node at the
       * given offset. This offset is given in terms of nodes at this level of
       * detail, i.e. an offset of 2 would refer to a node 2 nodes over, not
       * two voxel blocks over. If the node at the offset does not exist, then
       * it and its parent nodes are created.
       *
       * \param node The originating node of which to find a relative node.
       * \param offset The offset given in terms of nodes at the same level of
       * detail adjacent to the given node node. 
       * \return Shared pointer to the node at the offset relative to this
       * node.
       */
      std::shared_ptr<Node> getRelativeNode(
          const Node &node, const Coordinates &offset);

      /**
       * Returns an iterator starting at the octree root for iterating over the
       * LOD tree. This method simply leverages the Node::begin() method on the
       * root node, which implements the desired behavior.
       *
       * \return Iterator at the begining of the LOD octree.
       *
       * \sa Node::begin()
       */
      Node::Iterator begin() const {
        assert(m_root);  // We always have a root node
        return m_root->begin();
      }
      /**
       * Returns an iterator signifying the end of the octree. This iterator
       * always contains a null pointer, since the root node does not have any
       * parent node. This method simply leverages the Node::end() method on
       * the root node, which implements the desired behavior.
       *
       * \return Iterator at the begining of the LOD octree.
       *
       * \sa Node::end()
       */
      Node::Iterator end() const {
        assert(m_root);  // We always have a root node
        return m_root->end();
      }
  };
} } }

#endif
