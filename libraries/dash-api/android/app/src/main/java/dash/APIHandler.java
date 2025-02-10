package dash;

import android.bluetooth.BluetoothAdapter;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.media.AudioManager;
import android.net.Uri;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.BatteryManager;
import android.os.Environment;
import android.os.StatFs;
import android.provider.Settings;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.getpebble.android.kit.util.PebbleDictionary;

import java.lang.reflect.Method;

import data.CalendarManager;
import data.Event;

import static android.content.ContentValues.TAG;

class APIHandler {

    private static final int
        WIFI_AP_STATE_UNKNOWN = -1,
        WIFI_AP_STATE_DISABLING = 0,
        WIFI_AP_STATE_DISABLED = 1,
        WIFI_AP_STATE_ENABLING = 2,
        WIFI_AP_STATE_ENABLED = 3,
        WIFI_AP_STATE_FAILED = 4;

    private static final int
        BRIGHTNESS_MODE_MANUAL = 0,
        BRIGHTNESS_MODE_AUTO = 1;

    public static void handleGetData(Context context, int type, final PebbleDictionary out) {
        switch(type) {
            case Keys.DataTypeBatteryPercent:
                Intent batteryStatus = context.registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
                int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
                int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);

                out.addInt32(Keys.AppKeyDataValue, Math.round(((float)level / (float)scale) * 100.0F));
                break;

            case Keys.DataTypeGSMOperatorName:
                TelephonyManager tManager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
                String operatorName = tManager.getNetworkOperatorName();
                if (operatorName.length() < 1) {
                    operatorName = "Unknown";
                }

                out.addString(Keys.AppKeyDataValue, operatorName);
                break;

            case Keys.DataTypeGSMStrength:
                final TelephonyManager manager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
                final SignalListener listener = new SignalListener() {

                    @Override
                    public void onPercentKnown(int percent) {
                        out.addInt32(Keys.AppKeyDataValue, percent);
                    }

                };

                // Listen for update (first one happens ~immediately)
                manager.listen(listener, PhoneStateListener.LISTEN_SIGNAL_STRENGTHS);

                // After ~immediately, stop listening
                new Thread(new Runnable() {

                    @Override
                    public void run() {
                        try {
                            Thread.sleep(2000);
                            manager.listen(listener, PhoneStateListener.LISTEN_NONE);
                        } catch(Exception e) {
                            Log.e(TAG, "Exception waiting to stop listening to signal changes");    // Never happened yet
                            e.printStackTrace();
                        }
                    }

                }).start();
                break;

            case Keys.DataTypeWifiNetworkName:
                WifiManager wifiMgr = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
                WifiInfo wifiInfo = wifiMgr.getConnectionInfo();
                String name = wifiInfo.getSSID();

                // Strip quotes
                name = name.replace("\"", "");

                // Disconnection/Airplane mode give these strings
                if(name.equals("0x")) {
                    // On, but disconnected?
                    name = "Disconnected";
                } else if(name.equals("<unknown ssid>")) {
                    name = "Unknown";
                }

                out.addString(Keys.AppKeyDataValue, name);
                break;

            case Keys.DataTypeStorageFreeGBString: {
                StatFs statFs = new StatFs(Environment.getExternalStorageDirectory().getAbsolutePath());
                long free = statFs.getFreeBlocksLong() * (statFs.getBlockSizeLong());
                float gigs = free / 1073741824F;
                float temp = gigs;

                // Do some scaling for major and minor values
                int major = (int) Math.floor(temp);
                temp -= major;
                temp *= 10.0F;
                int minor = Math.round(temp) % 10;

                out.addString(Keys.AppKeyDataValue, "" + major + "." + minor + " GB");
            }   break;

            case Keys.DataTypeStoragePercentUsed: {
                StatFs statFs = new StatFs(Environment.getExternalStorageDirectory().getAbsolutePath());
                long free = statFs.getFreeBlocksLong() * (statFs.getBlockSizeLong());
                long total = statFs.getBlockCountLong() * statFs.getBlockSizeLong();
                int percent = Math.round(((float) free / (float) total) * 100);
                percent = 100 - percent;    // Get used, not free, as a percentage

                out.addInt32(Keys.AppKeyDataValue, percent);
            }   break;

            case Keys.DataTypeUnreadSMSCount:
                int unreadMessagesCount = 0;
                try {
                    Uri uri = Uri.parse("content://sms/inbox");
                    Cursor c = context.getContentResolver().query(uri, null, "read = 0", null, null);
                    unreadMessagesCount = c.getCount();
                    c.close();
                } catch(Exception e) {
                    Log.e(TAG, "Exception getting unread SMS: " + e.getLocalizedMessage());
                    e.printStackTrace();
                }
                out.addInt32(Keys.AppKeyDataValue, unreadMessagesCount);
                break;

            case Keys.DataTypeNextCalendarEventOneLine: {
                Event nextEvent = CalendarManager.getNextCalendarEvent(context);
                String result = "";
                if(nextEvent != null) {
                    String eventStr = nextEvent.getOneLineForm();
                    if (eventStr != null) {
                        result = eventStr;
                    }
                }
                out.addString(Keys.AppKeyDataValue, result);
            }   break;

            case Keys.DataTypeNextCalendarEventTwoLine: {
                Event nextEvent = CalendarManager.getNextCalendarEvent(context);
                String result = "";
                if(nextEvent != null) {
                    String eventStr = nextEvent.getTwoLineForm();
                    if (eventStr != null) {
                        result = eventStr;
                    }
                }
                out.addString(Keys.AppKeyDataValue, result);
            }   break;
        }
    }

    public static void handleSetFeature(Context context, int featureType, int featureState, PebbleDictionary out) {
        switch(featureType) {
            case Keys.FeatureTypeWifi:
                WifiManager wifiManager = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
                wifiManager.setWifiEnabled(featureState == Keys.FeatureStateOn);
                break;

            case Keys.FeatureTypeBluetooth:
                BluetoothAdapter.getDefaultAdapter().disable();
                break;

            case Keys.FeatureTypeRinger:
                AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
                switch (featureState) {
                    case Keys.FeatureStateRingerLoud:
                        audioManager.setRingerMode(AudioManager.RINGER_MODE_NORMAL);
                        break;
                    case Keys.FeatureStateRingerVibrate:
                        audioManager.setRingerMode(AudioManager.RINGER_MODE_VIBRATE);
                        break;
                    case Keys.FeatureStateRingerSilent: {
                        audioManager.setRingerMode(AudioManager.RINGER_MODE_SILENT);
                        audioManager.getRingerMode();   // WTF but works with this
                        audioManager.setRingerMode(AudioManager.RINGER_MODE_SILENT);
                    }   break;
                }
                break;

            case Keys.FeatureTypeAutoSync:
                ContentResolver.setMasterSyncAutomatically(featureState == Keys.FeatureStateOn);
                break;

            case Keys.FeatureTypeHotSpot:
                try {
                    WifiManager manager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
                    boolean enabled = (featureState == Keys.FeatureStateOn);
                    if (enabled) {
                        manager.setWifiEnabled(false);  //Disable WiFi first
                    }

                    // Invoke via reflection (prepare to lose this, too)
                    Method configMethod = manager.getClass().getMethod("getWifiApConfiguration");
                    WifiConfiguration config = (WifiConfiguration)configMethod.invoke(manager);
                    Method wifiMethod = manager.getClass().getMethod("setWifiApEnabled", WifiConfiguration.class, boolean.class);
                    wifiMethod.invoke(manager, config, enabled);
                } catch (Exception e) {
                    Log.e(TAG, "Exception setting WiFi AP state");
                    e.printStackTrace();
                }
                break;

            case Keys.FeatureTypeAutoBrightness:
                ContentResolver resolver = context.getContentResolver();
                Settings.System.putInt(resolver, Settings.System.SCREEN_BRIGHTNESS_MODE,
                        (featureState == Keys.FeatureStateOn) ? BRIGHTNESS_MODE_AUTO : BRIGHTNESS_MODE_MANUAL);
                break;
        }
    }

    public static void handleGetFeature(Context context, int featureType, PebbleDictionary out) {
        switch(featureType) {
            case Keys.FeatureTypeWifi:
                WifiManager wifiManager = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
                out.addInt32(Keys.AppKeyFeatureState,
                        wifiManager.isWifiEnabled() ? Keys.FeatureStateOn : Keys.FeatureStateOff);
                break;

            case Keys.FeatureTypeBluetooth:
                out.addInt32(Keys.AppKeyFeatureState, Keys.FeatureStateOn);
                break;

            case Keys.FeatureTypeRinger:
                AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
                switch (audioManager.getRingerMode()) {
                    case AudioManager.RINGER_MODE_NORMAL:
                        out.addInt32(Keys.AppKeyFeatureState, Keys.FeatureStateRingerLoud);
                        break;
                    case AudioManager.RINGER_MODE_VIBRATE:
                        out.addInt32(Keys.AppKeyFeatureState, Keys.FeatureStateRingerVibrate);
                        break;
                    case AudioManager.RINGER_MODE_SILENT:
                        out.addInt32(Keys.AppKeyFeatureState, Keys.FeatureStateRingerSilent);
                        break;
                }
                break;

            case Keys.FeatureTypeAutoSync:
                out.addInt32(Keys.AppKeyFeatureState,
                        ContentResolver.getMasterSyncAutomatically() ? Keys.FeatureStateOn : Keys.FeatureStateOff);
                break;

            case Keys.FeatureTypeHotSpot:
                int state = WIFI_AP_STATE_UNKNOWN;
                try {
                    WifiManager manager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
                    Method method = manager.getClass().getMethod("getWifiApState");

                    // Get the API state value
                    state = (Integer) method.invoke(manager);
                    if (state > 10) {
                        state -= 10; //Changes in Android 4.0
                    }
                } catch(Exception e) {
                    Log.e(TAG, "Error getting AP state");
                    e.printStackTrace();
                }

                // Well, is it on or off??
                if(state == WIFI_AP_STATE_ENABLED || state == WIFI_AP_STATE_ENABLING) {
                    out.addInt32(Keys.AppKeyFeatureState, Keys.FeatureStateOn);
                } else if(state == WIFI_AP_STATE_DISABLED || state == WIFI_AP_STATE_DISABLING) {
                    out.addInt32(Keys.AppKeyFeatureState, Keys.FeatureStateOff);
                } else {
                    Log.e(TAG, "Wifi AP in Unknown state after API query");
                    out.addInt32(Keys.AppKeyFeatureState, Keys.FeatureStateUnknown);
                }
                break;

            case Keys.FeatureTypeAutoBrightness:
                try {
                    int mode = Settings.System.getInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE);
                    out.addInt32(Keys.AppKeyFeatureState,
                            (mode == BRIGHTNESS_MODE_AUTO) ? Keys.FeatureStateOn : Keys.FeatureStateOff);
                } catch (Exception e) {
                    Log.e(TAG, "Exception getting autobrightness! Returning false.");
                    e.printStackTrace();
                    out.addInt32(Keys.AppKeyFeatureState, Keys.FeatureStateOff);
                }
                break;
        }
    }

}
