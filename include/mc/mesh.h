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

#ifndef MC_MESH_H_
#define MC_MESH_H_

#include <mc/vertex.h>

/**
 * \addtogroup libmc
 * @{
 */

/**
 * \defgroup mcMesh mcMesh
 */

/**
 * \addtogroup mcMesh
 * @{
 */

/** \file mc/mesh.h
 *
 * This file contains definitions for the mcMesh structure and the structures
 * that it contains.
 */

#include "vector.h"

/**
 * Face structure for meshes generated by libmc. Each index in the face
 * corresponds with a vertex index returend by mcMesh_addVertex().
 *
 * Mesh faces are often triangles, but for some isosurface extraction
 * algorithms they can have more than three vertex indices. For instance, the
 * MC-Patch isosurface extraction algorithm generates faces with up to six
 * vertex indices, and the vertices themselves are not necessarily co-planar.
 *
 * Since the number of faces cannot be known at compile time, it becomes
 * necessary to initialize face memory with the mcFace_init() routine.
 */
typedef struct mcFace {
  unsigned int *indices;
  unsigned int numIndices;
} mcFace;

/**
 * Initializes a mcFace structure.
 *
 * \param self The mcFace structure to initialize.
 * \param numIndices The number of vertex indices that this mcFace structure is
 * to store.
 *
 * Since libmc supports any number of edges for a face, it becomes necessary to
 * dynamically allocate the appropriate number of face indices for any given
 * face structure. Because dynamically allocating memory for many such small
 * structures would incur a high overhead cost, the library maintains an
 * internal pool of memory for common face sizes such as triangles and quads.
 */
void mcFace_init(
    mcFace *self,
    unsigned int numIndices);

/**
 * Copies a mcFace structure into another mcFace structure.
 *
 * \param self The mcFace structure being initalized with the copied data.
 * \param other The mcFace structure whose data is to be copied.
 *
 * This routine is analogous to a copy constructor in C++; the face structure
 * pointed to by \p self should be uninitialized before this routine is called.
 * After this routine is called, \p self should be treated as an initialized
 * mcFace structure, and should eventually be destroyed with a call to
 * mcFace_destroy().
 */
void mcFace_copy(
    mcFace *self,
    const mcFace *other);

/**
 * Moves a mcFace structure into another mcFace structure. This is analogous to
 * a C++ move constructor.
 *
 * \param self The mcFace structure being moved to, analogous to the \p this
 * parameter in a C++ move constructor.
 * \param other The mcFace structure being moved.
 *
 * \todo I don't think this routine is being used at this time, but it could be used
 * in the future if the mcFace structure was being backed by a memory pool.
 */
void mcFace_move(
    mcFace *self,
    mcFace *other);

/**
 * Frees the memory allocated for an mcFace structure.
 *
 * \param self The mcFace structure to destroy.
 *
 * This routine is important to call with each face after use, since all mcFace
 * structures must allocate some memory.
 */
void mcFace_destroy(
    mcFace *self);

/**
 * The structure for representing all meshes generated by libmc. This is an
 * important structure, since any user of the library will need to nagigate
 * this structure in order to obtain mesh data.
 *
 * The mcMesh structure is not intended to be used as-is; it should be copied
 * into whatever mesh structure is appropriate for the given application.
 *
 * \todo Some add vertex/face callbacks could supersede the mcMesh structure.
 * There is also the issue of meshes generated within OpenCL or similar.
 */
typedef struct mcMesh {
  mcVertex *vertices;
  mcFace *faces;
  unsigned int numVertices, numFaces, numIndices;
  unsigned int sizeVertices, sizeFaces;
  int isTriangleMesh;
} mcMesh;

/**
 * Initializes the mcMesh structure. This allocates memory for the vertex and
 * face structures that grow as more vertices and faces are added.
 *
 * \param self The mcMesh structure to be initialized.
 */
void mcMesh_init(
    mcMesh *self);

/**
 * Free memory allocated by the mcMesh structure. This also frees all vertices
 * and faces held by the mesh structure.
 *
 * \param self The mcMesh structure to destroy.
 */
void mcMesh_destroy(
    mcMesh *self);

/**
 * \internal
 * Doubles the number of vertices that can be stored by this mesh.
 * \endinternal
 *
 * \param self The mcMesh structure whose vetex storage we are doubling.
 */
void mcMesh_growVertices(
    mcMesh *self);

/**
 * \internal
 * Doubles the number of faces that can be stored by this mesh.
 * \endinternal
 *
 * \param self The mcMesh structure whose face storage we are growing.
 */
void mcMesh_growFaces(
    mcMesh *self);

/**
 * Adds a vertex to the mcMesh structure. The unsigned integer returned is the
 * vertex index of the vertex just added. That index must be used with faces
 * subsequently added with mcMesh_addFace().
 *
 * \param self The mcMesh structure to which we are adding a vertex.
 * \param vertex The vertex to be added.
 * \return Vertex index of the vertex just added.
 *
 * \sa mcMesh_addFace()
 */
unsigned int mcMesh_addVertex(
    mcMesh *self,
    const mcVertex *vertex);

/**
 * Adds a face to the mcMesh structure. Any face added to the mcMesh structure
 * must use vertex indices as returned by mcMesh_addVertex().
 *
 * \param self The mcMesh structure to which we are adding a face.
 * \param face The face to be added.
 *
 * The mesh structure does not take ownership of \p face; instead, it makes a
 * copy of the vertex indices.
 *
 * \todo This copy behavior might change in a future version of libmc if it is
 * discovered that this is a performance bottleneck.
 */
void mcMesh_addFace(
    mcMesh *self,
    const mcFace *face);

/** @} */

/** @} */

#endif
