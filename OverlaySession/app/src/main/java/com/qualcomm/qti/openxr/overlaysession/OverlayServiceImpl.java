/****************************************************************
 * Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ****************************************************************/

package com.qualcomm.qti.openxr.overlaysession;

import android.content.Context;
import android.util.Log;

public class OverlayServiceImpl extends IOverlayService.Stub {
  private static final String TAG = "OverlayServiceIml";

  static { System.loadLibrary("native-lib"); }

  public OverlayServiceImpl() { Log.i(TAG, "OverlayServiceImpl: "); }

  @Override
  public void startOverlay() {
    // Log.i(TAG, "startOverlay: ");
    nativeStartOverlay();
  }

  @Override
  public void stopOverlay() {
    // Log.i(TAG, "stopOverlay: ");
    nativeStopOverlay();
  }

  public void shutdown() { nativeShutdown(); }

  public void Initial(Context context) { nativeInitial(context); }

  public void SetBatteryLevel(int level) { nativeSetBatteryLevel(level); }

  private native void nativeInitial(Context context);
  private native void nativeShutdown();
  private native void nativeStartOverlay();
  private native void nativeStopOverlay();
  private native void nativeSetBatteryLevel(int level);
}
