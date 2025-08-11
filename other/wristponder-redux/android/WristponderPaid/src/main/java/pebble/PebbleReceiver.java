package pebble;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.getpebble.android.kit.PebbleKit;

import java.util.UUID;

import config.Build;

import static com.getpebble.android.kit.Constants.APP_UUID;
import static com.getpebble.android.kit.Constants.MSG_DATA;
import static com.getpebble.android.kit.Constants.TRANSACTION_ID;

public class PebbleReceiver extends BroadcastReceiver {

	public final String TAG = PebbleReceiver.class.getName();
	
	@Override
	public void onReceive(Context context, Intent intent) {
		final UUID receivedUUID = (UUID) intent.getSerializableExtra(APP_UUID);
		if (!Build.WATCHAPP_UUID.equals(receivedUUID)) {
			Log.d(TAG, "UUID does not match");
			return;
		}

        // ACK
        PebbleKit.sendAckToPebble(context, intent.getIntExtra(TRANSACTION_ID, -1));

        // PebbleDictionary
        final String jsonData = intent.getStringExtra(MSG_DATA);
        if (jsonData == null || jsonData.isEmpty()) {
            Log.e(TAG, "JSON data was empty");
            return;
        }

        //Launch service
        Intent service = new Intent(context, PushService.class);
        service.putExtra("json", jsonData);
        context.startService(service);
	}
	
}
