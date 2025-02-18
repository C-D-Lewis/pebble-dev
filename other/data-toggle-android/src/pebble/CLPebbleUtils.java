package pebble;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

public class CLPebbleUtils {

	/**
	 * Appstore install for 2.0 use
	 * @param context
	 * @param appStoreUID
	 */
	public static void appStoreInstall(Context context, String appStoreUID) {
		Intent intent = new Intent(Intent.ACTION_VIEW);
		intent.setData(Uri.parse("pebble://appstore/" + appStoreUID));
		intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		context.startActivity(intent);
	}
	
	/**
	 * Sideload install from file in 'assets' folder for 1.X and 2.0 use
	 * @param context
	 * @param assetFilename
	 */
	public static void sideloadInstall(Context context, String assetFilename) {
		try {
			InputStream input = context.getAssets().open(assetFilename);
			File appFile = new File(context.getFilesDir(), assetFilename);
			appFile.setReadable(true, false);
			OutputStream output = new FileOutputStream(appFile);
			try {
				byte[] buffer = new byte[1024];
				int read;
				while ((read = input.read(buffer)) != -1)
					output.write(buffer, 0, read);
				output.flush();
			} finally {
				output.close();
			}
			input.close();
			Intent intent = new Intent(Intent.ACTION_VIEW);
//			intent.setComponent(new ComponentName("com.getpebble.android", "com.getpebble.android.ui.UpdateActivity"));
//			intent.setData(Uri.fromFile(appFile));
			intent.setDataAndType(Uri.fromFile(appFile), "application/octet-stream");
			context.startActivity(intent);
		} catch (Exception e) {
			Toast.makeText(context, "App install failed: " + e.getLocalizedMessage(), Toast.LENGTH_LONG).show();
		}
	}
	
	/**
	 * Let use choose install source for 2.0 use
	 * @param parent
	 * @param pbwFilename
	 * @param appstoreUid
	 */
	public static void installWatchapp(final Activity parent, final String pbwFilename, final String appstoreUid) {
		AlertDialog.Builder dialog = new AlertDialog.Builder(parent);
		dialog.setTitle("Choose Install Method");
		dialog.setMessage("Choose 'Pebble Appstore' if you are using 2.0 BETA 9+, else choose 'Local .pbw'.");
		dialog.setPositiveButton("Local .pbw", new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				Toast.makeText(parent, "This may require two attempts!", Toast.LENGTH_LONG).show();
				sideloadInstall(parent, pbwFilename);
			}
			
		});
		dialog.setNeutralButton("Pebble Appstore", new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				appStoreInstall(parent, appstoreUid);
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
}
