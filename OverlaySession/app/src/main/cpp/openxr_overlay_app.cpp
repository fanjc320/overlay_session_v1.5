/****************************************************************
 * Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ****************************************************************/

#include "openxr_overlay_app.h"
#include <thread>

void OpenXRApp::initial(JavaVM *vm, void *context) {
  LOGI("initial ");
  engine.serviceContext = context;
  engine.vm = vm;
  mOverlayHided.store(true);
  mShutdown.store(false);
  mRequestExitSession.store(false);
  mBatteryLevel.store(-1);
}

void OpenXRApp::shutdown() {
  LOGI("shutdown ");
  if (!mOverlayHided) {
    endOverlay();
  }
  mShutdown.store(true);
}

void OpenXRApp::startOverlay() {
  LOGI("startOverlay ");
  // create session and begin session when session is ready in loop
  mOverlayHided.store(false);
}

void OpenXRApp::endOverlay() {
  // request exit session and destroy session in loop
  LOGI("endOverlay ");
  mOverlayHided.store(true);
  mRequestExitSession.store(true);
}

void OpenXRApp::update() {
  LOGI("overlay session update begin ");
  if (mOverlayHided.load() && mSessionStopped) {
     LOGI("overlay session sleep ");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return;
  }
   LOGI("overlay session update ... ");

  if (mRequestExitSession.load()) {
     LOGI("overlay mRequestExitSession");
    if (XR_SUCCEEDED(xrRequestExitSession(engine.state.xrSession))) {
      LOGI("overlay mRequestExitSession succeed");
      mRequestExitSession.store(false);
    }
  }
  if (!mOverlayStarted) {
    startOverlayXR();
  }

  // poll event, begin session, destroy session, and rendering
  AppCommon::app_poll_events(&engine);

  if (engine.sessionState == XR_SESSION_STATE_EXITING) {
    stopOverlayXR();
    LOGI("overlay session stopOverlayXR ... ");
    return;
  }
  if (engine.ready) {
      LOGI("overlay session rendering ... ");
    rendering();
  }
}

void OpenXRApp::initOpenXR() {

  engine.depthInfo.supported = false;
  engine.depthInfo.infos = nullptr;
  LOGI("initOpenXR");

  // initial loader
  PFN_xrInitializeLoaderKHR initializeLoader = nullptr;
  if (XR_SUCCEEDED(
          xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR",
                                (PFN_xrVoidFunction *)(&initializeLoader)))) {
    XrLoaderInitInfoAndroidKHR loaderInitInfoAndroid;
    memset(&loaderInitInfoAndroid, 0, sizeof(loaderInitInfoAndroid));
    loaderInitInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
    loaderInitInfoAndroid.next = NULL;
    loaderInitInfoAndroid.applicationVM = (void *)engine.vm;
    loaderInitInfoAndroid.applicationContext = (void *)engine.serviceContext;
    initializeLoader(
        (const XrLoaderInitInfoBaseHeaderKHR *)&loaderInitInfoAndroid);
  }

  AppCommon::app_query_layers_and_extensions(&engine, engine.vm,
                                             engine.serviceContext);

  XrInstanceCreateInfoAndroidKHR instanceCreateInfoAndroidKHR;
  instanceCreateInfoAndroidKHR.type = XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR;
  instanceCreateInfoAndroidKHR.next = nullptr;
  instanceCreateInfoAndroidKHR.applicationVM = (void *)engine.vm;
  instanceCreateInfoAndroidKHR.applicationActivity =
      (void *)engine.serviceContext;

  // TODO: should not hard-code these ideally
  const char *const enabledExtensions[] = {
      XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME,
      XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME, XR_EXTX_OVERLAY_EXTENSION_NAME};

  XrInstanceCreateInfo instanceCreateInfo = {
      .type = XR_TYPE_INSTANCE_CREATE_INFO,
      .next = &instanceCreateInfoAndroidKHR,
      .createFlags = 0,
      .enabledExtensionCount =
          sizeof(enabledExtensions) / sizeof(*enabledExtensions),
      .enabledExtensionNames = enabledExtensions,
      .enabledApiLayerNames = nullptr,
      .enabledApiLayerCount = 0,
      .applicationInfo =
          {
              .applicationName = "OpenXR overlay session Sample",
              .engineName = "",
              .applicationVersion = 1,
              .engineVersion = 0,
              .apiVersion = XR_CURRENT_API_VERSION,
          },

  };
  AppCommon::app_create_instance(&instanceCreateInfo, &engine);
  AppCommon::app_get_instance_properties(&engine);
  AppCommon::app_get_system_prop(&engine);
  AppCommon::app_enum_view_configuration(&engine);
  engine.state.m_views.resize(engine.state.viewCount, {XR_TYPE_VIEW});
}

