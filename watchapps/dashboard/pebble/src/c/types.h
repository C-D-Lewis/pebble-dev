#pragma once

#include <pebble.h>

// TODO: Pebble packet schema and callbacks library
typedef enum {
  AppKeyGSMName = 0,
  AppKeyGSMPercent,
  AppKeyWifiName,
  AppKeyBatteryPercent,
  AppKeyStorageFreeGBMajor,
  AppKeyStorageFreeGBMinor,
  AppKeyToggleWifi,           // Begin iterable sequence
  AppKeyToggleData,           //
  AppKeyToggleBluetooth,      //
  AppKeyToggleRinger,         // 
  AppKeyToggleSync,           //
  AppKeyToggleWifiAP,         // 
  AppKeyToggleFindPhone,      //
  AppKeyToggleLockPhone,      //
  AppKeyToggleAutoBrightness, //
  AppKeyToggleOrderString,    // End iterable sequence (non-inclusive)
  AppKeyIsLollipop,
  AppKeyStoragePercent,
  AppKeyQuickLaunchEnabled,
  AppKeyQuickLaunchType,
  AppKeyVersion
} AppKey; // KEEP IN ORDER

typedef enum {
  MessageTypeRequestAll = 543,
  MessageTypeToggle
} MessageType;

// Must stay linear for data_init()!
typedef enum {
  ToggleTypeWifi = 0,
  ToggleTypeData,
  ToggleTypeBluetooth,
  ToggleTypeRinger,
  ToggleTypeSync,
  ToggleTypeWifiAP,
  ToggleTypeFindPhone,
  ToggleTypeLockPhone,
  ToggleTypeAutoBrightness,

  ToggleTypeCount
} ToggleType;

typedef enum {
  ToggleStateWaiting = 0,
  ToggleStateOff,
  ToggleStateOn,
  ToggleStateLoud,
  ToggleStateVibrate,
  ToggleStateSilent,
  ToggleStateBrightnessAutomatic,
  ToggleStateBrightnessManual,
  ToggleStateLocked
} ToggleState;

typedef enum {
  ErrorCodeNoRoot = 0,
  ErrorCodeNoDeviceAdmin,
  ErrorCodeLockSuccess,
  ErrorCodeDataNotEnabled,
  ErrorCodeWrongVersion = 40
} ErrorCode;

typedef struct {
  int type;
  int state;
  int hour;
  int minute;
  int set;
  int wakeup_id;
} Event;
// List row keys are row index

typedef enum {
  PersistKeyIsLollipop = 453786
} PersistKey;