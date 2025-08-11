package pebble;

import java.util.UUID;

import android.content.Context;
import android.util.Log;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

/**
 * Check watchapp version against a known compatible version on Android
 * 1. Call version_check(char *version, void(*callback)(bool)); from watchapp
 * 2. On reception of that dictionary, call check() below to issue response and trigger callback
 * 
 * @author Chris Lewis
 */
public class VersionCheck {
	
	private static final String TAG = VersionCheck.class.getName();
	
	public static final int
		KEY_VERSION_CHECK_VERSION = 58283,
		KEY_VERSION_CHECK_SUCCESS = 58278,
		KEY_VERSION_CHECK_FAILURE = 58232;
	
	/**
	 * To be called from a PebbleDataReceiver implementation
	 * @param context		Context of the Android app
	 * @param uuid			UUID of the watchapp
	 * @param dict			Incoming dictionary
	 * @param localVersion	Compatible version with this Android app version
	 */
	public static void check(Context context, UUID uuid, PebbleDictionary dict, String localVersion) {
		if(dict.getString(KEY_VERSION_CHECK_VERSION) != null) {
			String remoteVersion = dict.getString(KEY_VERSION_CHECK_VERSION).toString();
			
			//Let the watch know
			PebbleDictionary response = new PebbleDictionary();
			
			if(localVersion.equals(remoteVersion)) {
				response.addInt32(KEY_VERSION_CHECK_SUCCESS, 0);
				Log.d(TAG, "Version check passed!");
			} else {
				response.addInt32(KEY_VERSION_CHECK_FAILURE, 0);
				Log.d(TAG, "Version check failed!");
			}
			
			PebbleKit.sendDataToPebble(context, uuid, response);
		}
	}

}
