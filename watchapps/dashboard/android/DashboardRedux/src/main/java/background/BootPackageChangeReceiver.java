package background;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import cl_toolkit.Logger;
import config.Build;
import config.Keys;
import config.Runtime;

/**
 * I really love Android APIs sometimes...
 * http://stackoverflow.com/questions/11277302/i-cant-receive-broadcast-on-battery-state-change/11277524#11277524
 */
public class BootPackageChangeReceiver extends BroadcastReceiver {

    private static final String TAG = BootPackageChangeReceiver.class.getName();

    public static final String ACTION_START_BATTERY_MONITORING = "com.wordpress.ninedof.dashboard.action.START_BATTERY_MONITORING";

    private static final int
        MINS = 30,
        REQUEST_CODE = 1;

    private boolean thisApp(Intent intent) {
        return intent.getData().getEncodedSchemeSpecificPart().equals(Build.PACKAGE_NAME);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        // Do our best to be alive for future background usage...
        PebbleReceiver.launchHandlerService(context, "");

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        if(!prefs.getBoolean(Keys.PREF_KEY_CHARGE_NOTIFICATION_ENABLED, false)) {
            return;
        }

        String action = intent.getAction();
        Runtime.log(context, TAG, "Starting battery monitor due to...", Logger.INFO);  // TODO sometimes this is never followed up

        // Boot completed
        if(action.equals(Intent.ACTION_BOOT_COMPLETED)) {
            Runtime.log(context, TAG, "Boot completed", Logger.INFO);
        }

        // Launched by me
        else if(action.equals(ACTION_START_BATTERY_MONITORING)) {
            Runtime.log(context, TAG, "Requested by Dashboard", Logger.INFO);
        }

        // App updated or installed
        else if(action.equals(Intent.ACTION_PACKAGE_REPLACED)
             || action.equals(Intent.ACTION_PACKAGE_ADDED)) {
            if(!thisApp(intent)) {
                return;
            }
            Runtime.log(context, TAG, "Dashboard package added or replaced", Logger.INFO);
        }

        else {
            Runtime.log(context, TAG, "Some other reason: " + action, Logger.INFO);
        }

        startAlarms(context);
    }

    private static PendingIntent getIdenticalPendingIntent(Context context) {
        Intent i = new Intent(context, KeepAliveReceiver.class);
        return PendingIntent.getBroadcast(context, REQUEST_CODE, i, 0);
    }

    public static void startAlarms(Context context) {
        clearAlarms(context);

        AlarmManager alarmManager = (AlarmManager)context.getSystemService(Context.ALARM_SERVICE);

        PendingIntent pendingIntent = getIdenticalPendingIntent(context);
        alarmManager.setRepeating(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + 1000, 1000 * 60 * MINS, pendingIntent); // Millisec * Second * Minute

        Runtime.log(context, TAG, "AM start", Logger.DEBUG);
    }

    public static void clearAlarms(Context context) {
        AlarmManager alarmManager = (AlarmManager)context.getSystemService(Context.ALARM_SERVICE);
        PendingIntent pendingIntent = getIdenticalPendingIntent(context);
        alarmManager.cancel(pendingIntent); // Works because the pendingIntent is identical to that which was registered

        Runtime.log(context, TAG, "AM cancel", Logger.INFO);
    }

}
