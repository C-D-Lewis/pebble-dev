package pebble;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import com.getpebble.android.kit.util.PebbleDictionary;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

public class PebbleUtils {

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
	 * Alternative sideloading method
	 * Source: http://forums.getpebble.com/discussion/comment/103733/#Comment_103733 
	 * @param ctx
	 * @param assetFilename
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
	 * Let use choose install source for 2.0 use
	 * @param parent
	 * @param pbwFilename
	 * @param appstoreUid
	 */
	public static void installEither(final Context context, final String pbwFilename, final String appstoreUid) {
		AlertDialog.Builder dialog = new AlertDialog.Builder(context);
		dialog.setTitle("Choose Install Method");
		dialog.setMessage("Choose 'Pebble Appstore' if you are using 2.0, else choose 'Local .pbw'.");
		dialog.setPositiveButton("Local .pbw", new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				Toast.makeText(context, "This may require two attempts!", Toast.LENGTH_LONG).show();
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
	
	public static boolean hasInt(PebbleDictionary dict, int key) {
		return dict.getInteger(key) != null;
	}
	
	public static boolean hasString(PebbleDictionary dict, int key) {
		return dict.getString(key) != null;
	}
}
