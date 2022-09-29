adb shell setprop com.qualcomm.openxr.overlaysession.test true
adb shell setprop com.qualcomm.openxr.overlaysession.show false
adb shell am start-foreground-service com.qualcomm.qti.openxr.overlaysession/.OverlayService
pause