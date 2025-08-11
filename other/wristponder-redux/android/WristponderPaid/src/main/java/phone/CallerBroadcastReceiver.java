package phone;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.getpebble.android.kit.PebbleKit;

import config.Build;
import config.Keys;

public class CallerBroadcastReceiver extends BroadcastReceiver {

	private static final String TAG = CallerBroadcastReceiver.class.getName();

	@Override
	public void onReceive(final Context context, Intent intent) {
		TelephonyManager telephony = (TelephonyManager)context.getSystemService(Context.TELEPHONY_SERVICE);

		PhoneStateListener pListener = new PhoneStateListener() {

			public void onCallStateChanged(int state, String incomingNumber) {
				switch(state) {
				case TelephonyManager.CALL_STATE_RINGING:
					Log.d(TAG, "Phone number incoming: " + incomingNumber);

					//Store the number
					SharedPreferences.Editor ed = PreferenceManager.getDefaultSharedPreferences(context).edit();
					ed.putString(Keys.PREF_CALLER_NAME, incomingNumber);
					ed.commit();
					
					//Auto start?
					SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
					
					boolean autoStart = prefs.getBoolean(Keys.PREF_AUTOSTART, false);
					if(autoStart) {
						PebbleKit.startAppOnPebble(context, Build.WATCHAPP_UUID);
					}
					break;
				}	
			};

		};

		telephony.listen(pListener, PhoneStateListener.LISTEN_CALL_STATE);
	}
}
