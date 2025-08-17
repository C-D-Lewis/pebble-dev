package background;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.app.admin.DevicePolicyManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.media.AudioManager;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.BatteryManager;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.StatFs;
import android.preference.PreferenceManager;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;
import com.stericson.RootTools.RootTools;
import com.wordpress.ninedof.dashboard.R;

import activity.Landing;
import androidx.annotation.RequiresApi;
import androidx.core.app.NotificationCompat;
import cl_toolkit.Logger;
import cl_toolkit.Platform;
import cl_toolkit.Radios;
import cl_toolkit.Root;
import cl_toolkit.UserInterface;
import config.Build;
import config.Keys;
import config.Runtime;
import util.PebbleUtils;
import util.VersionCheck;

public class HandlerService extends Service {
    //Configuration
    private final static String TAG = HandlerService.class.getName();

    private static final int FOREGROUND_ID = 3489343;

    /**
     * See what the watch wants
     * This is the real meat and potatoes of Dashboard!
     *
     * @param dict Incoming data
     */
    public void parse(PebbleDictionary dict) {
        final Context context = getApplicationContext();

        Runtime.log(context, TAG, "Dashboard v" + Build.VERSION
                + " (compatible v" + Build.WATCH_APP_COMPATIBLE_VERSION + ")", Logger.INFO);

        // Version check
        if(PebbleUtils.hasString(dict, Keys.AppKeyVersion)) {
            String version = dict.getString(Keys.AppKeyVersion);
            if(!version.equals(Build.WATCH_APP_COMPATIBLE_VERSION)) {
                Runtime.log(context, TAG, "Wrong version! Watch is at " + version, Logger.ERROR);

                PebbleDictionary out = new PebbleDictionary();
                dict.addInt32(Keys.ErrorCodeWrongVersion, 0);
                PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, out);
                Runtime.log(context, TAG, "Send wrong version key to watch app", Logger.INFO);
                return;  // Nothing more to do here
            }

            Runtime.log(context, TAG, "Correct version: " + version, Logger.INFO);
        }

        //Requesting status of all toggles?
        if (PebbleUtils.hasInt(dict, Keys.MessageTypeRequestAll)) {
            Runtime.log(context, TAG, "Got sync all request", Logger.INFO);
            final PebbleDictionary out = new PebbleDictionary();
            out.addInt8(Keys.MessageTypeRequestAll, (byte) 1);

            addIsLollipop(out);
            addBatteryLevel(out);
//            addBatteryPlugged(out); descoped
            addOperatorName(out);
            addOperatorStrength(out);
            addWifiName(out);
            addFreeSpace(out);
            addToggleOrder(out);
            addQuickLaunchSettings(out);

            addWifiState(out);
            addMobileDataState(out);
            addBluetoothState(out);
            addRingerState(out);
            addAutoSyncState(out);
            addWifiApState(out);
            addPhoneState(out);
            addLockState(out);
            addAutoBrightnessState(out);
//            addFindPhoneState(out); descoped, tempramental

            new Thread(new Runnable() {

                @Override
                public void run() {
                    try {
                        Thread.sleep(300);  // Delay sending while we gather GSM signal callbacks
                                            // This could be as low as 100ms, but hard to test
                        PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, out);
                        Runtime.log(context, TAG, "Sent MessageTypeRequestAll response!", Logger.INFO);
                    } catch(Exception e) {
                        Runtime.log(context, TAG, "Exception waiting to send dictionary", Logger.ERROR);
                        Runtime.logStackTrace(context, e);
                    }
                }

            }).start();

