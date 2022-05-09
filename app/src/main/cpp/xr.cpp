#include <android_native_app_glue.h>

#include <EGL/egl.h>
#include <GLES3/gl32.h>

#include <cstring>

#include "xr.h"

#include "logger.h"

#include "common/gfxwrapper_opengl.h"
#include <common/xr_linear.h>

namespace Math {
    namespace Pose {
        XrPosef Identity() {
            XrPosef t{};
            t.orientation.w = 1;
            return t;
        }
    }  // namespace Pose
}  // namespace Math

static void xr_initialize_loader(void* vm, void *activity) {
    PFN_xrInitializeLoaderKHR initializeLoader = nullptr;
    if (XR_SUCCEEDED(
            xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)(&initializeLoader)))) {
        XrLoaderInitInfoAndroidKHR loaderInitInfoAndroid;
        memset(&loaderInitInfoAndroid, 0, sizeof(loaderInitInfoAndroid));
        loaderInitInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
        loaderInitInfoAndroid.next = NULL;
        loaderInitInfoAndroid.applicationVM = vm;
        loaderInitInfoAndroid.applicationContext = activity;
        initializeLoader((const XrLoaderInitInfoBaseHeaderKHR*)&loaderInitInfoAndroid);
    }
}

static void xr_initialize_system(XRContext* xr) {
    XrSystemGetInfo systemInfo{XR_TYPE_SYSTEM_GET_INFO};
    systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    systemInfo.next = nullptr;
    xrGetSystem(xr->instance, &systemInfo, &xr->systemId);

    // Extension function must be loaded by name
    PFN_xrGetOpenGLESGraphicsRequirementsKHR pfnGetOpenGLESGraphicsRequirementsKHR = nullptr;
    xrGetInstanceProcAddr(xr->instance, "xrGetOpenGLESGraphicsRequirementsKHR",
                          reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetOpenGLESGraphicsRequirementsKHR));

    XrGraphicsRequirementsOpenGLESKHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR};
    pfnGetOpenGLESGraphicsRequirementsKHR(xr->instance, xr->systemId, &graphicsRequirements);
}

static void xr_create_instance(XRContext* xr, void* vm, void *activity) {
    xr_initialize_loader(vm, activity);

    std::vector<const char*> extensions;
    extensions.push_back(XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME);
    extensions.push_back(XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME);

    XrInstanceCreateInfoAndroidKHR instanceCreateInfoAndroid = {XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR};
    instanceCreateInfoAndroid.applicationVM = vm;
    instanceCreateInfoAndroid.applicationActivity = activity;
    instanceCreateInfoAndroid.next = nullptr;

    XrInstanceCreateInfo createInfo{XR_TYPE_INSTANCE_CREATE_INFO};
    createInfo.next = &instanceCreateInfoAndroid;
    createInfo.enabledExtensionCount = (uint32_t)extensions.size();
    createInfo.enabledExtensionNames = extensions.data();

    strcpy(createInfo.applicationInfo.applicationName, "Boundary");
    createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

    if (XR_SUCCEEDED(xrCreateInstance(&createInfo, &xr->instance))) {
        LOGI("xrCreateInstance succeeded");
    } else {
        LOGE("xrCreateInstance failed");
    }
    xr_initialize_system(xr);
}

static void xr_create_session(XRContext* xr, EGLDisplay display, EGLContext context) {
    XrGraphicsBindingOpenGLESAndroidKHR graphicsBinding{XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR};
    graphicsBinding.display = display;
    graphicsBinding.config = (EGLConfig)0;
    graphicsBinding.context = context;
    graphicsBinding.next = nullptr;

    XrSessionCreateInfo createInfo{XR_TYPE_SESSION_CREATE_INFO};
    createInfo.next = &graphicsBinding;
    createInfo.systemId = xr->systemId;
    if (XR_SUCCEEDED(xrCreateSession(xr->instance, &createInfo, &xr->session))) {
        LOGI("xrCreateSession succeeded");
    } else {
        LOGE("xrCreateSession failed");
    }

    XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
    referenceSpaceCreateInfo.poseInReferenceSpace = Math::Pose::Identity();
    referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    referenceSpaceCreateInfo.next = nullptr;
    xrCreateReferenceSpace(xr->session, &referenceSpaceCreateInfo, &xr->appSpace);
}

