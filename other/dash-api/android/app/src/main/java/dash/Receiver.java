package dash;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.getpebble.android.kit.Constants;
import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

import java.util.UUID;

import static com.getpebble.android.kit.Constants.TRANSACTION_ID;

/**
 * Formats (inbound):
 *   RequestTypeGetData
 *     DataTypeKey         - DataType
 *   RequestTypeSetFeature
 *     FeatureTypeKey      - FeatureType
 *     FeatureStateKey     - FeatureState
 *   RequestTypeGetFeature
 *     FeatureTypeKey      - FeatureType
 *
 * Formats (outbound):
 *   RequestTypeGetData
 *     DataTypeKey         - DataType
 *     DataValueKey        - DataValue
 *   RequestTypeSetFeature
 *     FeatureTypeKey      - FeatureType
 *     FeatureStateKey     - FeatureState
 *   RequestTypeGetFeature
 *     FeatureTypeKey      - FeatureType
 *     FeatureStateKey     - FeatureState
 */
public class Receiver extends BroadcastReceiver {

    private static final String TAG = Receiver.class.getName();

    @Override
    public void onReceive(Context context, Intent intent) {
        try {
            // Intent from Pebble Android app
            String json = intent.getStringExtra(Constants.MSG_DATA);
            PebbleDictionary dict = PebbleDictionary.fromJson(json);

            // Is this a job for Captain Dash API?
            if(dict.getInteger(Keys.AppKeyUsesDashAPI) == null) {
                return;
            }

            UUID uuid = (UUID)intent.getSerializableExtra(Constants.APP_UUID);
            Log.d(TAG, "Packet received from " + uuid.toString());

            // ACK
            int transactionId = intent.getIntExtra(TRANSACTION_ID, -1);
            PebbleKit.sendAckToPebble(context, transactionId);

            // Call Service
            Intent i = new Intent(context, Service.class);
            i.putExtra("json", json);
            i.putExtra("uuid", uuid.toString());
            context.startService(i);
        } catch(Exception e) {
            Log.e(TAG, "Error getting PebbleDictionary from JSON");
            e.printStackTrace();
        }
    }

}
