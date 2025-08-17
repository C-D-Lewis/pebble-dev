package background;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import cl_toolkit.Logger;
import config.Build;
import config.Keys;
import config.Runtime;

public class App extends Application {

    private static final String TAG = App.class.getName();

    @Override
    public void onCreate() {
        super.onCreate();
        Context context = getApplicationContext();

        if (!Build.RELEASE) {
            Runtime.log(context, TAG, "Application onCreate", Logger.DEBUG);
        }

        // Start KeepAlive alarms here, since on first install we won't get PACKAGE_ADDED intent broadcast, and it won't be boot time
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        if(prefs.getBoolean(Keys.PREF_FIRST_RUN, true)) {
            Intent i = new Intent(context, BootPackageChangeReceiver.class);
            i.setAction(BootPackageChangeReceiver.ACTION_START_BATTERY_MONITORING);
            sendBroadcast(i);

            // DON'T CLEAR FIRST RUN FLAG, Landing.java will do this
        }

        PebbleReceiver.registerReceiver(context);
    }

    @Override
    public void onTerminate() {
        Context context = getApplicationContext();
        PebbleReceiver.unregisterReceiver(context);

        super.onTerminate();
    }
}
