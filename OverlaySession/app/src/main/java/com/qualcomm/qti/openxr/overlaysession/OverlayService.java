/****************************************************************
 * Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ****************************************************************/

package com.qualcomm.qti.openxr.overlaysession;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.IBinder;
import android.util.Log;
import androidx.core.app.NotificationCompat;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class OverlayService extends Service {
  private static final String TAG = "OverlayService";
  public static final String FIRST_TIME_TAG = "first_time";
  public static final String ASSETS_SUB_FOLDER_NAME = "raw";
  public static final int BUFFER_SIZE = 1024;

  public static String adbTestProp = "com.qualcomm.openxr.overlaysession.test";
  public static String adbTestPropFlag =
      "com.qualcomm.openxr.overlaysession.show";
  public static String enableTest = "false";
  ADBTestThread testThread = null;

  private OverlayServiceImpl binder;

  private final BroadcastReceiver batteryReceiver = new BroadcastReceiver() {
    int batteryLevel = -1;
    @Override
    public void onReceive(Context context, Intent intent) {
      batteryLevel = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
      binder.SetBatteryLevel(batteryLevel);
      // Log.i(TAG, "onReceive: battery level :" + batteryLevel);
    }
  };

  @Override
  public void onCreate() {
    super.onCreate();
    Log.e(TAG, "onCreate: ");

    // startService self if client only call bindService
    Intent intent = new Intent(this, OverlayService.class);
    startService(intent);

    binder = new OverlayServiceImpl();
    binder.Initial(this);

    IntentFilter filter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
    registerReceiver(batteryReceiver, filter);

    try {
      enableTest = getProp(adbTestProp);
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    } catch (NoSuchMethodException e) {
      e.printStackTrace();
    } catch (InvocationTargetException e) {
      e.printStackTrace();
    } catch (IllegalAccessException e) {
      e.printStackTrace();
    }
    if (enableTest.equals("true")) {
      testThread = new ADBTestThread();
      testThread.start();
    }
  }

  @Override
  public IBinder onBind(Intent intent) {
    Log.e(TAG, "onBind: ");
    return binder;
  }

  @Override
  public boolean onUnbind(Intent intent) {
    Log.e(TAG, "onUnbind: ");
    return true;
  }

  @Override
  public void onDestroy() {
    Log.e(TAG, "onDestroy: ");
    unregisterReceiver(batteryReceiver);
    binder.shutdown();
    if (testThread != null) {
      try {
        testThread.stop();
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }
    super.onDestroy();
  }

  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {
    Log.e(TAG, "onStartCommand: ");

    NotificationManager manager =
        (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
    NotificationChannel channel = new NotificationChannel(
        "overlay", "overlay", NotificationManager.IMPORTANCE_HIGH);
    channel.setAllowBubbles(false);
    channel.enableVibration(false);
    manager.createNotificationChannel(channel);

    NotificationCompat.Builder foregroundNotification =
        new NotificationCompat.Builder(this, "overlay");
    foregroundNotification.setOngoing(true);
    foregroundNotification.setContentTitle("Overlay session service running")
        .setSmallIcon(R.drawable.ic_launcher_background)
        .setContentText("Overlay session service running")
        .setNotificationSilent();

    Notification notification = foregroundNotification.build();

    startForeground(111, notification);
    return super.onStartCommand(intent, flags, startId);
  }

  String getProp(String propName)
      throws ClassNotFoundException, NoSuchMethodException,
             InvocationTargetException, IllegalAccessException {
    Class clazz = null;
    clazz = Class.forName("android.os.SystemProperties");
    Method method = clazz.getDeclaredMethod("get", String.class);
    String prop = (String)method.invoke(null, propName);
    // Log.e(TAG, "getProp: " + propName + " value " + prop);
    return prop;
  }

  class ADBTestThread implements Runnable {
    private Thread t;
    private volatile boolean stop = false;
    @Override
    public void run() {
      String prop = "false";
      String tmpProp = "false";
      while (!stop) {
        try {
          tmpProp = getProp(adbTestPropFlag);
        } catch (ClassNotFoundException e) {
          e.printStackTrace();
        } catch (NoSuchMethodException e) {
          e.printStackTrace();
        } catch (InvocationTargetException e) {
          e.printStackTrace();
        } catch (IllegalAccessException e) {
          e.printStackTrace();
        }
        if (tmpProp.equals(prop)) {
        } else if (tmpProp.equals("true")) {
          binder.startOverlay();
          prop = tmpProp;
        } else if (tmpProp.equals("false")) {
          binder.stopOverlay();
          prop = tmpProp;
        }
        try {
          Thread.sleep(500);
        } catch (InterruptedException e) {
          e.printStackTrace();
        }
      }
    }
    public void start() {
      if (t == null) {
        t = new Thread(this, "overlay test thread");
        t.start();
      }
    }
    public void stop() throws InterruptedException {
      stop = true;
      if (t != null) {
        t.join();
      }
    }
  }
}