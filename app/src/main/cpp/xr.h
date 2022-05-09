#ifndef _OPENGLESCAMERA_XR_H_
#define _OPENGLESCAMERA_XR_H_

#include <vector>

#include <openxr/openxr_reflection.h>
#include <openxr/openxr_platform.h>

#include "opengles.h"

struct Swapchain {
    XrSwapchain handle;
    int32_t width;
    int32_t height;
};

typedef struct XRContext_ {
    XrInstance instance{XR_NULL_HANDLE};
    XrSystemId systemId{XR_NULL_SYSTEM_ID};
    XrSession session{XR_NULL_HANDLE};
    XrSpace appSpace{XR_NULL_HANDLE};
    std::vector<XrViewConfigurationView> configViews;
    std::vector<XrView> views;
    std::vector<Swapchain> swapchains;
    XrSwapchainImageOpenGLESKHR swapchainImages[2][3];

    XrEventDataBuffer eventDataBuffer;

    OpenGLESContext gles;

    bool initialized = false;
    bool is_session_running = false;
} XRContext;

void xr_init(XRContext* xr, void* vm, void* activity, EGLDisplay display, EGLContext context, AAssetManager* am);
void xr_handle_events(XRContext* xr);
void xr_render_frame(XRContext* xr);
void xr_destroy(XRContext* xr);

#endif // _OPENGLESCAMERA_XR_H_