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

#ifndef MC_ALGORITHMS_NIELSON_DUAL_NIELSON_DUAL_TABLES_H_
#define MC_ALGORITHMS_NIELSON_DUAL_NIELSON_DUAL_TABLES_H_

#include "common.h"

/**
 * This table facilitates fast lookup of vertices that are generated by the
 * MC-Dual algorithm as described by Nielson. This table includes both edge
 * intersections associated with each vertex and vertex connectivity for each
 * vertex.
 */
extern const mcNielsonDualVertexList mcNielsonDual_vertexTable[];

/**
 * The midpoint vertex table is derived from mcNielsonDual_vertexTable. The
 * cooked table includes precalculated vertex positions where all edge
 * intersections are taken at the midpoint of the edge. This is the strategy
 * used by Nielson in his "Dual Marching Cubes" paper. More sophisticated
 * strategies can be used, but the midpoint strategy produces good results.
 */
extern const mcNielsonDualCookedVertexList mcNielsonDual_midpointVertexTable[];

/**
 * The quad vertex lookup table facilitates quick lookup of the vertex index
 * associated with a given edge for a given cube configuration. In MC-Dual,
 * there can be up to 4 vertices per voxel, so this vertex index ranges from 0
 * to 3. This lookup is needed in order to quickly access the vertices of
 * adjacent voxels when connecting vertices of a quad.
 *
 * This table is indexed by edge in the high order byte and cube configuration
 * in the low order byte for a total of 16 * 256 = 4096 entries. Valid vertex
 * indices range from 0 to 3, and vertices that do not exist have index -1.
 */
extern const int mcNielsonDual_vertexIndexLookupTable[];

#endif
