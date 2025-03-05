package config;

import android.content.Context;
import cl_toolkit.Logger;
import cl_toolkit.Storage;

public class Runtime {
	
	private static Logger getLogger(Context context) {
		return new Logger(Storage.getAppStorage(context) + "/" + Build.DEBUG_LOG_NAME, Build.MAX_LOG_BYTES);
	}

    public static void log(Context context, String TAG, String message, String level) {
        getLogger(context).log(TAG, message, level);
    }

    public static void startNewSession(Context context) {
        getLogger(context).startNewSession();
    }

    public static void logStackTrace(Context context, Exception e) {
        getLogger(context).logStackTrace(e);
    }

}
