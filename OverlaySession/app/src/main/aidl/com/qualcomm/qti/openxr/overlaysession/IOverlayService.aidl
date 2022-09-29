// IOverlayService.aidl
package com.qualcomm.qti.openxr.overlaysession;

import android.os.ParcelFileDescriptor;

interface IOverlayService {
    void startOverlay();
    void stopOverlay();
}