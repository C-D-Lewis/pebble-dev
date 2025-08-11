package cl_toolkit;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.provider.Settings;

/**
 * Convenience methods for Activity GUI
 * @author Chris Lewis
 */
public class UserInterface {

    public static final int
            BRIGHTNESS_MODE_MANUAL = 0,
            BRIGHTNESS_MODE_AUTO = 1;

	/**
	 * Show a simple yes-no dialogue
	 * @param context			Activity context
	 * @param title				Title of the dialog
	 * @param message			Main message or question
	 * @param positiveLabel		Label for the positive button
	 * @param positiveListener	Listener for when the positive button is pressed
	 * @param negativeLabel		Label for the negative button
	 * @param negativeListener	Listener for when the negative button is pressed
	 */
	public static void showDialog(Context context, String title, String message, 
			   String positiveLabel, DialogInterface.OnClickListener positiveListener, 
			   String negativeLabel, DialogInterface.OnClickListener negativeListener) {
		AlertDialog.Builder builder = new AlertDialog.Builder(context);
		builder.setTitle(title);
		builder.setMessage(message);
		
		if(negativeListener != null)
		builder.setNegativeButton(negativeLabel, negativeListener);
		
		if(positiveListener != null)
		builder.setPositiveButton(positiveLabel, positiveListener);
		
		AlertDialog dialog = builder.create();
		dialog.show();
	}
	
	public static void uiDelayed(final Activity a, final Runnable r, final long delay) {
		new Thread(new Runnable() {

			@Override
			public void run() {
				try {
					Thread.sleep(delay);
					a.runOnUiThread(r);
				} catch(Exception e) {
					e.printStackTrace();
				}
			}

		}).start();
	}

    /**
     * Set the state of Auto/Adaptive brightness
     * @param context
     * @param state True for auto, false for manual. Last state appears to be restored for manual
     */
    public static void setAutoBrightnessEnabled(Context context, boolean state) {
        ContentResolver resolver = context.getContentResolver();
        if(state) {
            Settings.System.putInt(resolver, Settings.System.SCREEN_BRIGHTNESS_MODE, BRIGHTNESS_MODE_AUTO);
        } else {
            Settings.System.putInt(resolver, Settings.System.SCREEN_BRIGHTNESS_MODE, BRIGHTNESS_MODE_MANUAL);
        }
    }

    /**
     * Get whether Auto Brightness is enabled by the user.
     * @param context
     * @return  true if set to Auto/Adaptive. False otherwise.
     */
    public static boolean getAutoBrightnessEnabled(Context context) {
        ContentResolver resolver = context.getContentResolver();
        int mode;
        try {
            mode = Settings.System.getInt(resolver, Settings.System.SCREEN_BRIGHTNESS_MODE);
            return mode == BRIGHTNESS_MODE_AUTO ? true : false;
        } catch (Exception e) {
            System.err.println("Exception getting brightness! Returning false.");
            return false;
        }
    }

}
