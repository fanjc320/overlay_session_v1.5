/****************************************************************
 * Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ****************************************************************/

#include <jni.h>
#include <string>
#include <thread>

#include "openxr_overlay_app.h"

static std::unique_ptr<std::thread> appThread{};

extern "C" JNIEXPORT void JNICALL
Java_com_qualcomm_qti_openxr_overlaysession_OverlayServiceImpl_nativeInitial(
    JNIEnv *env, jobject thiz, jobject context) {
  LOGI("nativeInitial");

  if (appThread) {
    LOGI("already initial, shouldn't happen! ");
  }
  JavaVM *vm;
  env->GetJavaVM(&vm);
  OpenXRApp::getInstance().initial(vm, env->NewGlobalRef(context));
  appThread = std::make_unique<std::thread>([&]() { main(); });
}

extern "C" JNIEXPORT void JNICALL
Java_com_qualcomm_qti_openxr_overlaysession_OverlayServiceImpl_nativeShutdown(
    JNIEnv *env, jobject thiz) {
  LOGI("nativeShutdown");

  if (!appThread) {
    LOGI("already shutdown, shouldn't happen! ");
    return;
  }
  OpenXRApp::getInstance().shutdown();
  env->DeleteGlobalRef(
      static_cast<jobject>(OpenXRApp::getInstance().engine.serviceContext));
  appThread->join();
  appThread.reset(nullptr);
}

extern "C" JNIEXPORT void JNICALL
Java_com_qualcomm_qti_openxr_overlaysession_OverlayServiceImpl_nativeStartOverlay(
    JNIEnv *env, jobject thiz) {
   LOGI("nativeStartOverlay");
  OpenXRApp::getInstance().startOverlay();
}

extern "C" JNIEXPORT void JNICALL
Java_com_qualcomm_qti_openxr_overlaysession_OverlayServiceImpl_nativeStopOverlay(
    JNIEnv *env, jobject thiz) {
   LOGI("nativeStopOverlay");
  OpenXRApp::getInstance().endOverlay();
}

extern "C" JNIEXPORT void JNICALL
Java_com_qualcomm_qti_openxr_overlaysession_OverlayServiceImpl_nativeSetBatteryLevel(
    JNIEnv *env, jobject thiz, jint level) {
  OpenXRApp::getInstance().setBatteryLevel(level);
}