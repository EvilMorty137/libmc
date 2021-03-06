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

#include <cassert>
#include <glm/gtc/type_ptr.hpp>

extern "C" {
#include <mc/algorithms/common/cube.h>
}

#include <mcxx/vector.h>

#include "../common/glError.h"
#include "../common/shaderProgram.h"
#include "../common/shaders.h"

#include "cubeObject.h"

namespace mc {namespace samples { namespace cubes {
  CubeObject::CubeObject(
      unsigned int cube,
      unsigned int res_x, unsigned int res_y, unsigned int res_z,
      mcAlgorithmFlag algorithm,
      const glm::vec3 &position,
      const glm::quat &orientation)
    : MeshObject(position, orientation),
      m_isDrawScalarField(false),
      m_resX(res_x), m_resY(res_y), m_resZ(res_z), m_algorithm(algorithm),
      m_intensity(1.0f)
  {
    // Generate a simple cube wireframe and send it to the GL
    glGenBuffers(1, &m_cubeWireframeVertices);
    FORCE_ASSERT_GL_ERROR();
    glGenBuffers(1, &m_cubeWireframeIndices);
    FORCE_ASSERT_GL_ERROR();
    m_generateCubeWireframe();

    // Initialize the cube and send isosurface triangles, etc. to the GL
    glGenBuffers(1, &m_pointBuffer);
    FORCE_ASSERT_GL_ERROR();
    this->setDrawWinding(true);
    this->setCube(cube);
  }

