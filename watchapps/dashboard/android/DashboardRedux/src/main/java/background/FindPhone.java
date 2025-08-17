package background;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.IBinder;
import android.preference.PreferenceManager;
import androidx.annotation.RequiresApi;
import androidx.core.app.NotificationCompat;
import android.widget.Toast;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;
import com.wordpress.ninedof.dashboard.R;

import java.io.IOException;

import cl_toolkit.Logger;
import config.Build;
import config.Keys;
import config.Runtime;

public class FindPhone extends Service {
	
	private static final String TAG = FindPhone.class.getName();
	private static final int ID_FIND_PHONE = 347;
	
	private static MediaPlayer mediaPlayer; // Exist forever pls (or at least while the sound is on)
	private static int userVolume;
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
        Context context = getApplicationContext();

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		Runtime.log(context, TAG, "FindPhone onStartCommand", Logger.INFO);
		
		boolean isRunning = prefs.getBoolean(Keys.PREF_FIND_PHONE_RUNNING, false);
		if(isRunning) {
			Runtime.log(context, TAG, "Stopping find phone...", Logger.INFO);
			
			//Stop media
			final AudioManager audioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
			audioManager.setStreamVolume(AudioManager.STREAM_ALARM, userVolume, AudioManager.FLAG_PLAY_SOUND);

            if(mediaPlayer != null) {
                mediaPlayer.stop();
                mediaPlayer.reset();
            } else {
                Runtime.log(context, TAG, "mediaPlayer null when trying to stop Find Phone sound!", Logger.ERROR);
            }

			PebbleDictionary dict = new PebbleDictionary();
			dict.addInt8(Keys.AppKeyToggleFindPhone, (byte) Keys.ToggleStateOff);
			PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, dict);
			
			SharedPreferences.Editor ed = prefs.edit();
			ed.putBoolean(Keys.PREF_FIND_PHONE_RUNNING, false);
			ed.commit();

            // Stop service
            stopSelf();
		} else {
			Runtime.log(context, TAG, "Starting find phone...", Logger.INFO);
            String message = "Playing Find Phone sound. Use Find Phone toggle again to cancel.";
            String channelId = "find_phone_notification";

            if (android.os.Build.VERSION.SDK_INT > android.os.Build.VERSION_CODES.O) {
                createNotificationChannel(channelId, "Find My Phone");
            } else {
				// Start
				NotificationCompat.Builder builder = new NotificationCompat.Builder(context, channelId);
				builder.setSmallIcon(R.drawable.ic_launcher_notif);
				builder.setContentTitle("Dashboard");
				builder.setStyle(new NotificationCompat.BigTextStyle().bigText(message));
				builder.setColor(getResources().getColor(R.color.main_colour));
				startForeground(ID_FIND_PHONE, builder.build());
			}

            // Choose user's choice or default
            String uriString = prefs.getString(Keys.PREF_KEY_FIND_PHONE_FILE, Keys.PREF_VALUE_FIND_PHONE_DEFAULT);
            Uri uri;
            if(uriString.equals(Keys.PREF_VALUE_FIND_PHONE_DEFAULT)) {
                uri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
                Runtime.log(context, TAG, "Used Notification tone.", Logger.INFO);
                Toast.makeText(context, "No file chosen, or was unavailable. Used Notification sound.", Toast.LENGTH_SHORT).show();
            } else {
                uri = Uri.parse(uriString);
                Runtime.log(context, TAG, "Using loaded Uri: " + uriString, Logger.INFO);
            }
			
			playSound(uri);
		}
		
		Runtime.log(context, TAG, "FindPhone onStartCommand finished", Logger.INFO);
		
		// Finally
		return super.onStartCommand(intent, flags, startId);
	}
	
	private void playSound(Uri uri) {
        Context context = getApplicationContext();
		final AudioManager audioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);

        if(mediaPlayer != null) {
            // Stop current playback
            mediaPlayer.stop();
            mediaPlayer.reset();

            // Reset volume
            audioManager.setStreamVolume(AudioManager.STREAM_ALARM, userVolume, AudioManager.FLAG_PLAY_SOUND);
        } else {
            // Create new one
            mediaPlayer = new MediaPlayer();
        }

        try {
            if(!mediaPlayer.isPlaying()) {
                // Get current volume
                userVolume = audioManager.getStreamVolume(AudioManager.STREAM_ALARM);

                // Boost volume
                audioManager.setStreamVolume(AudioManager.STREAM_ALARM, audioManager.getStreamMaxVolume(AudioManager.STREAM_ALARM), AudioManager.FLAG_PLAY_SOUND);

                mediaPlayer.setDataSource(context, uri);
                mediaPlayer.setAudioStreamType(AudioManager.STREAM_ALARM);
                mediaPlayer.setLooping(true);
                mediaPlayer.prepare();
                mediaPlayer.start();
                Runtime.log(context, TAG, "Sound started.", Logger.INFO);

				PebbleDictionary out = new PebbleDictionary();
				out.addInt8(Keys.AppKeyToggleFindPhone, (byte) Keys.ToggleStateOn);
				PebbleKit.sendDataToPebble(context, Build.WATCH_APP_UUID, out);
				
				SharedPreferences.Editor ed = PreferenceManager.getDefaultSharedPreferences(context).edit();
				ed.putBoolean(Keys.PREF_FIND_PHONE_RUNNING, true);
				ed.commit();
				
				Runtime.log(context, TAG, "Sound started.", Logger.INFO);
			}
		} catch (IOException e) {
            Toast.makeText(context, "Your Find Phone sound was unavailable. Resetting to default...", Toast.LENGTH_LONG).show();
            SharedPreferences.Editor ed = PreferenceManager.getDefaultSharedPreferences(context).edit();
            ed.putString(Keys.PREF_KEY_FIND_PHONE_FILE, Keys.PREF_VALUE_FIND_PHONE_DEFAULT);
            ed.commit();
		}
		audioManager.setStreamVolume(AudioManager.STREAM_ALARM, audioManager.getStreamMaxVolume(AudioManager.STREAM_ALARM), AudioManager.FLAG_PLAY_SOUND);
	}

	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}

	@RequiresApi(android.os.Build.VERSION_CODES.O)
	private String createNotificationChannel(String channelId, String channelName){
		NotificationChannel channel = new NotificationChannel(channelId, channelName, NotificationManager.IMPORTANCE_NONE);
		channel.setLightColor(getResources().getColor(R.color.main_colour));
		channel.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE);
		NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		manager.createNotificationChannel(channel);

		return channelId;
	}
}