static void xr_create_swapchain(XRContext* xr) {
    // Query and cache view configuration views.
    uint32_t viewCount;
    xrEnumerateViewConfigurationViews(xr->instance, xr->systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr);
    xr->configViews.resize(viewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
    xrEnumerateViewConfigurationViews(xr->instance, xr->systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount, xr->configViews.data());

    // Create and cache view buffer for xrLocateViews later.
    xr->views.resize(viewCount, {XR_TYPE_VIEW});

    for (uint32_t i = 0; i < viewCount; i++) {
        const XrViewConfigurationView& vp = xr->configViews[i];

        // Create the swapchain.
        XrSwapchainCreateInfo swapchainCreateInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        swapchainCreateInfo.arraySize = 1;
        swapchainCreateInfo.format = GL_RGBA8;
        swapchainCreateInfo.width = vp.recommendedImageRectWidth;
        swapchainCreateInfo.height = vp.recommendedImageRectHeight;
        swapchainCreateInfo.mipCount = 1;
        swapchainCreateInfo.faceCount = 1;
        swapchainCreateInfo.sampleCount = 1;
        swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        Swapchain swapchain;
        swapchain.width = swapchainCreateInfo.width;
        swapchain.height = swapchainCreateInfo.height;
        xrCreateSwapchain(xr->session, &swapchainCreateInfo, &swapchain.handle);

        xr->swapchains.push_back(swapchain);

        uint32_t imageCount = 0;
        xrEnumerateSwapchainImages(swapchain.handle, 0, &imageCount, nullptr);
        for (int j = 0; j < imageCount; j++)
            xr->swapchainImages[i][j].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR;
        xrEnumerateSwapchainImages(swapchain.handle, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)xr->swapchainImages[i]);
    }
}

void xr_init(XRContext* xr, void* vm, void* activity, EGLDisplay display, EGLContext context, AAssetManager* am) {
    if (xr->initialized)
        return;
    xr_create_instance(xr, vm, activity);
    opengles_init(&xr->gles, am);
    xr_create_session(xr, display, context);
    xr_create_swapchain(xr);
    xr->initialized = true;
}

void xr_destroy(XRContext* xr) {
    if (!xr->initialized)
        return;

    for (Swapchain swapchain : xr->swapchains) {
        xrDestroySwapchain(swapchain.handle);
    }

    if (xr->appSpace != XR_NULL_HANDLE) xrDestroySpace(xr->appSpace);
    if (xr->session != XR_NULL_HANDLE) xrDestroySession(xr->session);
    if (xr->instance != XR_NULL_HANDLE) xrDestroyInstance(xr->instance);

    opengles_deinit(&xr->gles);
}

