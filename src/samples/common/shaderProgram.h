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

#ifndef MC_SAMPLES_COMMON_SHADER_PROGRAM_H_
#define MC_SAMPLES_COMMON_SHADER_PROGRAM_H_

#include <GL/glew.h>
#include <string>

namespace mc { namespace samples {
  /**
   * Class representing a compiled and linked GL shader program, suitable for
   * rendering GL graphics.
   */
  class ShaderProgram {
    private:
      // FIXME: Clean these up with a couple macros
      // FIXME: Remove "Location" from all of these variables
      GLuint m_shaderProgram;
      GLuint m_modelViewLocation, m_projectionLocation,
             m_modelViewProjectionLocation, m_normalTransformLocation,
             m_lightPositionLocation, m_lightIntensityLocation,
             m_timeLocation;
      GLuint m_vertPositionLocation, m_vertNormalLocation, m_vertColorLocation,
             m_vertTexCoordLocation, m_vertVelocityLocation;
      GLuint m_vertStartTimeLocation;
      GLuint m_texture0;

      static GLuint m_compileShader(const std::string &file, GLenum type);
      static void m_linkShaderProgram(GLuint shaderProgram);

    protected:
      /**
       * A virtual member function to allow derived classes the opportunity to
       * register attribute and/or uniform locations immediately after the shader
       * program has been linked.
       */
      // FIXME: This virtual call cannot be made in the constructor... I should
      //        just remove this.
      virtual void initLocations();

    public:
      ShaderProgram(const std::string &vert, const std::string &frag);
      virtual ~ShaderProgram();

      GLuint modelViewLocation() const { return m_modelViewLocation; }
      GLuint projectionLocation() const { return m_projectionLocation; }
      GLuint modelViewProjectionLocation() const { return m_modelViewProjectionLocation; }
      GLuint normalTransformLocation() const { return m_normalTransformLocation; }
      GLuint lightPositionLocation() const { return m_lightPositionLocation; }
      GLuint lightIntensityLocation() const { return m_lightIntensityLocation; }
      GLuint timeLocation() const { return m_timeLocation; }
      GLuint vertPositionLocation() const { return m_vertPositionLocation; }
      GLuint vertNormalLocation() const { return m_vertNormalLocation; }
      GLuint vertColorLocation() const { return m_vertColorLocation; }
      GLuint vertTexCoordLocation() const { return m_vertTexCoordLocation; }
      GLuint vertVelocityLocation() const { return m_vertVelocityLocation; }
      GLuint vertStartTimeLocation() const { return m_vertStartTimeLocation; }
      GLuint texture0() const { return m_texture0; }

      void use() const;
  };
} }

#endif
