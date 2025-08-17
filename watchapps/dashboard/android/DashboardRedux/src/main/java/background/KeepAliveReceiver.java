package background;

import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.BatteryManager;
import android.preference.PreferenceManager;
import androidx.core.app.NotificationCompat;

import com.wordpress.ninedof.dashboard.R;

import cl_toolkit.Logger;
import cl_toolkit.Platform;
import config.Build;
import config.Keys;
import config.Runtime;

/**
 * AKA 'how hard can it be to notify when fully charged??'
 */
public class KeepAliveReceiver extends BroadcastReceiver {

    private static final String TAG = KeepAliveReceiver.class.getName();

    public static int NOTIFICATION_ID = 543897;

    private static BroadcastReceiver receiver;

    @Override
    public void onReceive(Context context, Intent intent) {
        if(!Build.RELEASE) {
            Runtime.log(context, TAG, "KeepAliveReceiver onReceive()", Logger.DEBUG);
        }

        if(receiver == null) {
            Runtime.log(context, TAG, "Battery receiver was null, replacing it. Next unregister will fail.", Logger.INFO);
        }
        registerReceiver(context.getApplicationContext());
    }

    public static void registerReceiver(Context context) {
        unregisterReceiver(context);
        receiver = new BroadcastReceiver() {

            @Override
            public void onReceive(final Context context, final Intent batteryStatus) {
                handleChange(context, batteryStatus);
            }

        };
        IntentFilter filter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        context.getApplicationContext().registerReceiver(receiver, filter);

        Runtime.log(context, TAG, "Registered new battery level receiver", Logger.INFO);
        if(!Build.RELEASE) {
            notify(context, "Receiver registered.");
        }
    }

    public static void unregisterReceiver(Context context) {
        if(receiver == null) {
            Runtime.log(context, TAG, "Failed to unregister battery receiver - it was null", Logger.INFO);
            return;
        }

        try {
            context.getApplicationContext().unregisterReceiver(receiver);
            if(!Build.RELEASE) {
                Runtime.log(context, TAG, "Unregistered battery receiver", Logger.INFO);
            }
        } catch(Exception e) {
            Runtime.log(context, TAG, "Exception unregistering receiver - it may have already been unregistered", Logger.ERROR);
            Runtime.logStackTrace(context, e);
        }
    }

    private static boolean isPluggedIn(Intent batteryState) {
        int plugged = batteryState.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
        return plugged == BatteryManager.BATTERY_PLUGGED_AC || plugged == BatteryManager.BATTERY_PLUGGED_USB;
    }

    private static void handleChange(Context c, Intent batteryStatus) {
        Context context = c.getApplicationContext();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);

        // Get new level
        int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
        int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
        int status = batteryStatus.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
        float batteryPct = (float) level / (float) scale;
        int currentLevel = Math.round(batteryPct * 100.0F);
        boolean isOnCharge = (status == BatteryManager.BATTERY_STATUS_CHARGING ||
                status == BatteryManager.BATTERY_STATUS_FULL);

        if(!Build.RELEASE) {
            Runtime.log(context, TAG, "Battery level now " + currentLevel + " (" + batteryPct + ")", Logger.DEBUG);
        }

        // Ratchet - Are we going up from 99%?
        int lastLevel = prefs.getInt(Keys.PREF_KEY_CHARGE_NOTIFICATION_LAST_VALUE, -1);
        if(lastLevel == 99 && currentLevel == 100) {
            // Does the user care?
            boolean monitorIsEnabled = prefs.getBoolean(Keys.PREF_KEY_CHARGE_NOTIFICATION_ENABLED, false);
            if(monitorIsEnabled && isOnCharge) {
                notify(context, "Your phone is now fully charged!");
                Runtime.log(context, TAG, "Fully charged, notifying.", Logger.INFO);
            } else {
                Runtime.log(context, TAG, "Fully charged, but notification is not enabled. Ignoring.", Logger.INFO);
            }
        }

        // Dismiss on unplug
        if(!isPluggedIn(batteryStatus)) {
            try {
                NotificationManager mNotifyMgr = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
                mNotifyMgr.cancel(NOTIFICATION_ID);
                Runtime.log(context, TAG, "Dismissed on unplug", Logger.INFO);
            } catch (Exception e) {
                Runtime.log(context, TAG, "Exception dismissing notification, was it not present?", Logger.INFO);
                Runtime.logStackTrace(context, e);
            }
        }

        // Remember last value
        SharedPreferences.Editor ed = prefs.edit();
        ed.putInt(Keys.PREF_KEY_CHARGE_NOTIFICATION_LAST_VALUE, currentLevel);
        ed.commit();
    }

    private static void notify(Context context, String content) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(context);
        if(Platform.isLollipopOrAbove()) {
            builder.setSmallIcon(R.drawable.ic_launcher_notif);
        } else {
            builder.setSmallIcon(R.drawable.ic_launcher);
        }
        builder.setContentTitle("Dashboard");
        builder.setContentText(content);
        builder.setColor(context.getResources().getColor(R.color.main_colour));

        NotificationManager mNotifyMgr = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        mNotifyMgr.notify(NOTIFICATION_ID, builder.build());
    }

}
