# dashboard-api-status

Status of the Android APIs that Dashboard relies upon. Because they will
probably start disappearing one by one in the future (Data already has!).

These are provided either as the API methods, or pseudocode if they're hacky.


## Wifi

Get: `WifiManager.setWifiEnabled()`

Set: `WifiManager.isWifiEnabled()`


## Data

Get: `NetworkInfo.getNetworkInfo(ConnectivityManager.TYPE_MOBILE).getState()`

Set (Android 4.4 and below): 
```
ConnectivityManager.class.getDeclaredMethod("setMobileDataEnabled", boolean.class).invoke(...)
```

Set (Android 5.0 and above, **requires root**):
```
Runtime.getRuntime().exec("su") ...

"service call phone" 
  + getTransactionCode("com.android.internal.telephony.ITelephony", "setDataEnabled")
  + " i32 " + (enabled ? "1" : "0")
  + "\n exit"
```


## Bluetooth

Get: `BluetoothAdapter.getDefaultAdapter().isEnabled()`

Set: `BluetoothAdapter.getDefaultAdapter().enable()/.disable()`


## Ringer

Get: `AudioManager.getRingerMode()`

Set: `AudioManager.setRingerMode(...)`


## AutoSync

Get: `ContentResolver.getMasterSyncAutomatically()`

Set: `ContentResolver.setMasterSyncAutomatically()`


## Wifi AP (Hotspot)

Get: `WifiManager.getClass().getMethod("getWifiApState").invoke(...)`

Set: 
```
WifiManager.getClass().getMethod("setWifiApEnabled", WifiConfiguration.class, boolean.class).invoke(...)
```


## Lock Screen

Get: N/A

Set (Requires Device Admin permission): `DevicePolicyManager.lockNow()`


## Auto Brightness

Get: 
```
Settings.System.getInt(ContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE) == 1
```

Set:
```
Settings.System.putInt(ContentResolver, Settings.System.SCREEN_BRIGHTNESS_MODE, auto ? 1 : 0);
```
