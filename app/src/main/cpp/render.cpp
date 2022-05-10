#include <initializer_list>
#include <cstdlib>

#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>

#include <cstring>
#include <vector>
#include <cmath>

#include "logger.h"
#include "render.h"
#include "shader.h"
#include "texture.h"
#include "boundary.h"


void opengles_init(AAssetManager* am) {
    // Check openGL on the system
    auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
    for (auto name : opengl_info) {
        auto info = glGetString(name);
        LOGI("OpenGL Info: %s", info);
    }


    boundary_init(am);


    glLineWidth(10.0f);
}

void opengles_deinit() {
    boundary_deinit();

}

void opengles_render_view(XrMatrix4x4f vp) {
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);

    // Clear swapchain and depth buffer.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    boundary_draw_grid(vp);

    boundary_draw_surface(vp);

}