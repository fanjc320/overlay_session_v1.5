adb shell setprop com.qualcomm.openxr.overlaysession.test false
adb shell setprop com.qualcomm.openxr.overlaysession.show false
adb shell am stopservice com.qualcomm.qti.openxr.overlaysession/.OverlayService
pause