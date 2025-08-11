package dash;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;
import com.wordpress.ninedof.dashapi.R;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.UUID;

import activity.Landing;
import data.Meta;
import data.PermissionManager;


public class Service extends android.app.Service {

    private static final String TAG = Service.class.getName();
    private static final boolean DEBUG = true;

    private void parse(PebbleDictionary dict, final UUID uuid) {
        Context context = getApplicationContext();
        final PebbleDictionary out = new PebbleDictionary();

        // Check version first
        String versionRemote = dict.getString(Keys.AppKeyLibraryVersion);
        if(Meta.isRemoteCompatible(versionRemote)) {
            out.addInt32(Keys.RequestTypeError, 0);
            out.addInt32(Keys.AppKeyErrorCode, Keys.ErrorCodeSuccess);
        } else {
            out.addInt32(Keys.RequestTypeError, 0);
            out.addInt32(Keys.AppKeyErrorCode, Keys.ErrorCodeWrongVersion);
            PebbleKit.sendDataToPebble(getApplicationContext(), uuid, out);
            return;
        }

        // Get data request
        if(dict.getInteger(Keys.RequestTypeGetData) != null) {
            out.addInt32(Keys.RequestTypeGetData, 0);

            int type = dict.getInteger(Keys.AppKeyDataType).intValue();
            out.addInt32(Keys.AppKeyDataType, type);
            APIHandler.handleGetData(context, type, out);
        }

        // Set feature request
        if(dict.getInteger(Keys.RequestTypeSetFeature) != null) {
            // Permitted?
            if(!PermissionManager.isPermitted(context, uuid)) {
                out.addInt32(Keys.RequestTypeError, 0);
                out.addInt32(Keys.AppKeyErrorCode, Keys.ErrorCodeNoPermissions);

                notifyNoPermission(uuid);
            } else {
                out.addInt32(Keys.RequestTypeSetFeature, 0);

                int type = dict.getInteger(Keys.AppKeyFeatureType).intValue();
                out.addInt32(Keys.AppKeyFeatureType, type);
                int state = dict.getInteger(Keys.AppKeyFeatureState).intValue();
                out.addInt32(Keys.AppKeyFeatureState, state);

                APIHandler.handleSetFeature(context, type, state, out);
            }
        }

        // Get feature request
        if(dict.getInteger(Keys.RequestTypeGetFeature) != null) {
            out.addInt32(Keys.RequestTypeGetFeature, 0);

            int type = dict.getInteger(Keys.AppKeyFeatureType).intValue();
            out.addInt32(Keys.AppKeyFeatureType, type);
            APIHandler.handleGetFeature(context, type, out);
        }

        // Is available request
        if(dict.getInteger(Keys.RequestTypeIsAvailable) != null) {
            // Handled by AppKeyLibraryVersion in header
        }

        // Wait at least xms for GSM signal strength listener to resolve
        // Does not appear to happen immediately
        new Thread(new Runnable() {

            @Override
            public void run() {
                PebbleKit.sendDataToPebble(getApplicationContext(), uuid, out);
                Log.d(TAG, "Sent response to " + uuid.toString());
                if(DEBUG) {
                    Log.d(TAG, "JSON out: " + out.toJsonString());
                }
            }

        }).start();

    }

    private void checkPermissionEntry(PebbleDictionary dict, UUID uuid) {
        Context context = getApplicationContext();

        // Add for Landing ArrayList
        String name = dict.getString(Keys.AppKeyAppName);
        if(PermissionManager.getName(context, uuid) == null) {
            // First time we've seen this app?
            PermissionManager.setPermitted(context, uuid, false);   // This also modifies the list

        }

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        PermissionManager.setName(context, uuid, name);
    }

    private void notifyNoPermission(UUID uuid) {
        Context context = getApplicationContext();
        String name = PermissionManager.getName(context, uuid);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(context);
        builder.setSmallIcon(R.drawable.ic_launcher_notif);
        builder.setContentTitle("Dash API");
        builder.setContentText(name + " is requesting permission to write to an Android API. Tap to grant permission.");
        builder.setColor(Color.DKGRAY);
        builder.setAutoCancel(true);

        PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
                new Intent(this, Landing.class), PendingIntent.FLAG_UPDATE_CURRENT);
        builder.setContentIntent(contentIntent);

        int id = 1;
        NotificationManager mNotifyMgr = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        mNotifyMgr.notify(id, builder.build());
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null) {
            Log.e(TAG, "Intent was null");
            return super.onStartCommand(intent, flags, startId);
        }

        try {
            Log.d(TAG, "onStartCommand()");

            //Get dictionary and parse
            Bundle extras = intent.getExtras();
            String jsonData = extras.getString("json");
            if(DEBUG) {
                Log.d(TAG, "JSON in: " + jsonData);
                analyse(jsonData);
            }

            String uuidString = extras.getString("uuid");
            UUID uuid = UUID.fromString(uuidString);
            PebbleDictionary dict = PebbleDictionary.fromJson(jsonData);
            checkPermissionEntry(dict, uuid);
            parse(dict, uuid);
        } catch (Exception e) {
            Log.e(TAG, "onStartCommand() threw exception: " + e.getLocalizedMessage());
            e.printStackTrace();
        }

        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

    private void analyse(String jsonData) {
        try {
            JSONArray dictArr = new JSONArray(jsonData);
            for(int i = 0; i < dictArr.length(); i++) {
                JSONObject obj = dictArr.getJSONObject(i);
                int key = obj.getInt("key");
                String keyStr;
                keyStr = Keys.ReqKeyDataFTypeToString(key);
                if(keyStr == null) {
                    // Try second method
                    keyStr = Keys.ErrToString(key);
                }

                String value = obj.getString("value");
                String valueStr;
                int v = 0;
                try {
                    v = Integer.parseInt(value);
                } catch(Exception e) {
                    // Was a string
                    v = -1;
                }
                if(v >= 0) {
                    // Was integer
                    valueStr = Keys.ReqKeyDataFTypeToString(v);
                } else {
                    valueStr = value;
                }

                Log.d(TAG, "analyse: k=" + keyStr + ", v=" + valueStr);
            }
        } catch(Exception e) {
            Log.e(TAG, "NOT JSON");
        }
    }

}