void OpenXRApp::initRenderingResources() {
  AppCommon::app_init_display(&engine);
  LOGI("overlay  initRenderingResources ImGui");
  // Setup ImGui
  IMGUI_CHECKVERSION();
  LOGI("overlay  initRenderingResources ImGui 11");
  ImGui::CreateContext();
  LOGI("overlay  initRenderingResources ImGui 22");
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.IniFilename =
      nullptr; // don't want imgui to load up any settings from disk
  ImGui::StyleColorsDark(); // use dark style
  io.DisplaySize.x = engine.quadImageWidth;
  io.DisplaySize.y = engine.quadImageHeight;
  io.FontGlobalScale = 3.f;

  // Init ImGui backend
  bool imguiInitRes = ImGui_ImplOpenGL3_Init("#version 300 es");
  if (!imguiInitRes) {
    LOGI("overlay initRenderingResources AndroidXrPerfHUD::Initialize", "Failed to init ImGui");
    assert(0);
  }

  LOGI("overlay initRenderingResources end");
}

void OpenXRApp::initSwapchains() {
  AppCommon::app_enum_sc_format(&engine);
  // Projection
  {
    XrSwapchainCreateInfo swapchainCreateInfo = {
        .type = XR_TYPE_SWAPCHAIN_CREATE_INFO,
        .usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT |
                      XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT,
        .createFlags = 0,
        .format = GL_RGBA8,
        .sampleCount = 1,
        .width = engine.quadImageWidth,
        .height = engine.quadImageHeight,
        .faceCount = 1,
        .arraySize = 1,
        .mipCount = 1,
        .next = nullptr,
    };

    AppCommon::app_create_swapchain(
        &swapchainCreateInfo, &engine.state.xrSession, &engine.quadSwapchain);
  }

  // Init depth buffer
  AppCommon::app_init_depth_buffer(&engine);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_PROTECTED_EXT, GL_TRUE);
}

void OpenXRApp::destroyRenderingResources() {
  LOGI("destroyRenderingResources ");

  // shutdown ImGui backend and context
  ImGui_ImplOpenGL3_Shutdown();
  ImGui::DestroyContext();
}

void OpenXRApp::shutdownOpenXR() {
  LOGI("shutdownOpenXR ");
  XrResult result = AppCommon::app_destroy_instance(&engine);
  if (XR_SUCCESS != result) {
    LOGW("Destroy XR instance failed: %d", result);
    assert(0);
  }
}

void OpenXRApp::destroySwapchains() {
  AppCommon::app_destroy_swapchain(&engine.quadSwapchain);
  glDeleteTextures(1, &engine.depthBuffer);
  engine.depthBuffer = 0;
}

void OpenXRApp::destroyOpenXRSession() {
  LOGI("destroyOpenXRSession ");
  destroySwapchains();

  XrResult result = AppCommon::app_destroy_space(engine.state.xrLocalSpace);
  if (XR_SUCCESS != result) {
    LOGW("Destroy XR local space failed: %d", result);
    assert(0);
  }

  result = AppCommon::app_destroy_space(engine.state.xrViewSpace);
  if (XR_SUCCESS != result) {
    LOGW("Destroy XR view space failed: %d", result);
    assert(0);
  }

  if (engine.state.xrSession) {
    result = AppCommon::app_destroy_session(engine.state.xrSession);
    if (XR_SUCCESS != result) {
      LOGW("Destroy XR session failed: %d", result);
      assert(0);
    }
    engine.state.xrSession = nullptr;
  }
}