        // It's an individual toggle request
        } else if(PebbleUtils.hasInt(dict, Keys.MessageTypeToggle)) {
            Runtime.log(context, TAG, "Got toggle request", Logger.INFO);

            // Toggle WiFi
            if (PebbleUtils.hasInt(dict, Keys.AppKeyToggleWifi)) {
                int newState = Keys.ToggleStateOff;
                // These forks may look redundant, but they will guard against improper values from the watchapp
                if (PebbleUtils.getInt(dict, Keys.AppKeyToggleWifi) == Keys.ToggleStateOn) {
                    // Turn it on
                    newState = Keys.ToggleStateOn;
                } else if (PebbleUtils.getInt(dict, Keys.AppKeyToggleWifi) == Keys.ToggleStateOff) {
                    // Turn it off
                    newState = Keys.ToggleStateOff;
                }

                boolean success = Radios.setWiFiState(context, newState == Keys.ToggleStateOn);
                Runtime.log(context, TAG, "Wifi " + newState + " succeeded: " + success, Logger.INFO);

                PebbleDictionary out = new PebbleDictionary();
                out.addInt8(Keys.AppKeyToggleWifi, (byte) newState);
                PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, out);
            }

            // Toggle Data
            if (PebbleUtils.hasInt(dict, Keys.AppKeyToggleData)) {
                PebbleDictionary out = new PebbleDictionary();

                int newState = Keys.ToggleStateOff;
                if (PebbleUtils.getInt(dict, Keys.AppKeyToggleData) == Keys.ToggleStateOn) {
                    newState = Keys.ToggleStateOn;
                } else if (PebbleUtils.getInt(dict, Keys.AppKeyToggleData) == Keys.ToggleStateOff) {
                    newState = Keys.ToggleStateOff;
                }
                boolean newStateB = newState == Keys.ToggleStateOn;
                Runtime.log(context, TAG, "Attempting to toggle data " + (newStateB ? "ON" : "OFF"), Logger.INFO);

                if (!Platform.isLollipopOrAbove()) {
                    // Simple, use the reflected API
                    boolean success = Radios.setMobileDataEnabled(context, newStateB);
                    Runtime.log(context, TAG, "Data " + newState + " succeeded: " + success, Logger.INFO);
                    out.addInt8(Keys.AppKeyToggleData, (byte) newState);
                } else {
                    // Not so simple, we need root!
                    try {
                        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
                        if (prefs.getBoolean(Keys.PREF_KEY_DATA_ENABLED, false)) {
                            Runtime.log(context, TAG, "Using root method.", Logger.INFO);

                            // Attempt with root
                            if (RootTools.isAccessGiven()) {
                                // Attempt to set to new state (we don't actually know how the su command went)
                                boolean success = Root.rootSetMobileDataEnabled(context, newStateB);
                                Runtime.log(context, TAG, "Root Mobile data success: " + success, Logger.INFO);
                                out.addInt8(Keys.AppKeyToggleData, (byte) newState);
                            } else {
                                // Failed to get root, disable
                                out.addInt8(Keys.AppKeyToggleData, (byte) Keys.ErrorCodeNoRoot);
                                Runtime.log(context, TAG, "Root denied when attempting toggle", Logger.ERROR);
                            }
                        } else {
                            // Failed to get root, disable
                            out.addInt8(Keys.AppKeyToggleData, (byte) Keys.ErrorCodeDataNotEnabled);
//                            notifyRootRequest(); User does not want to be bothered
                            Runtime.log(context, TAG, "Data with root not enabled when attempting toggle", Logger.ERROR);
                        }
                    } catch (Exception e) {
                        // Failed to get root, disable
                        out.addInt8(Keys.AppKeyToggleData, (byte) Keys.ErrorCodeNoRoot);
                        Runtime.log(context, TAG, "Root denied when attempting toggle", Logger.ERROR);
                        Runtime.logStackTrace(context, e);
                    }
                }
                PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, out);
            }

            //Toggle BT? (MUST BE MANUALLY RECONNECTED!)
            if (PebbleUtils.hasInt(dict, Keys.AppKeyToggleBluetooth)) {
                boolean success = Radios.setBluetoothState(false);
                Runtime.log(context, TAG, "BT off succeeded: " + success, Logger.INFO);

                PebbleDictionary out = new PebbleDictionary();
                out.addInt8(Keys.AppKeyToggleBluetooth, (byte) Keys.ToggleStateOff);    // Always
                PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, out);
            }

            //Set Ringer?
            if (PebbleUtils.hasInt(dict, Keys.AppKeyToggleRinger)) {
                AudioManager aManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

                int mode = PebbleUtils.getInt(dict, Keys.AppKeyToggleRinger);

                // Any mode other than silent may require unsetting DnD
                if (mode != Keys.ToggleStateSilent) {
                    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
                        NotificationManager mNotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
                        mNotificationManager.setInterruptionFilter(NotificationManager.INTERRUPTION_FILTER_ALL);
                    }
                }

                switch (mode) {
                    case Keys.ToggleStateLoud:
                        aManager.setRingerMode(AudioManager.RINGER_MODE_NORMAL);
                        Runtime.log(context, TAG, "Loud mode attempted.", Logger.INFO);
                        break;
                    case Keys.ToggleStateVibrate:
                        aManager.setRingerMode(AudioManager.RINGER_MODE_VIBRATE);
                        Runtime.log(context, TAG, "Vibrate mode attempted.", Logger.INFO);
                        break;
                    case Keys.ToggleStateSilent: {
                        try {
                            Runtime.log(context, TAG, "Silent mode attempting....", Logger.INFO);
                            // Damn you Android 5.0 Priority mode with no API!!
                            // Also damn you DND in Android N!
                            // https://stackoverflow.com/questions/58044974/enable-silent-mode-in-android-without-triggering-do-not-disturb
                            aManager.setRingerMode(AudioManager.RINGER_MODE_NORMAL);
                            aManager.setRingerMode(AudioManager.RINGER_MODE_SILENT);
                            int newMode = aManager.getRingerMode();   // WTF but works
                            Runtime.log(context, TAG, "Ringer mode now " + newMode, Logger.INFO);
                            if (newMode != AudioManager.RINGER_MODE_SILENT) {
                                Runtime.log(context, TAG, "New ringer mode does not match requested mode!", Logger.ERROR);
                            } else {
                                Runtime.log(context, TAG, "New ringer mode matched RINGER_MODE_SILENT (" + AudioManager.RINGER_MODE_SILENT + "): " + newMode, Logger.DEBUG);
                            }
                            Runtime.log(context, TAG, "Silent mode attempted.", Logger.INFO);
                        } catch (Exception e) {
                            Runtime.log(context, TAG, "Error toggling silent mode: " + e.getMessage(), Logger.ERROR);
                        }
                    }   break;
                }

                PebbleDictionary out = new PebbleDictionary();
                out.addInt8(Keys.AppKeyToggleRinger, (byte) mode);  // Echo back for updating UI
                PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, out);
            }

            //Set AutoSync?
            if (PebbleUtils.hasInt(dict, Keys.AppKeyToggleSync)) {
                int newState = Keys.ToggleStateOff;
                if (PebbleUtils.getInt(dict, Keys.AppKeyToggleSync) == Keys.ToggleStateOn) {
                    // Turn it on
                    newState = Keys.ToggleStateOn;
                } else if (PebbleUtils.getInt(dict, Keys.AppKeyToggleSync) == Keys.ToggleStateOff) {
                    // Turn it off
                    newState = Keys.ToggleStateOff;
//                } else {
//                    // Invert it
//                    newState = !ContentResolver.getMasterSyncAutomatically();
                }

                ContentResolver.setMasterSyncAutomatically(newState == Keys.ToggleStateOn);   // I'm worried this will go away soon
                Runtime.log(context, TAG, "Sync " + newState + " attempted.", Logger.INFO);

                PebbleDictionary out = new PebbleDictionary();
                out.addInt8(Keys.AppKeyToggleSync, (byte) newState);
                PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, out);
            }

            //Set Wifi AP?
            if (PebbleUtils.hasInt(dict, Keys.AppKeyToggleWifiAP)) {
                int newState = Keys.ToggleStateOff;
                if (PebbleUtils.getInt(dict, Keys.AppKeyToggleWifiAP) == Keys.ToggleStateOn) {
                    // Turn it on
                    newState = Keys.ToggleStateOn;
                } else if (PebbleUtils.getInt(dict, Keys.AppKeyToggleWifiAP) == Keys.ToggleStateOff) {
                    // Turn it off
                    newState = Keys.ToggleStateOff;
//                } else {
//                    // Invert it
//                    newState = !Radios.getWifiApEnabled(context);
                }

                // Disable
                if (newState != Keys.ToggleStateOn) {
                    //Disable AP
                    final boolean success = Radios.setWifiApEnabled(context, false);
                    new Handler().postDelayed(new Runnable() {

                        @Override
                        public void run() {
                            //Wait for AP to disable
                            Runtime.log(context, TAG, "WiFi AP off: " + success, Logger.INFO);

                            //Respond to Pebble
                            PebbleDictionary out = new PebbleDictionary();
                            out.addInt8(Keys.AppKeyToggleWifiAP, (byte) Keys.ToggleStateOff);
                            PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, out);
                        }

                    }, 3000L);
                }

                // Enable
                else {
                    //Turn on Network Data and AP
                    SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
                    if (!Platform.isLollipopOrAbove()) {
                        Radios.setMobileDataEnabled(context, true);
                    } else if (prefs.getBoolean(Keys.PREF_KEY_DATA_ENABLED, false)) {
                        try {
                            boolean success = Root.rootSetMobileDataEnabled(context, true);
                            Runtime.log(context, TAG, "Root Mobile data success: " + success, Logger.INFO);
                        } catch (Exception e) {
                            Runtime.log(context, TAG, "Exception performing AP data toggle ON!", Logger.ERROR);
                        }
                    } else {
                        Runtime.log(context, TAG, "5.0+ data state not enabled, user has not opted in to root method.", Logger.INFO);
                    }

                    boolean success = Radios.setWifiApEnabled(context, true);
                    Runtime.log(context, TAG, "WiFi AP on: " + success, Logger.INFO);

                    //Respond to Pebble
                    PebbleDictionary out = new PebbleDictionary();
                    out.addInt8(Keys.AppKeyToggleWifiAP, (byte) Keys.ToggleStateOn);
                    out.addInt8(Keys.AppKeyToggleWifi, (byte) Keys.ToggleStateOff);
                    out.addInt8(Keys.AppKeyToggleData, (byte) Keys.ToggleStateOn);

                    PebbleKit.sendDataToPebble(this, Build.WATCH_APP_UUID, out);
                }
            }

            //Sound find phone alarm - toggle only
            if (PebbleUtils.hasInt(dict, Keys.AppKeyToggleFindPhone)) {
                Runtime.log(context, TAG, "Toggling find phone...", Logger.INFO);
                startService(new Intent(context, FindPhone.class));
            }

            // Lock screen?
            if (PebbleUtils.hasInt(dict, Keys.AppKeyToggleLockPhone)) {
                PebbleDictionary out = new PebbleDictionary();
                boolean success;
                try {
                    DevicePolicyManager man = (DevicePolicyManager) context.getSystemService(Context.DEVICE_POLICY_SERVICE);
                    man.lockNow();

                    success = true;
                    out.addInt8(Keys.AppKeyToggleLockPhone, (byte) Keys.ErrorCodeLockSuccess);
                } catch (Exception e) {
                    Runtime.log(context, TAG, "Failed to lock. Not device admin?", Logger.ERROR);
                    Runtime.logStackTrace(context, e);

                    success = false;
                    out.addInt8(Keys.AppKeyToggleLockPhone, (byte) Keys.ErrorCodeNoDeviceAdmin);

                    // Remember we failed
                    SharedPreferences.Editor ed = PreferenceManager.getDefaultSharedPreferences(context).edit();
                    ed.putBoolean(Keys.PREF_KEY_ADMIN, false);
                    ed.commit();
                }

                Runtime.log(context, TAG, "Lock screen attempt: " + success, Logger.INFO);
                PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, out);
            }

            // Auto brightness?
            if (PebbleUtils.hasInt(dict, Keys.AppKeyToggleAutoBrightness)) {
                int newState = Keys.ToggleStateBrightnessAutomatic;
                if (PebbleUtils.getInt(dict, Keys.AppKeyToggleAutoBrightness) == Keys.ToggleStateBrightnessAutomatic) {
                    // Turn it on
                    newState = Keys.ToggleStateBrightnessAutomatic;
                } else if (PebbleUtils.getInt(dict, Keys.AppKeyToggleAutoBrightness) == Keys.ToggleStateBrightnessManual) {
                    // Turn it off
                    newState = Keys.ToggleStateBrightnessManual;
//                } else {
//                    // Invert it
//                    newState = !UserInterface.getAutoBrightnessEnabled(context);
                }

                boolean newStateB = newState == Keys.ToggleStateBrightnessAutomatic;
                UserInterface.setAutoBrightnessEnabled(context, newStateB);
                Runtime.log(context, TAG, "Setting auto brightness to " + (newStateB ? "ON" : "OFF"), Logger.INFO);

                PebbleDictionary out = new PebbleDictionary();
                out.addInt8(Keys.AppKeyToggleAutoBrightness, (byte) newState);
                PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, out);
            }
        } else {
            Runtime.log(context, TAG, "Unknown MessageType!", Logger.ERROR);

            // Could be 4.7 asking for version (4.8 won't answer how it expects!)
            PebbleDictionary response = new PebbleDictionary();
            response.addInt32(VersionCheck.KEY_VERSION_CHECK_FAILURE, 0);
            PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, response);
        }
    }

    private void addQuickLaunchSettings(PebbleDictionary out) {
        Context context = getApplicationContext();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);

        boolean enabled = prefs.getBoolean(Keys.PREF_KEY_QUICK_LAUNCH_ENABLED, false);
        Runtime.log(context, TAG, "Quick launch enabled? " + enabled, Logger.INFO);

        int type = prefs.getInt(Keys.PREF_KEY_QUICK_LAUNCH_TYPE, Keys.ToggleTypeWifi);
        Runtime.log(context, TAG, "Chosen quick launch type: " + type, Logger.INFO);

        out.addInt8(Keys.AppKeyQuickLaunchEnabled, (byte)(enabled ? 1 : 0));
        out.addInt8(Keys.AppKeyQuickLaunchType, (byte) type);
    }

    private void addOperatorStrength(final PebbleDictionary out) {
        final Context context = getApplicationContext();

        final TelephonyManager man = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        PSLCallback callback = new PSLCallback() {

            @Override
            public void onPercentKnown(int percent) {
                if(out != null) {
                    out.addInt8(Keys.AppKeyGSMPercent, (byte) percent);
                } else {
                    Runtime.log(context, TAG, "Tried to add new phone state to null dict!", Logger.ERROR);
                }
            }

        };
        final SignalListener listener = new SignalListener(context, callback);
        man.listen(listener, PhoneStateListener.LISTEN_SIGNAL_STRENGTHS);

        new Thread(new Runnable() {

            @Override
            public void run() {
                // Delay unlistening
                try {
                    Thread.sleep(5000);
                    man.listen(listener, PhoneStateListener.LISTEN_NONE);
                    Runtime.log(context, TAG, "Stopped listening for signal strength updates", Logger.INFO);
                } catch(Exception e) {
                    Runtime.log(context, TAG, "Exception waiting to stop listening to signal changes", Logger.ERROR);
                    Runtime.logStackTrace(context, e);
                }
            }

        }).start();
    }

    private void addWifiName(PebbleDictionary out) {
        WifiManager wifiMgr = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiMgr.getConnectionInfo();
        String name = wifiInfo.getSSID();
        name = name.replace("\"", "");

        if(name.equals("0x")) {
            // On, but disconnected?
            name = "Disconnected";
        } else if(name.equals("<unknown ssid>")) {
            name = "Unknown";
        }

        out.addString(Keys.AppKeyWifiName, name);
        Runtime.log(getApplicationContext(), TAG, "Wifi network name: " + name, Logger.INFO);
    }

    /**
     * http://stackoverflow.com/questions/4595334/get-free-space-on-internal-memory
     */
    private void addFreeSpace(PebbleDictionary out) {
        StatFs statFs = new StatFs(Environment.getExternalStorageDirectory().getAbsolutePath());
        long free = statFs.getFreeBlocksLong() * (statFs.getBlockSizeLong());
        final float gigs =  free / 1073741824F;
        float temp = gigs;

        int major = (int)Math.floor(temp);
        temp -= major;
        temp *= 10.0F;
        int minor = (int)Math.round(temp) % 10;
        out.addInt8(Keys.AppKeyStorageFreeGBMajor, (byte) major);
        out.addInt8(Keys.AppKeyStorageFreeGBMinor, (byte) minor);

        long total = statFs.getBlockCountLong() * statFs.getBlockSizeLong();
        int percent = Math.round(((float)free / (float)total) * 100);
        percent = 100 - percent;
        out.addInt8(Keys.AppKeyStoragePercent, (byte)percent);
        Runtime.log(getApplicationContext(), TAG, "Free space: " + gigs + "GB. Major/minor: " + major + "/" + minor + " (" + percent + "% used)", Logger.INFO);
    }

    private void addAutoBrightnessState(PebbleDictionary out) {
        Context context = getApplicationContext();

        boolean autoEnabled = UserInterface.getAutoBrightnessEnabled(context);
        Runtime.log(context, TAG, "Auto brightness is " + (autoEnabled ? "ON" : "OFF"), Logger.INFO);
        out.addInt8(Keys.AppKeyToggleAutoBrightness, (byte) (autoEnabled ? Keys.ToggleStateBrightnessAutomatic : Keys.ToggleStateBrightnessManual));
    }

    private void addIsLollipop(PebbleDictionary out) {
        boolean isLollipop = Platform.isLollipopOrAbove();
        Runtime.log(getApplicationContext(), TAG, "Platform is " + (isLollipop ? "" : "NOT") + " 5.0+.", Logger.INFO);
        out.addInt8(Keys.AppKeyIsLollipop, (byte) (isLollipop ? 1 : 0));
    }

    private void addPhoneState(PebbleDictionary out) {
        Runtime.log(getApplicationContext(), TAG, "Adding Find Phone state OFF...", Logger.INFO);
        out.addInt8(Keys.AppKeyToggleFindPhone, (byte) 0);
    }

    private void addToggleOrder(PebbleDictionary out) {
        out.addString(Keys.AppKeyToggleOrderString, constructConfigurationString(getApplicationContext()));
    }

    private void addWifiApState(PebbleDictionary out) {
        Context context = getApplicationContext();

        boolean enabled = Radios.getWifiApEnabled(context);
        Runtime.log(context, TAG, "Wifi AP is " + (enabled ? "ON" : "OFF"), Logger.INFO);
        out.addInt8(Keys.AppKeyToggleWifiAP, (enabled ? (byte) 1 : (byte) 0));
    }

    private void addAutoSyncState(PebbleDictionary out) {
        boolean wasSyncing = ContentResolver.getMasterSyncAutomatically();
        Runtime.log(getApplicationContext(), TAG, "Autosync is " + (wasSyncing ? "ON" : "OFF"), Logger.INFO);
        out.addInt8(Keys.AppKeyToggleSync, (wasSyncing ? (byte) 1 : (byte) 0));    //Report the current
    }

    private void addRingerState(PebbleDictionary out) {
        Context context = getApplicationContext();

        //Get ringer state
        AudioManager aManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        int mode = aManager.getRingerMode();
        Runtime.log(context, TAG, "GOT RINGER " + mode, Logger.INFO);
        switch (mode) {
            case AudioManager.RINGER_MODE_NORMAL:
                out.addInt8(Keys.AppKeyToggleRinger, (byte) Keys.ToggleStateLoud);
                Runtime.log(context, TAG, "Volume: LOUD", Logger.INFO);
                break;
            case AudioManager.RINGER_MODE_VIBRATE:
                out.addInt8(Keys.AppKeyToggleRinger, (byte) Keys.ToggleStateVibrate);
                Runtime.log(context, TAG, "Volume: VIBRATE", Logger.INFO);
                break;
            case AudioManager.RINGER_MODE_SILENT:
                out.addInt8(Keys.AppKeyToggleRinger, (byte) Keys.ToggleStateSilent);
                Runtime.log(context, TAG, "Volume: SILENT", Logger.INFO);
                break;
        }
    }

    private void addBluetoothState(PebbleDictionary out) {
        boolean btState = Radios.getBluetoothState();
        Runtime.log(getApplicationContext(), TAG, "BT is : " + (btState ? "true" : "false"), Logger.INFO);
        out.addInt8(Keys.AppKeyToggleBluetooth, btState ? (byte) 1 : (byte) 0);
    }

    private void addMobileDataState(PebbleDictionary out) {
        boolean networkState = Radios.getMobileDataEnabled(this);
        Runtime.log(getApplicationContext(), TAG, "Data is : " + (networkState ? "true" : "false"), Logger.INFO);
        out.addInt8(Keys.AppKeyToggleData, networkState ? (byte) 1 : (byte) 0);
    }

    private void addWifiState(PebbleDictionary out) {
        boolean wifiState = Radios.getWiFiEnabled(this);
        Runtime.log(getApplicationContext(), TAG, "Wifi is connected: " + (wifiState ? "true" : "false"), Logger.INFO);
        out.addInt8(Keys.AppKeyToggleWifi, wifiState ? (byte) 1 : (byte) 0);
    }

    private void addOperatorName(PebbleDictionary out) {
        //Get operator name
        TelephonyManager tManager = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        String operatorName = tManager.getNetworkOperatorName();

        Runtime.log(getApplicationContext(), TAG, "Operator: " + operatorName, Logger.INFO);
        if (operatorName.length() > 0) {
            out.addString(Keys.AppKeyGSMName, operatorName);
        } else {
            //Unknown!
            out.addString(Keys.AppKeyGSMName, "Unknown");
        }
    }

    private void addBatteryLevel(PebbleDictionary out) {
        IntentFilter iFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Intent batteryStatus = registerReceiver(null, iFilter);

        int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
        int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
        float batteryPct = (float) level / (float) scale;

        Runtime.log(getApplicationContext(), TAG, "Battery level is " + batteryPct, Logger.INFO);
        out.addInt8(Keys.AppKeyBatteryPercent, (byte) Math.round(batteryPct * 100.0F));
    }

    private void addLockState(PebbleDictionary out) {
        Runtime.log(getApplicationContext(), TAG, "Adding lock state off...", Logger.INFO);
        out.addInt8(Keys.AppKeyToggleLockPhone, (byte) Keys.ToggleStateOff);
    }

    /**
     * Concat a string of array indices indicating toggle positioning choices
     */
    public static String constructConfigurationString(Context context) {
        String result = "";
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        for (int i = 0; i < Landing.spinnerIds.length; i++) {
            int nextValue = prefs.getInt(Keys.PREF_CONFIGURE_BASE + "" + i, -1);
            if (nextValue != -1) {
                result += "" + nextValue;
            }
        }

        Runtime.log(context, TAG, "Config is " + result, Logger.INFO);
        return result;
    }

    /******************************* Implements Service ********************************/

