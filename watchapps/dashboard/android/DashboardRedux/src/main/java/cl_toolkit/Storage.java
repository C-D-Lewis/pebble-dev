package cl_toolkit;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.InputStream;

/**
 * Common Android storage related helpers
 * @author Chris Lewis
 */
public class Storage {
	
	private static final String TAG = Storage.class.getName();
	
	/**
	 * Get user storage directory on all devices
	 * @return	String to the directory
	 */
	public static String getStorage() {
		return Environment.getExternalStorageDirectory().getAbsolutePath();
	}
	
	/**
	 * Get the provite (data/data) directory for storage
	 * @param context	Context object for the current application
	 * @return			String in that directory
	 */
	public static String getPrivateStorage(Context context) {
		return context.getFilesDir().getAbsolutePath();
	}

	/**
     * Get a directory in storage/Android/data/[app]/files/*
     * @param context       Context object for the current application
     * @return                      String path to the location, or getExternalStorageDirectory if context or result is null
     */
   public static String getAppStorage(Context context) {
       String result = context.getExternalFilesDir(null).getAbsolutePath();

       if(result != null) {
           return result;
       } else {
           Log.e(TAG, "getAppStorage() returned null!");
           return Environment.getExternalStorageDirectory().getAbsolutePath();
       }
   }
	
	/**
	 * Convenience method to create a new foler
	 * @param pathToFolder	Path to the new folder
	 */
	public static void createFolder(String pathToFolder) {
		File test = new File(pathToFolder);
		if(!test.canRead() || !test.canWrite()) {
			test.mkdirs();
		}
	}
	
	/**
	 * Get an input stream for a file in the assets folder
	 * @param context	Application context
	 * @param assetName	Name of the asset file
	 * @return			Stream of that file. Remember to .close()!
	 */
	public static InputStream getAssetStream(Context context, String assetName) {
		try {
			return context.getAssets().open(assetName);
		} catch(Exception e) {
			e.printStackTrace();
			return null;
		}
	}
	
}
