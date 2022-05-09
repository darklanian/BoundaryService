#ifndef _OPENGLESCAMERATEST_OPENGLES_H_
#define _OPENGLESCAMERATEST_OPENGLES_H_

#include <android/asset_manager_jni.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "common/gfxwrapper_opengl.h"
#include <common/xr_linear.h>

typedef struct {
    GLuint swapchainFramebuffer;
    GLuint vertexArrayID;
    GLuint programID;
    GLuint MatrixID;
    GLuint textureID;
    GLuint vertexBuffer;
    GLuint uvBuffer;
    GLuint texture;
    glm::mat4 MVP;
} OpenGLESContext;

void opengles_init(OpenGLESContext* gles, AAssetManager* am);
void opengles_deinit(OpenGLESContext* gles);
void opengles_render_view(OpenGLESContext* gles, XrMatrix4x4f vp);

#endif //_OPENGLESCAMERATEST_OPENGLES_H_