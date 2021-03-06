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

#include <cstdlib>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../common/demo.h"
#include "../common/orthographicCamera.h"
#include "../common/scene.h"
#include "coloredSquareObject.h"

using namespace mc::samples;
using namespace mc::samples::coloredSquares;

class ColoredSquares : public Demo {
  private:
    std::shared_ptr<OrthographicCamera> m_camera;
    std::shared_ptr<ColoredSquareObject> m_coloredSquareObject;
  public:
    ColoredSquares(int argc, char **argv)
      : Demo(argc, argv, "Colored Marching Squares Demo")
    {
      if (this->argError())
        return;

      // Populate the graphics scene
      m_camera = std::shared_ptr<OrthographicCamera>(
          new OrthographicCamera(
            -0.5f,  // left
            1.5f,  // right
            -0.5f,  // bottom
            1.5f,  // top
            0.1f,  // near
            1000.0f,  // far
            glm::vec3(0.0f, 0.0f, 15.0f)  // position
            ));
      this->scene()->addObject(m_camera);
      this->setCamera(m_camera);
      m_coloredSquareObject = std::shared_ptr<ColoredSquareObject>(
          new ColoredSquareObject());
      this->scene()->addObject(m_coloredSquareObject);
    }
};

ColoredSquares *demo;

void main_loop() {
  demo->mainLoop();
}

int main(int argc, char **argv) {
  demo = new ColoredSquares(argc, argv);
  if (demo->argError()) {
    delete demo;
    return EXIT_FAILURE;
  }

  if (demo->isScreenshot()) {
    demo->drawScreenshot();
  } else {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (1) {
      main_loop();
      // TODO: Wait for VSync? Or should we poll input faster than that?
    }
#endif
  }

  // FIXME: This can never be reached if the demo object calls exit()
  delete demo;

  return EXIT_SUCCESS;
}
