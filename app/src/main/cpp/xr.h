#ifndef _OPENGLESCAMERA_XR_H_
#define _OPENGLESCAMERA_XR_H_

#include <vector>

#include <openxr/openxr_reflection.h>
#include <openxr/openxr_platform.h>

#include "render.h"

struct Swapchain {
    XrSwapchain handle;
    int32_t width;
    int32_t height;
};

void xr_init(void* vm, void* activity, AAssetManager* am);
void xr_handle_events();
void xr_render_frame();
void xr_destroy();
bool xr_is_session_running();

#endif // _OPENGLESCAMERA_XR_H_