static void xr_render_view(XRContext* xr, const XrCompositionLayerProjectionView& layerView, XrSwapchainImageOpenGLESKHR* swapchainImage){
    glBindFramebuffer(GL_FRAMEBUFFER, xr->gles.swapchainFramebuffer);

    const uint32_t colorTexture = swapchainImage->image;

    glViewport(static_cast<GLint>(layerView.subImage.imageRect.offset.x),
               static_cast<GLint>(layerView.subImage.imageRect.offset.y),
               static_cast<GLsizei>(layerView.subImage.imageRect.extent.width),
               static_cast<GLsizei>(layerView.subImage.imageRect.extent.height));

    //glFrontFace(GL_CW);
    //glCullFace(GL_BACK);
    //glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    //const uint32_t depthTexture = GetDepthTexture(colorTexture);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    // Clear swapchain and depth buffer.
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    const auto& pose = layerView.pose;
    XrMatrix4x4f proj;
    XrMatrix4x4f_CreateProjectionFov(&proj, GRAPHICS_OPENGL_ES, layerView.fov, 0.05f, 100.0f);
    XrMatrix4x4f toView;
    XrVector3f scale{1.f, 1.f, 1.f};
    XrMatrix4x4f_CreateTranslationRotationScale(&toView, &pose.position, &pose.orientation, &scale);
    XrMatrix4x4f view;
    XrMatrix4x4f_InvertRigidBody(&view, &toView);
    XrMatrix4x4f vp;
    XrMatrix4x4f_Multiply(&vp, &proj, &view);

    opengles_render_view(&xr->gles, vp);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static bool xr_render_layer(XRContext* xr, XrTime predictedDisplayTime, std::vector<XrCompositionLayerProjectionView>& projectionLayerViews,
                     XrCompositionLayerProjection& layer) {
    XrResult res;

    XrViewState viewState{XR_TYPE_VIEW_STATE};
    uint32_t viewCapacityInput = (uint32_t)xr->views.size();
    uint32_t viewCountOutput;

    XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
    viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    viewLocateInfo.displayTime = predictedDisplayTime;
    viewLocateInfo.space = xr->appSpace;

    res = xrLocateViews(xr->session, &viewLocateInfo, &viewState, viewCapacityInput, &viewCountOutput, xr->views.data());
    if ((viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) == 0 ||
        (viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) == 0) {
        return false;  // There is no valid tracking poses for the views.
    }

    projectionLayerViews.resize(viewCountOutput);

    // Render view to the appropriate part of the swapchain image.
    for (uint32_t i = 0; i < viewCountOutput; i++) {
        // Each view has a separate swapchain which is acquired, rendered to, and released.
        const Swapchain viewSwapchain = xr->swapchains[i];

        XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};

        uint32_t swapchainImageIndex;
        xrAcquireSwapchainImage(viewSwapchain.handle, &acquireInfo, &swapchainImageIndex);

        XrSwapchainImageWaitInfo waitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
        waitInfo.timeout = XR_INFINITE_DURATION;
        xrWaitSwapchainImage(viewSwapchain.handle, &waitInfo);

        projectionLayerViews[i] = {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
        projectionLayerViews[i].pose = xr->views[i].pose;
        projectionLayerViews[i].fov = xr->views[i].fov;
        projectionLayerViews[i].subImage.swapchain = viewSwapchain.handle;
        projectionLayerViews[i].subImage.imageRect.offset = {0, 0};
        projectionLayerViews[i].subImage.imageRect.extent = {viewSwapchain.width, viewSwapchain.height};

        xr_render_view(xr, projectionLayerViews[i], &xr->swapchainImages[i][swapchainImageIndex]);

        XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        xrReleaseSwapchainImage(viewSwapchain.handle, &releaseInfo);
    }
    layer.space = xr->appSpace;
    layer.viewCount = (uint32_t)projectionLayerViews.size();
    layer.views = projectionLayerViews.data();
    return true;
}

void xr_render_frame(XRContext* xr) {
    XrFrameWaitInfo frameWaitInfo{XR_TYPE_FRAME_WAIT_INFO};
    XrFrameState frameState{XR_TYPE_FRAME_STATE};
    xrWaitFrame(xr->session, &frameWaitInfo, &frameState);

    XrFrameBeginInfo frameBeginInfo{XR_TYPE_FRAME_BEGIN_INFO};
    xrBeginFrame(xr->session, &frameBeginInfo);

    std::vector<XrCompositionLayerBaseHeader*> layers;
    XrCompositionLayerProjection layer{XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    std::vector<XrCompositionLayerProjectionView> projectionLayerViews;
    if (frameState.shouldRender == XR_TRUE) {
        if (xr_render_layer(xr, frameState.predictedDisplayTime, projectionLayerViews, layer)) {
            layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&layer));
        }
    }

    XrFrameEndInfo frameEndInfo{XR_TYPE_FRAME_END_INFO};
    frameEndInfo.displayTime = frameState.predictedDisplayTime;
    frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    frameEndInfo.layerCount = (uint32_t)layers.size();
    frameEndInfo.layers = layers.data();
    xrEndFrame(xr->session, &frameEndInfo);

    //LOGI("xrEndFrame");
}

void xr_handle_events(XRContext* xr) {
    if (!xr->initialized)
        return;

    memset(&xr->eventDataBuffer, 0, sizeof(XrEventDataBuffer));
    XrEventDataBaseHeader* event = reinterpret_cast<XrEventDataBaseHeader*>(&xr->eventDataBuffer);
    *event = {XR_TYPE_EVENT_DATA_BUFFER, };
    while (xrPollEvent(xr->instance, &xr->eventDataBuffer) == XR_SUCCESS) {
        if (xr->eventDataBuffer.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
            auto sessionStateChangedEvent = *reinterpret_cast<const XrEventDataSessionStateChanged*>(event);
            switch (sessionStateChangedEvent.state) {
                case XR_SESSION_STATE_READY:
                if (!xr->is_session_running){
                    XrSessionBeginInfo sessionBeginInfo {
                        .type = XR_TYPE_SESSION_BEGIN_INFO,
                        .next = nullptr,
                        .primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO};
                    if (XR_SUCCEEDED(xrBeginSession(xr->session, &sessionBeginInfo))) {
                        xr->is_session_running = true;
                        LOGI("begin session");
                    }
                }
                    break;
                case XR_SESSION_STATE_STOPPING:
                    xr->is_session_running = false;
                    xrEndSession(xr->session);
                    LOGI("end session");
                    break;
                default:
                    break;
            }
        }
    }
}