//    private Notification notifyBelowOreo() {
//        Intent notificationIntent = new Intent(this, Landing.class);
//        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
//        return new NotificationCompat.Builder(this)
//                .setSmallIcon(R.drawable.ic_launcher_notif)
//                .setContentTitle("Dashboard for Pebble")
//                .setContentText("Required notification for background toggling")
//                .setContentIntent(pendingIntent)
//                .build();
//    }

    @RequiresApi(android.os.Build.VERSION_CODES.O)
    private Notification notifyAboveOreo() {
        String channelId = Build.PACKAGE_NAME;
        String channelName = "Dashboard Background Service";
        NotificationChannel chan = new NotificationChannel(channelId, channelName, NotificationManager.IMPORTANCE_NONE);
        chan.setLightColor(Color.BLUE);
        chan.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE);
        NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        assert manager != null;
        manager.createNotificationChannel(chan);

        NotificationCompat.Builder notificationBuilder = new NotificationCompat.Builder(this, channelId);
        return notificationBuilder.setOngoing(true)
            .setSmallIcon(R.drawable.ic_launcher_notif)
            .setContentTitle("Required notification for background toggling")
            .setPriority(NotificationManager.IMPORTANCE_NONE)
            .setCategory(Notification.CATEGORY_SERVICE)
            .build();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Context context = getApplicationContext();

        // Background apps are cached inactive after API 28 (Oreo) :(
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            startForeground(FOREGROUND_ID, notifyAboveOreo());
        }

        Runtime.log(context, TAG, "onStartCommand", Logger.INFO);

        try {
            if (intent != null) {
                //Get JSON
                String jsonData = intent.getExtras().getString("json");  // Thanks, SteveP!
                if(!Build.RELEASE) {
                    Runtime.log(context, TAG, "Dict JSON: " + jsonData, Logger.DEBUG);
                }

                // Could be 'keep alive' type of launch...
                if (jsonData == null || jsonData.equals("")) {
                    return Service.START_STICKY;
                }

                try {
                    //Get dictionary and parse
                    PebbleDictionary dict = PebbleDictionary.fromJson(jsonData);
                    parse(dict);
                } catch (Exception e) {
                    config.Runtime.logStackTrace(context, e);
                }
            } else {
                Runtime.log(context, TAG, "Intent is null! The Service may have been restarted.", Logger.ERROR);
            }

            Runtime.log(context, TAG, "onStartCommand finished.", Logger.INFO);
        } catch (Exception e) {
            Runtime.log(context, TAG, "onStartCommand() threw exception: " + e.getLocalizedMessage(), Logger.INFO);
            Runtime.logStackTrace(context, e);
        }

        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

}
