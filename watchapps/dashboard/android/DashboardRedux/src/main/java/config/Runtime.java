package config;

import android.content.Context;

import cl_toolkit.Logger;
import cl_toolkit.Storage;

/**
 * Run-time data
 * @author Chris Lewis
 */
public class Runtime {
	
	private static Logger getLogger(Context context) {
		return new Logger(Storage.getAppStorage(context) + "/" + Build.DEBUG_LOG_NAME, Build.DEBUG_LOG_MAX_SIZE_BYTES);
	}

    public static void log(Context context, String TAG, String message, String level) {
        getLogger(context).log(TAG, message, level);
    }

    public static void logStackTrace(Context context, Exception e) {
        getLogger(context).logStackTrace(e);
    }

    public static void startNewSession(Context context) {
        getLogger(context).startNewSession();
    }

}