  void CubeObject::m_generateCubeWireframe() {
    // Iterate over the cube vertices
    WireframeVertex vertices[8];
    unsigned int pos[3];
    for (int sampleIndex = 0; sampleIndex < 8; ++sampleIndex) {
      mcCube_sampleRelativePosition(sampleIndex, pos);
      vertices[sampleIndex].pos[0] = pos[0] ? 2.0f : 0.0f;
      vertices[sampleIndex].pos[1] = pos[1] ? 2.0f : 0.0f;
      vertices[sampleIndex].pos[2] = pos[2] ? 2.0f : 0.0f;
      vertices[sampleIndex].color[0] = 0.0f;
      vertices[sampleIndex].color[1] = 0.0f;
      vertices[sampleIndex].color[2] = 1.0f;
    }
    // Send the vertices to the GL
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeWireframeVertices);
    FORCE_ASSERT_GL_ERROR();
    glBufferData(
        GL_ARRAY_BUFFER,  /* target */
        sizeof(WireframeVertex) * 8,  /* size */
        vertices,  /* data */
        GL_STATIC_DRAW  /* usage */
        );
    FORCE_ASSERT_GL_ERROR();
    // Iterate over cube edges to make edge lines
    unsigned int indices[MC_CUBE_NUM_EDGES * 2];
    for (int edge = 0; edge < MC_CUBE_NUM_EDGES; ++edge) {
      unsigned int sampleIndices[2];
      mcCube_edgeSampleIndices(edge, sampleIndices);
      indices[edge * 2] = sampleIndices[0];
      indices[edge * 2 + 1] = sampleIndices[1];
    }
    // Send the indices to the GL
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeWireframeIndices);
    FORCE_ASSERT_GL_ERROR();
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,  /* target */
        sizeof(unsigned int) * MC_CUBE_NUM_EDGES * 2,  /* size */
        indices,  /* data */
        GL_STATIC_DRAW  /* usage */
        );
    FORCE_ASSERT_GL_ERROR();
  }

  void CubeObject::m_generateDebugPoints(const Mesh *mesh) {
    CubeScalarField sf(m_cube, m_intensity);

    // TODO: Generate points at cube vertices
    // TODO: Generate triangle points at edge intersections

    // Generate a grid of debugging points and send them to the GL
    m_numPoints = m_resX * m_resY * m_resZ;
    WireframeVertex *points = new WireframeVertex[m_numPoints];
    for (int z = 0; z < m_resZ; ++z) {
      for (int y = 0; y < m_resY; ++y) {
        for (int x = 0; x < m_resZ; ++x) {
          int i = x + y * m_resX + z * m_resX * m_resY;
          points[i].pos[0] = 2.0f * (float)x / (float)(m_resX - 1);
          points[i].pos[1] = 2.0f * (float)y / (float)(m_resY - 1);
          points[i].pos[2] = 2.0f * (float)z / (float)(m_resZ - 1);
          float value = sf(
              points[i].pos[0] - 1.0f,
              points[i].pos[1] - 1.0f,
              points[i].pos[2] - 1.0f);
          if (value >= 0.0f) {
            points[i].color[0] = 1.0f;
            points[i].color[1] = 0.0f;
            points[i].color[2] = 0.0f;
          } else {
            points[i].color[0] = 0.0f;
            points[i].color[1] = 1.0f;
            points[i].color[2] = 0.0f;
          }
        }
      }
    }

    // Send the points to the GL
    glBindBuffer(GL_ARRAY_BUFFER, m_pointBuffer);
    FORCE_ASSERT_GL_ERROR();
    glBufferData(
        GL_ARRAY_BUFFER,  // target
        sizeof(WireframeVertex) * m_numPoints,  // size
        points,  // data
        GL_STATIC_DRAW  // usage
        );
    FORCE_ASSERT_GL_ERROR();

    delete[] points;
  }

  void CubeObject::m_update() {
    CubeScalarField sf(m_cube, m_intensity);
    const Mesh *mesh = m_builder.buildIsosurface(
        sf,  // scalarField
        m_algorithm,  // algorithm
        m_resX, m_resY, m_resZ,  // res
        Vec3(-1.0f, -1.0f, -1.0f),  // min
        Vec3(1.0f, 1.0f, 1.0f)  // max
        );

    // Generate point data to send to the GL for visual debugging
    m_generateDebugPoints(mesh);

    // Generate verious other mesh data and send it to the GL
    this->setMesh(*mesh);
  }

  void CubeObject::m_drawCubeWireframe(
      const glm::mat4 &modelView,
      const glm::mat4 &projection) const
  {
    // Use our shader for drawing wireframes
    std::shared_ptr<ShaderProgram> shader = Shaders::wireframeShader();
    shader->use();

    // Prepare the uniform values
    assert(shader->modelViewLocation() != -1);
    glUniformMatrix4fv(
        shader->modelViewLocation(),  // location
        1,  // count
        0,  // transpose
        glm::value_ptr(modelView)  // value
        );
    ASSERT_GL_ERROR();
    assert(shader->projectionLocation() != -1);
    glUniformMatrix4fv(
        shader->projectionLocation(),  // location
        1,  // count
        0,  // transpose
        glm::value_ptr(projection)  // value
        );
    ASSERT_GL_ERROR();

    // Prepare the vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeWireframeVertices);
    ASSERT_GL_ERROR();
    assert(shader->vertPositionLocation() != -1);
    glEnableVertexAttribArray(shader->vertPositionLocation());
    ASSERT_GL_ERROR();
    glVertexAttribPointer(
        shader->vertPositionLocation(),  // index
        3,  // size
        GL_FLOAT,  // type
        0,  // normalized
        sizeof(WireframeVertex),  // stride
        &(((WireframeVertex *)0)->pos[0])  // pointer
        );
    ASSERT_GL_ERROR();
    assert(shader->vertColorLocation() != -1);
    glEnableVertexAttribArray(shader->vertColorLocation());
    ASSERT_GL_ERROR();
    glVertexAttribPointer(
        shader->vertColorLocation(),  // index
        3,  // size
        GL_FLOAT,  // type
        0,  // normalized
        sizeof(WireframeVertex),  // stride
        &(((WireframeVertex *)0)->color[0])  // pointer
        );
    ASSERT_GL_ERROR();

    // Draw the wireframe lines
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeWireframeIndices);
    ASSERT_GL_ERROR();
    glLineWidth(10.0f);
    ASSERT_GL_ERROR();
    glEnable(GL_LINE_SMOOTH);
    ASSERT_GL_ERROR();
    glDrawElements(
        GL_LINES,  // mode
        MC_CUBE_NUM_EDGES * 2,  // count
        GL_UNSIGNED_INT,  // type
        0  // indices
        );
    ASSERT_GL_ERROR();
  }

  void CubeObject::m_drawDebugPoints(
      const glm::mat4 &modelView,
      const glm::mat4 &projection) const
  {
    // Use our shader for drawing points
    std::shared_ptr<ShaderProgram> shader = Shaders::pointShader();
    shader->use();

    // Prepare the uniform values
    assert(shader->modelViewLocation() != -1);
    glUniformMatrix4fv(
        shader->modelViewLocation(),  // location
        1,  // count
        0,  // transpose
        glm::value_ptr(modelView)  // value
        );
    ASSERT_GL_ERROR();
    assert(shader->projectionLocation() != -1);
    glUniformMatrix4fv(
        shader->projectionLocation(),  // location
        1,  // count
        0,  // transpose
        glm::value_ptr(projection)  // value
        );
    ASSERT_GL_ERROR();

    // Prepare the vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, m_pointBuffer);
    ASSERT_GL_ERROR();
    assert(shader->vertPositionLocation() != -1);
    glEnableVertexAttribArray(shader->vertPositionLocation());
    ASSERT_GL_ERROR();
    glVertexAttribPointer(
        shader->vertPositionLocation(),  // index
        3,  // size
        GL_FLOAT,  // type
        0,  // normalized
        sizeof(WireframeVertex),  // stride
        &(((WireframeVertex *)0)->pos[0])  // pointer
        );
    ASSERT_GL_ERROR();
    assert(shader->vertColorLocation() != -1);
    glEnableVertexAttribArray(shader->vertColorLocation());
    ASSERT_GL_ERROR();
    glVertexAttribPointer(
        shader->vertColorLocation(),  // index
        3,  // size
        GL_FLOAT,  // type
        0,  // normalized
        sizeof(WireframeVertex),  // stride
        &(((WireframeVertex *)0)->color[0])  // pointer
        );
    ASSERT_GL_ERROR();

    // Draw the points for debugging
#ifndef __EMSCRIPTEN__
    glPointSize(1.0f);
    ASSERT_GL_ERROR();
#endif
    glDrawArrays(
        GL_POINTS,  // mode
        0,  // first
        m_numPoints  // count
        );
    ASSERT_GL_ERROR();
  }

  void CubeObject::draw(const glm::mat4 &modelWorld,
      const glm::mat4 &worldView, const glm::mat4 &projection,
      float alpha, bool debug)
  {
    // Compute the matrices we need for the shaders
    glm::mat4 modelView = worldView * modelWorld;
    glm::mat4 modelViewProjection = projection * modelView;

    // Draw the cube wireframe
    m_drawCubeWireframe(modelView, projection);

    if (m_isDrawScalarField) {
      // Draw the lattice points and edge intersection points
      m_drawDebugPoints(modelView, projection);
    }

    // Draw the mesh itself
    MeshObject::draw(modelWorld, worldView, projection, alpha, debug);
  }

  void CubeObject::setCube(unsigned int cube) {
    m_cube = cube;

    // (Re-)evaluate the isosurface extraction algorithm, since we have a new
    // isosurface
    m_update();
  }

  void CubeObject::setAlgorithm(mcAlgorithmFlag algorithm) {
    m_algorithm = algorithm;

    // Re-evaluate the isosurface, since we have selected a new algorithm
    m_update();
  }

  void CubeObject::setResolution(
      unsigned int x, unsigned int y, unsigned int z)
  {
    m_resX = x;
    m_resY = y;
    m_resZ = z;

    m_update();
  }

  void CubeObject::setIntensity(float intensity) {
    m_intensity = intensity;

    m_update();
  }

  CubeObject::CubeScalarField::CubeScalarField(
      unsigned int cube, float intensity)
    : m_cube(cube), m_intensity(intensity)
  {
  }

  float CubeObject::CubeScalarField::operator()(
      float x, float y, float z)
  {
    x = (x + 1.0f) / 2.0f;
    y = (y + 1.0f) / 2.0f;
    z = (z + 1.0f) / 2.0f;
    // A trilinear interpolation between the cube vertices
    float result = 0.0f;
    for (unsigned int z_index = 0; z_index <= 1; ++z_index) {
      for (unsigned int y_index = 0; y_index <= 1; ++y_index) {
        for (unsigned int x_index = 0; x_index <= 1; ++x_index) {
          unsigned int i = mcCube_sampleIndex(x_index, y_index, z_index);
          float value = mcCube_sampleValue(i, m_cube) ? -m_intensity : 1.0f;
          result +=
            (x_index ? x : 1.0f - x) *
            (y_index ? y : 1.0f - y) *
            (z_index ? z : 1.0f - z) *
            value;
        }
      }
    }
    return result;
  }
} } }
