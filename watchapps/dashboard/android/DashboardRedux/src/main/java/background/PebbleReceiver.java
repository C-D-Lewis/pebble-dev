package background;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

import com.getpebble.android.kit.PebbleKit;

import cl_toolkit.Logger;
import config.Build;
import config.Runtime;
import util.PebbleUtils;

import static com.getpebble.android.kit.Constants.MSG_DATA;
import static com.getpebble.android.kit.Constants.TRANSACTION_ID;

public class PebbleReceiver extends BroadcastReceiver {
	//Configuration
	public final static String TAG = PebbleReceiver.class.getName();

	private static PebbleReceiver receiver;

	@Override
	public void onReceive(Context context, Intent intent) {
		try {
			if(PebbleUtils.isThisApp(intent, Build.WATCH_APP_UUID)) {
				Runtime.startNewSession(context);
                Runtime.log(context, TAG, "Dashboard received Intent with matching UUID.", Logger.INFO);

				//Ack
				int transactionId = intent.getIntExtra(TRANSACTION_ID, -1);
				PebbleKit.sendAckToPebble(context, transactionId);

				//Get dictionary
				String jsonData = intent.getStringExtra(MSG_DATA);
				if (jsonData != null && !jsonData.isEmpty()) {
					//Launch service w/dictionary JSON
					launchHandlerService(context, jsonData);

				} else {
					Runtime.log(context, TAG, "jsonData was null or empty!", Logger.ERROR);
				}
			}
		} catch(Exception e) {
			Runtime.log(context, TAG, "Receiver threw exception: " + e.getLocalizedMessage(), Logger.ERROR);
			Runtime.logStackTrace(context, e);
		}
	}

	public static void launchHandlerService(Context context, String jsonData) {
		Intent service = new Intent(context, HandlerService.class);
		service.putExtra("json", jsonData);

		// Background apps are cached inactive after API 28 (Oreo) :(
		final int sdkInt = android.os.Build.VERSION.SDK_INT;
		if (sdkInt >= android.os.Build.VERSION_CODES.O) {
			Runtime.log(context, TAG, "startForegroundService on version " + sdkInt, Logger.INFO);
			context.startForegroundService(service);
		} else {
			Runtime.log(context, TAG, "startService on version " + sdkInt, Logger.INFO);
			context.startService(service);
		}
	}

	public static void registerReceiver(Context context) {
		unregisterReceiver(context);
		receiver = new PebbleReceiver();

		context.getApplicationContext().registerReceiver(receiver, new IntentFilter("com.getpebble.action.app.RECEIVE"));

		Runtime.log(context, TAG, "Registered new Pebble receiver", Logger.INFO);
	}

	public static void unregisterReceiver(Context context) {
		if(receiver == null) {
			Runtime.log(context, TAG, "Failed to unregister Pebble receiver - it was null", Logger.INFO);
			return;
		}

		try {
			context.getApplicationContext().unregisterReceiver(receiver);
			if(!Build.RELEASE) {
				Runtime.log(context, TAG, "Unregistered Pebble receiver", Logger.INFO);
			}
		} catch(Exception e) {
			Runtime.log(context, TAG, "Exception unregistering receiver - it may have already been unregistered", Logger.ERROR);
			Runtime.logStackTrace(context, e);
		}
	}
}
