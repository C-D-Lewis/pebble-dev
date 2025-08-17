package util;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

import com.getpebble.android.kit.util.PebbleDictionary;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

public class PebbleUtils {

	/**
	 * Appstore install for 2.0 use (deep link reliability may vary between offical Pebble app releases...)
	 * @param appStoreUID	UID from dev-portal.getpebble.com
	 */
	public static void appStoreInstall(Context context, String appStoreUID) {
		Intent intent = new Intent(Intent.ACTION_VIEW);
		intent.setData(Uri.parse("pebble://appstore/" + appStoreUID));
		intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		context.startActivity(intent);
	}
	
    /**
     * Alternative sideloading method
     * Source: http://forums.getpebble.com/discussion/comment/103733/#Comment_103733
     * @param assetFilename	File name of the asset to sideload
     */
    public static void sideloadInstall(Context ctx, String assetFilename) {
        try {
            Intent intent = new Intent(Intent.ACTION_VIEW);
            File file = new File(ctx.getExternalFilesDir(null), assetFilename);
            InputStream is = ctx.getResources().getAssets().open(assetFilename);
            OutputStream os = new FileOutputStream(file);
            byte[] pbw = new byte[is.available()];
            is.read(pbw);
            os.write(pbw);
            is.close();
            os.close();
            intent.setDataAndType(Uri.fromFile(file), "application/pbw");
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            ctx.startActivity(intent);
        } catch (IOException e) {
            Toast.makeText(ctx, "App install failed: " + e.getLocalizedMessage(), Toast.LENGTH_LONG).show();
        }
    }
	
	/**
	 * Let user choose install source for 2.0 use
	 * @param pbwFilename	File name for .pbw option
	 * @param appstoreUid	UID from dev-portal.getpebble.com
	 */
	public static void installEither(final Context context, final String pbwFilename, final String appstoreUid) {
		AlertDialog.Builder dialog = new AlertDialog.Builder(context);
		dialog.setTitle("Install Watchapp");
		dialog.setMessage("Choose your preferred method of installation.");
		dialog.setPositiveButton("Local .pbw", new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				sideloadInstall(context, pbwFilename);
			}
			
		});
		dialog.setNeutralButton("Pebble Appstore", new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				try {
					appStoreInstall(context, appstoreUid);
				} catch (Exception e) {
					Toast.makeText(context, "Pebble Appstore not found! Try 'Local.pbw' instead.", Toast.LENGTH_SHORT).show();
				}
			}
			
		});
		dialog.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				dialog.dismiss();
			}
			
		});
		dialog.show();
	}
	
	/**
	 * See if the Pebble Dictionary has a TupletInteger
	 * @param dict	Dictionary that may contain the key
	 * @param key	The key itself to look for data.
	 * @return	true if a K-V pair is there
	 */
	public static boolean hasInt(PebbleDictionary dict, int key) {
		return dict.getInteger(key) != null;
	}
	
	/**
	 * Get a TupletInteger from the dictionary
	 * @param dict	Dictionary that contains the key
	 * @param key	The key to look for
	 * @return	the value associated with the key
	 */
	public static int getInt(PebbleDictionary dict, int key) {
		return dict.getInteger(key).intValue();
	}
	
	/**
	 * See if the Pebble Dictionary has a TupletCString
	 * @param dict	Dictionary that may contain the key
	 * @param key	The key itself to look for data.
	 * @return	true if a K-V pair is there
	 */
	public static boolean hasString(PebbleDictionary dict, int key) {
		return dict.getString(key) != null;
	}
	
	/**
	 * Check a Receiver's intent to see if it is for this app
	 * @param intent		Broadcast intent containing Pebble data
	 * @param thisAppUUID	UUID of this app
	 * @return				true if the two match
	 */
	public static boolean isThisApp(Intent intent, UUID thisAppUUID) {
		return intent.getSerializableExtra("uuid").equals(thisAppUUID);
	}
	
}
