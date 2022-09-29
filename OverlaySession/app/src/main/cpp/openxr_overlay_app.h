/****************************************************************
 * Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ****************************************************************/

#include <android/log.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <initializer_list>
#include <memory>
#include <vector>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>

#include "Extensions.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/transform.hpp"
#include <glm/gtx/quaternion.hpp>

#include "AppCommon.h"
#include "Geometry.h"
#include "KtxLoader.h"
#include "Shader.h"
//#include "common/xr_linear.h"
#include "xr_linear.h"

#include "imgui.h"
#include "imgui_internal.h"
//#include "imgui_impl_opengl3.h"
#include "backends/imgui_impl_opengl3.h"

#define TAG "OpenXR overlay"

#define LOGI(...)                                                              \
  ((void)__android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))
#define LOGW(...)                                                              \
  ((void)__android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__))

int main();

#define MODEL_POSITION_RADIUS 4.0f
#define BUFFER_SIZE 512
#define ROTATION_SPEED 5.0f
#define NUM_MODELS 6

struct Engine : public AppCommon::base_engine {
  JavaVM *vm = nullptr;
  void *serviceContext = nullptr;

  const uint32_t quadImageWidth = 768;
  const uint32_t quadImageHeight = 256;

  AppCommon::Swapchain quadSwapchain;

  XrExtent2Df quadLayerSize = {.width = 0.45f, .height = 0.15f};
  XrPosef  quadLayerPose = {.orientation = {.x = 0.f,.y=0.f,.z=0.f,.w = 1.f},
                            .position = {.x = 0.f,.y=-0.05f,.z=-0.75f}};

  GLuint depthBuffer; // common depth buffer... should be alright as we assume
};

class OpenXRApp {
public:
  std::atomic<bool> mOverlayHided;
  std::atomic<bool> mRequestExitSession;
  std::atomic<bool> mShutdown;
  bool mSessionStopped = true;
  bool mOverlayStarted = false;

  std::atomic<int> mBatteryLevel;
public:
  static OpenXRApp &getInstance() {
    static OpenXRApp instance;
    return instance;
  }

  // initial openxr context and resources
  void initial(JavaVM *vm, void *context);
  // destroy openxr context and resources
  void shutdown();
  // create session and begin session
  void startOverlay();
  // end session and destroy session
  void endOverlay();

public:
  Engine engine;
  void initOpenXR();
  void initRenderingResources();
  void shutdownOpenXR();
  void destroyRenderingResources();
  void update();
  void setBatteryLevel(int level);

private:
  void initSwapchains();
  void destroySwapchains();
  void createOpenXRSession();
  void destroyOpenXRSession();
  void rendering();
  void draw(uint32_t imgIndex);
  void PrepareGUIData();
  void startOverlayXR();
  void stopOverlayXR();
  OpenXRApp(){};
};