void OpenXRApp::createOpenXRSession() {
  XrSessionCreateInfoOverlayEXTX overlay_info = {
      .type = XR_TYPE_SESSION_CREATE_INFO_OVERLAY_EXTX,
      .sessionLayersPlacement = 1,
  };

  assert(!engine.state.xrSession);
  XrGraphicsBindingOpenGLESAndroidKHR gfxBinding = {
      .type = XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR,
      .next = &overlay_info,
      .display = engine.display,
      .config = engine.config,
      .context = engine.context};

  XrSessionCreateInfo createInfo = {.type = XR_TYPE_SESSION_CREATE_INFO,
                                    .next = &gfxBinding,
                                    .systemId = engine.state.xrSysId};
  AppCommon::app_create_session(&createInfo, &engine);

  // initialize space
  XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{
      XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
  referenceSpaceCreateInfo.poseInReferenceSpace = idPose;
  referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
  AppCommon::app_create_space(&referenceSpaceCreateInfo, &engine);

  XrReferenceSpaceCreateInfo viewSpaceInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
  viewSpaceInfo.poseInReferenceSpace = idPose;
  viewSpaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
  AppCommon::app_create_space(&viewSpaceInfo, &engine);

  initSwapchains();
}

void OpenXRApp::rendering() {
  XrFrameState frameState = {.type = XR_TYPE_FRAME_STATE, .next = nullptr};
  XrFrameWaitInfo frameWaitInfo = {.type = XR_TYPE_FRAME_WAIT_INFO,
                                   .next = nullptr};
  XrResult result =
      xrWaitFrame(engine.state.xrSession, &frameWaitInfo, &frameState);
  if (XR_SUCCESS != result) {
    LOGW("xrWaitFrame failed");
  } else {
    XrFrameBeginInfo frameBeginInfo = {.type = XR_TYPE_FRAME_BEGIN_INFO,
                                       .next = nullptr};
    result = xrBeginFrame(engine.state.xrSession, &frameBeginInfo);

    if (XR_SUCCESS != result) {
      LOGW("xrBeginFrame failed");
    }

    XrSwapchainImageAcquireInfo swapchainImageAcquireInfo = {
        .type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO, .next = nullptr};
    uint32_t bufferIndex;
    result = xrAcquireSwapchainImage(engine.quadSwapchain.xrSwapchain,
                                     &swapchainImageAcquireInfo, &bufferIndex);

    if (XR_SUCCESS != result) {
      LOGW("xrAcquireSwapchainImage failed, %d", result);
    }

    XrSwapchainImageWaitInfo swapchainImageWaitInfo = {
        .type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO,
        .next = nullptr,
        .timeout = 1000};
    result = xrWaitSwapchainImage(engine.quadSwapchain.xrSwapchain,
                                  &swapchainImageWaitInfo);

    if (XR_SUCCESS != result) {
      LOGW("xrWaitSwapchainImage failed %d", result);
    }

    draw(bufferIndex);

    XrSwapchainImageReleaseInfo swapchainImageReleaseInfo = {
        .type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO, .next = nullptr};
    result = xrReleaseSwapchainImage(engine.quadSwapchain.xrSwapchain,
                                     &swapchainImageReleaseInfo);

    if (XR_SUCCESS != result) {
      LOGW("xrReleaseSwapchainImage failed %d", result);
    }

    XrCompositionLayerQuad quadLayer = {
        .type = XR_TYPE_COMPOSITION_LAYER_QUAD,
        .next = nullptr,
        .layerFlags = XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT,
        .space = engine.state.xrLocalSpace,
        .eyeVisibility = XR_EYE_VISIBILITY_BOTH,
        .subImage =
            {
                .swapchain = engine.quadSwapchain.xrSwapchain,
                .imageArrayIndex = 0,
                .imageRect.offset.x = 0,
                .imageRect.offset.y = 0,
                .imageRect.extent.width =
                    static_cast<int32_t>(engine.quadImageWidth),
                .imageRect.extent.height =
                    static_cast<int32_t>(engine.quadImageHeight),
            },
        .pose = engine.quadLayerPose,
        .size = engine.quadLayerSize,
    };

    const XrCompositionLayerBaseHeader *const layers[] = {
        (const XrCompositionLayerBaseHeader *const) & quadLayer};

    XrFrameEndInfo frameEndInfo = {
        .type = XR_TYPE_FRAME_END_INFO,
        .displayTime = frameState.predictedDisplayTime,
        .layerCount = sizeof(layers) / sizeof(layers[0]),
        .layers = layers,
        .environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
        .next = nullptr};
    result = xrEndFrame(engine.state.xrSession, &frameEndInfo);

    if (XR_SUCCESS != result) {
      LOGW("xrEndFrame failed %d", result);
    }
  }
}

void OpenXRApp::draw(uint32_t imgIndex) {
  PrepareGUIData();

  glBindFramebuffer(GL_FRAMEBUFFER,
                    engine.quadSwapchain.glFramebuffers[imgIndex]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         engine.quadSwapchain.xrImages[imgIndex].image, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         engine.depthBuffer, 0);

//  ImGui_ImplOpenGL3_BackupRestoreGlStates(true);

  GLint scissor[4];
  scissor[0] = 0;
  scissor[1] = 0;
  scissor[2] = engine.quadImageWidth;
  scissor[3] = engine.quadImageHeight;
  glEnable(GL_SCISSOR_TEST);
  glViewport(0, 0, engine.quadImageWidth, engine.quadImageHeight);
  glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
  glClearColor(0.f, 0.f, 1.f, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_SCISSOR_TEST);

  ImGui::Render();

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

//  ImGui_ImplOpenGL3_BackupRestoreGlStates(false);
}

void OpenXRApp::PrepareGUIData() {
  // prepare gui data
  ImGui_ImplOpenGL3_NewFrame();
  ImGui::NewFrame();

  ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
  ImGui::SetNextWindowSize(
      ImVec2(engine.quadImageWidth, engine.quadImageHeight));
  bool showWindow = true;
  ImGui::Begin("Metrics", &showWindow,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoBackground // we'll clear ourself to alpha
                                                 // blend with other layers
                   | ImGuiWindowFlags_NoDecoration); // no window title

  // gui data
  ImVec2 txtSize = ImGui::CalcTextSize("-- Overlay sample --", nullptr);
  ImGui::SetCursorPosX((engine.quadImageWidth / 2.f) - (txtSize.x / 2.f));
  auto now = std::chrono::system_clock::now();
  auto now_time_t = std::chrono::system_clock::to_time_t(now);

  ImGui::Text("-- Overlay sample --");
  ImGui::Text("");
  ImGui::Text("Date: %s", ctime(&now_time_t));
  ImGui::Text("");
  ImGui::Text("Battery level: %d%%", mBatteryLevel.load());

  ImGui::End();
}
void OpenXRApp::startOverlayXR() {
  mOverlayStarted = true;
  initOpenXR();
  createOpenXRSession();
  mSessionStopped = false;
}

void OpenXRApp::stopOverlayXR() {
  mOverlayStarted = false;
  destroyOpenXRSession();
  shutdownOpenXR();
  mSessionStopped = true;
}

void OpenXRApp::setBatteryLevel(int level) { mBatteryLevel.store(level); }

int main() {
  auto &app = OpenXRApp::getInstance();
    LOGI("Test main ");
  app.initRenderingResources();
  LOGI("Test overlay main ");

  while (!app.mShutdown.load() || !app.mSessionStopped) {
    app.update();
  }

  app.destroyRenderingResources();
  return 0;
}
