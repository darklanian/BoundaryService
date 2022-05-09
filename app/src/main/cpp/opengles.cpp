#include <initializer_list>
#include <cstdlib>

#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>

#include <cstring>
#include <vector>

#include "logger.h"
#include "opengles.h"
#include "shader.h"

void checkGlError(const char* name) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        LOGE("%s error: 0x%08X", name, error);
}

std::vector<float> boundaryPoints;

void add_point(float x, float y, float z) {
    boundaryPoints.push_back(x);
    boundaryPoints.push_back(y);
    boundaryPoints.push_back(z);
}

void opengles_init(OpenGLESContext* gles, AAssetManager* am) {
    // Check openGL on the system
    auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
    for (auto name : opengl_info) {
        auto info = glGetString(name);
        LOGI("OpenGL Info: %s", info);
    }

    glGenFramebuffers(1, &gles->swapchainFramebuffer);

    gles->programID = create_program(am, "vertex.shader", "fragment.shader");

    gles->MatrixID = glGetUniformLocation(gles->programID, "MVP");
    gles->MVP = glm::mat4(1.0f);

    /*add_point(-0.5f, -0.5f, -5.0f);
    add_point(0.5f, -0.5f, -5.0f);
    add_point(0.0f, 0.5f, -5.0f);*/
    float y = -5.0f;
    float x, z;
    for (int i = 0; i < 10; i++) {
        x = i - 5.0f;
        z = -5.0f;
        add_point(x, y, z);
    }
    for (int i = 0; i < 10; i++) {
        z = i - 5.0f;
        x = 5.0f;
        add_point(x, y, z);
    }
    for (int i = 9; i >= 0; i--) {
        x = i - 5.0f;
        z = 5.0f;
        add_point(x, y, z);
    }
    for (int i = 9; i >= 0; i--) {
        z = i - 5.0f;
        x = -5.0f;
        add_point(x, y, z);
    }
}

void opengles_deinit(OpenGLESContext* gles) {
    glDeleteProgram(gles->programID);
}

void opengles_render_view(OpenGLESContext* gles, XrMatrix4x4f vp) {
    glUseProgram(gles->programID);
    //glUniformMatrix4fv(gles->MatrixID, 1, GL_FALSE, &gles->MVP[0][0]);

    XrVector3f translation {0.0f, 0.0f, 0.0f};
    XrQuaternionf rotation {0.0f, 0.0f, 0.0f, 1.0f};
    XrVector3f scale {1.0f, 1.0f, 1.0f};
    XrMatrix4x4f model;
    XrMatrix4x4f_CreateTranslationRotationScale(&model, &translation, &rotation, &scale);
    XrMatrix4x4f mvp;
    XrMatrix4x4f_Multiply(&mvp, &vp, &model);
    glUniformMatrix4fv(gles->MatrixID, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&mvp));


    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, boundaryPoints.size()*4*3, boundaryPoints.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glLineWidth(10.0f);
    glDrawArrays(GL_LINE_LOOP, 0, boundaryPoints.size()/3);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &buffer);
}