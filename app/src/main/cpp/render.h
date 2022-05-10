#ifndef _OPENGLESCAMERATEST_OPENGLES_H_
#define _OPENGLESCAMERATEST_OPENGLES_H_

#include <android/asset_manager_jni.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "common/gfxwrapper_opengl.h"
#include <common/xr_linear.h>
#include <vector>

void opengles_init(AAssetManager* am);
void opengles_deinit();
void opengles_render_view(XrMatrix4x4f vp);

#endif //_OPENGLESCAMERATEST_OPENGLES_H_