package pebble;

import static com.getpebble.android.kit.Constants.MSG_DATA;
import static com.getpebble.android.kit.Constants.TRANSACTION_ID;

import java.util.UUID;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.getpebble.android.kit.PebbleKit;

public class PebbleReceiver extends BroadcastReceiver {
	//Configuration
	public final String TAG = PebbleReceiver.class.getName();
	
	//Other members
	private UUID appUUID = UUID.fromString("23b109a6-4bd1-4e3d-90b5-92f4155cdb86");

	@Override
	public void onReceive(Context context, Intent intent) {
		//Filter for datatoggle app
		UUID receivedUUID = (UUID) intent.getSerializableExtra("uuid");
		
		if(!appUUID.equals(receivedUUID)) {
			Log.d(TAG, "UUID does not match");
			return;
		} else {
			Log.d(TAG, "Receiver awake!");
			
			//Ack
			final int transactionId = intent.getIntExtra(TRANSACTION_ID, -1);
			PebbleKit.sendAckToPebble(context, transactionId);
			
			//Get dictionary
			final String jsonData = intent.getStringExtra(MSG_DATA);
            if (jsonData == null || jsonData.isEmpty()) {
                return;
            } else {
				//Launch service
            	Intent service = new Intent(context, PebbleService.class);
				service.putExtra("json", jsonData);
				context.startService(service);
            }
		}
	}
	
}
