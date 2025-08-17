package activity;

import android.Manifest;
import android.app.ActionBar;
import android.app.Activity;
import android.app.NotificationManager;
import android.app.admin.DevicePolicyManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.drawable.ColorDrawable;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;

import androidx.cardview.widget.CardView;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.FragmentActivity;

import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationUtils;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.wordpress.ninedof.dashboard.R;

import org.json.JSONObject;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import adapter.ToggleSelector;
import background.BootPackageChangeReceiver;
import background.DeviceAdmin;
import background.HandlerService;
import background.KeepAliveReceiver;
import background.PebbleReceiver;
import cl_toolkit.Logger;
import cl_toolkit.UserInterface;
import cl_toolkit.Web;
import config.Build;
import config.Keys;
import config.Runtime;
import no_commit.NoCommit;

public class Landing extends FragmentActivity {

    private static final String TAG = Landing.class.getName();
    public static final int
        ADMIN_RESULT = 38,           // Result of Device Admin dialog
        RESULT_RINGTONE_URI = 3265;  // Result from ringtone picker

    private static int REQUEST_LOCATION = 1;

    private MediaPlayer mediaPlayer;

	private Handler handler = new Handler();
    private CardView layoutCard, optionsCard, installCard, newsCard;
    private TextView newsBody;
    private Switch adminSwitch;
    private ProgressBar newsProgressBar;

	public static final int[] spinnerIds = {
		R.id.s11,
		R.id.s12,
		R.id.s13,
		R.id.s21,
		R.id.s22,
		R.id.s23,
		R.id.s31,
		R.id.s32,
        R.id.s33
	};
	private ArrayList<Spinner> spinners = new ArrayList<Spinner>();

    private int userVolume;

	private void checkChangeLogs() {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		if (prefs.getBoolean(Keys.PREF_CHANGELOG_PREFIX + "" + Build.VERSION, true)) {

			/**
			 * RELEASE CHECKLIST!!
			 * - Android app version numbers MANIFEST & Build.java
			 * - Watchapp version numbers package.json & config.h
			 * - apps.json.dashboard text
			 * - Check asset .pbw is available and up to date
             * - Check no_commit.NoCommit is available
			 */
	
			// Show changelog
			UserInterface.showDialog(this, "What's New" + " (v" + Build.VERSION + ")\n", ""
                    + "- Update to Android Platform SDK 29.\n"
                    + "- Slightly refine the required notification text for smaller devices.\n"
					, "Done",
					new DialogInterface.OnClickListener() {
	
						@Override
						public void onClick(DialogInterface dialog, int which) {
							dialog.dismiss();
						}
	
					}, null, null);
	
			// Turn off flag
			SharedPreferences.Editor ed = prefs.edit();
			ed.putBoolean(Keys.PREF_CHANGELOG_PREFIX + "" + Build.VERSION, false);
			ed.commit();
		}
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_landing);

        final Context context = getApplicationContext();
        final SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);

        // Debug label
        TextView debugLabel = (TextView)findViewById(R.id.debug_label);
        if(Build.RELEASE) {
            debugLabel.setVisibility(View.GONE);
        }

        // Setup ActionBar
		final ActionBar aBar = getActionBar();
		aBar.setTitle("Dashboard");
		aBar.setBackgroundDrawable(new ColorDrawable(getResources().getColor(R.color.main_colour)));

		// Sweep, sweep
		checkChangeLogs();
		
        // Get UI members
		layoutCard = (CardView)findViewById(R.id.layout_card);
        installCard = (CardView)findViewById(R.id.install_card);
        newsCard = (CardView)findViewById(R.id.news_card);
        optionsCard = (CardView)findViewById(R.id.options_card);

        // Setup install card
		handler.postDelayed(new Runnable() {
			
			@Override
			public void run() {
				showCard(installCard);
			}
			
		}, 200);
        Button installButtonAppstore = (Button) findViewById(R.id.button_install_appstore);
        installButtonAppstore.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // Modern android doesn't allow file:// anymore :(
//				PebbleUtils.sideloadInstall(context, Build.WATCH_APP_PBW_NAME);
//				Toast.makeText(context, "If the Pebble Android app does not ask for confirmation, close it and try this again.", Toast.LENGTH_LONG).show();

                // Show appstore instead - love live Rebble!
                final Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setData(Uri.parse("https://apps.rebble.io/en_US/application/53ec8d840c3036447e000109?query=dashboard&section=watchapps&dev_settings=true"));
                startActivity(intent);
            }

        });
        Button installButtonGithub = (Button) findViewById(R.id.button_install_github);
        installButtonGithub.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                final Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setData(Uri.parse("https://github.com/C-D-Lewis/dashboard/tree/master/assets/releases"));
                startActivity(intent);
            }

        });

        // Device Admin
        adminSwitch = (Switch)findViewById(R.id.admin_switch);
        adminSwitch.setChecked(prefs.getBoolean(Keys.PREF_KEY_ADMIN, false));
        adminSwitch.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View view) {
                final boolean newState = adminSwitch.isChecked();
                adminSwitch.setChecked(false);
                if (newState && !prefs.getBoolean(Keys.PREF_KEY_ADMIN, false)) {
                    // Is not, request admin
                    requestDeviceAdmin();
                } else {
                    //Is, revoke!
                    DevicePolicyManager man = (DevicePolicyManager) getSystemService(Context.DEVICE_POLICY_SERVICE);
                    man.removeActiveAdmin(new ComponentName(context, DeviceAdmin.class));
                    Toast.makeText(context, "Device Administrator status removed", Toast.LENGTH_SHORT).show();

                    SharedPreferences.Editor ed = PreferenceManager.getDefaultSharedPreferences(context).edit();
                    ed.putBoolean(Keys.PREF_KEY_ADMIN, false);
                    ed.commit();
                }
            }
        });

		// Animate news card
        newsProgressBar = (ProgressBar)findViewById(R.id.news_progress_bar);
        newsBody = (TextView)findViewById(R.id.news_body);

		// Load spinners - but only if not already loaded
		if(spinners.size() >= Build.NUM_TOGGLES) {
			spinners.clear();
		}

		final SharedPreferences.Editor ed = prefs.edit();
        ToggleSelector adapter = new ToggleSelector(context);
		for(int i = 0; i < Build.NUM_TOGGLES; i++) {
			//Get View
			final Spinner s = (Spinner)findViewById(spinnerIds[i]);
            s.setAdapter(adapter);

			//Save whenever any are edited
			s.setOnItemSelectedListener(new OnItemSelectedListener() {
				
				public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
					saveConfig(s, position);
				}

				@Override
				public void onNothingSelected(AdapterView<?> parent) { }
				
			});

			//Set position according to preference
			int choice = prefs.getInt(Keys.PREF_CONFIGURE_BASE + "" + i, -1);
            if(choice >= Build.NUM_TOGGLES) {
                Runtime.log(context, TAG, "Bad config value: " + choice + "/" + Build.NUM_TOGGLES + ", resetting...", Logger.ERROR);
                resetConfiguration(ed);
                ed.commit();

                // Get new correct value
                SharedPreferences temp = PreferenceManager.getDefaultSharedPreferences(context);
                choice = temp.getInt(Keys.PREF_CONFIGURE_BASE + "" + i, -1);
            }
			if(choice != -1) {
				//There is a pref
				s.setSelection(choice, true);
			} else {
				//There is no pref - first time?
				s.setSelection(i, true);
				ed.putInt(Keys.PREF_CONFIGURE_BASE + "" + i, i);
			}
			
			spinners.add(s);
		}
		ed.commit();

		// Populate Quick Launch spinner
		Spinner quickSpinner = (Spinner)findViewById(R.id.quick_launch_spinner);
        quickSpinner.setAdapter(adapter);
        if(!prefs.contains(Keys.PREF_KEY_QUICK_LAUNCH_TYPE) && !prefs.contains(Keys.PREF_KEY_QUICK_LAUNCH_ENABLED)) {
            // Set default
            SharedPreferences.Editor edi = prefs.edit();
            edi.putInt(Keys.PREF_KEY_QUICK_LAUNCH_TYPE, Keys.ToggleTypeWifi);
            edi.putBoolean(Keys.PREF_KEY_QUICK_LAUNCH_ENABLED, false);
            edi.commit();
        }
        Switch quickEnabled = (Switch)findViewById(R.id.quick_launch_switch);
        quickEnabled.setChecked(prefs.getBoolean(Keys.PREF_KEY_QUICK_LAUNCH_ENABLED, false));
        quickEnabled.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                SharedPreferences.Editor ed = prefs.edit();
                ed.putBoolean(Keys.PREF_KEY_QUICK_LAUNCH_ENABLED, isChecked);
                ed.commit();
                Runtime.log(context, TAG, "Quick launch enabled? " + isChecked, Logger.INFO);
            }

        });
        int index = prefs.getInt(Keys.PREF_KEY_QUICK_LAUNCH_TYPE, Keys.ToggleTypeWifi);
        quickSpinner.setSelection(index, true);
        quickSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                SharedPreferences.Editor ed = prefs.edit();
                ed.putInt(Keys.PREF_KEY_QUICK_LAUNCH_TYPE, position);
                ed.commit();
                Runtime.log(context, TAG, "Saved quick launch index: " + position, Logger.INFO);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) { }

        });

        // Full charge notification UI
        Switch chargeNotifEnabled = (Switch)findViewById(R.id.charge_notif_switch);
        boolean isChecked = prefs.getBoolean(Keys.PREF_KEY_CHARGE_NOTIFICATION_ENABLED, false);
        chargeNotifEnabled.setChecked(isChecked);
        Runtime.log(context, TAG, "Charge notification enabled? " + isChecked, Logger.INFO);
        chargeNotifEnabled.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                SharedPreferences.Editor ed = prefs.edit();
                ed.putBoolean(Keys.PREF_KEY_CHARGE_NOTIFICATION_ENABLED, isChecked);
                ed.commit();
                Runtime.log(context, TAG, "Charge notification set to " + isChecked, Logger.INFO);

                if(isChecked) {
                    // Bring forth the hacks!
                    BootPackageChangeReceiver.startAlarms(context);
                    KeepAliveReceiver.registerReceiver(context);
                } else {
                    BootPackageChangeReceiver.clearAlarms(context);
                    KeepAliveReceiver.unregisterReceiver(context);
                }
            }

        });

        // Find Phone buttons
        Button findPhoneTest = (Button)findViewById(R.id.find_phone_sound_test);
        findPhoneTest.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                testFindPhoneSound();
            }

        });
        Button findPhoneSet = (Button)findViewById(R.id.find_phone_sound_set);
        findPhoneSet.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                Intent intent = new Intent(RingtoneManager.ACTION_RINGTONE_PICKER);
                intent.putExtra(RingtoneManager.EXTRA_RINGTONE_TYPE, RingtoneManager.TYPE_NOTIFICATION);
                intent.putExtra(RingtoneManager.EXTRA_RINGTONE_TITLE, "Choose a sound");
                intent.putExtra(RingtoneManager.EXTRA_RINGTONE_EXISTING_URI, (Uri) null);
                startActivityForResult(intent, RESULT_RINGTONE_URI);
            }

        });
        Button findPhoneDefault = (Button)findViewById(R.id.find_phone_sound_default);
        final TextView currentSound = (TextView)findViewById(R.id.find_phone_body_current_file);
        findPhoneDefault.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                Runtime.log(context, TAG, "Setting Find Phone to default", Logger.ERROR);
                SharedPreferences.Editor ed = prefs.edit();
                ed.putString(Keys.PREF_KEY_FIND_PHONE_FILE, Keys.PREF_VALUE_FIND_PHONE_DEFAULT);
                ed.commit();

                currentSound.setText("None set. Notification sound will be used.");
            }

        });
        String currentPath = prefs.getString(Keys.PREF_KEY_FIND_PHONE_FILE, Keys.PREF_VALUE_FIND_PHONE_DEFAULT);
        if(currentPath.equals(Keys.PREF_VALUE_FIND_PHONE_DEFAULT)) {
            currentPath = "None set. Notification sound will be used.";
        } else {
            currentPath = "Current: " + prefs.getString(Keys.PREF_KEY_FIND_PHONE_TITLE, currentPath);
        }
        currentSound.setText(currentPath);

        // Show layout items
        handler.postDelayed(new Runnable() {

            @Override
            public void run() {
                showCard(optionsCard);
            }

        }, 400);
		handler.postDelayed(new Runnable() {
			
			@Override
			public void run() {
				showCard(layoutCard);
			}
			
		}, 600);
        handler.postDelayed(new Runnable() {

            @Override
            public void run() {
                showCard(newsCard);
                downloadNews();
            }

        }, 800);

        // Ask for location permission, required for Hotspot toggle
        if (ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED)
        {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.ACCESS_FINE_LOCATION},
                    REQUEST_LOCATION);
        }

        // Check for DND permission, required for ringer mode silent toggle (ugh)
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
            if (!notificationManager.isNotificationPolicyAccessGranted()) {
                UserInterface.showDialog(
                    this,
                    "Permission Required",
                    "Do Not Disturb permission is required for the Ringer toggle to be able to set the Silent mode.\n\nPlease allow Dashboard on the next screen.", "Open DND Settings", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // https://stackoverflow.com/a/39152607
                            Intent intent = new Intent(android.provider.Settings.ACTION_NOTIFICATION_POLICY_ACCESS_SETTINGS);
                            startActivity(intent);
                        }
                    },
                    "Cancel", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                        }
                    }
                );
            }
        }

        // Do our best to be alive for future background usage...
        PebbleReceiver.launchHandlerService(context, "");
	}

	private void requestDeviceAdmin() {
        final Context context = getApplicationContext();
        UserInterface.showDialog(this, "Device Administrator Required",
                "In order to enable Lock Phone functionality, the following Device Administrator policies are required:\n\n"
                        + "Lock the screen: Required to lock the phone when the corresponding watchapp action is chosen.\n\n"
                        + "Permission will be requested on the next screen. Please grant the permission to enable the Lock Phone functionality.",
                "OK", new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        Intent intent = new Intent(DevicePolicyManager.ACTION_ADD_DEVICE_ADMIN);
                        intent.putExtra(DevicePolicyManager.EXTRA_DEVICE_ADMIN, new ComponentName(context, DeviceAdmin.class));
                        intent.putExtra(DevicePolicyManager.EXTRA_ADD_EXPLANATION, "Device Administrator policy 'Lock the screen' is required for Lock Phone toggle functionality.");
                        startActivityForResult(intent, ADMIN_RESULT);
                        dialogInterface.dismiss();
                    }
                },
                "Cancel", new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        dialogInterface.dismiss();
                    }
                }
        );
    }

    /**
     * http://stackoverflow.com/a/29697808
     */
    public Map<String, String> getNotificationUris() {
        RingtoneManager manager = new RingtoneManager(this);
        manager.setType(RingtoneManager.TYPE_NOTIFICATION);
        Cursor cursor = manager.getCursor();

        Map<String, String> list = new HashMap<>();
        while (cursor.moveToNext()) {
            String notificationTitle = cursor.getString(RingtoneManager.TITLE_COLUMN_INDEX);
            String notificationUri = cursor.getString(RingtoneManager.URI_COLUMN_INDEX) + "/" + cursor.getString(RingtoneManager.ID_COLUMN_INDEX);
            list.put(notificationUri, notificationTitle);
        }

        return list;
    }

    private void testFindPhoneSound() {
        Context context = getApplicationContext();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);

        String filePath = prefs.getString(Keys.PREF_KEY_FIND_PHONE_FILE, Keys.PREF_VALUE_FIND_PHONE_DEFAULT);
        if(filePath.equals(Keys.PREF_VALUE_FIND_PHONE_DEFAULT)) {
            Runtime.log(context, TAG, "No Find Phone sound set during test, using default", Logger.ERROR);
            SharedPreferences.Editor ed = prefs.edit();
            ed.putString(Keys.PREF_KEY_FIND_PHONE_FILE, Keys.PREF_VALUE_FIND_PHONE_DEFAULT);
            ed.commit();
        }

        // Choose user's choice or default
        String uriString = prefs.getString(Keys.PREF_KEY_FIND_PHONE_FILE, Keys.PREF_VALUE_FIND_PHONE_DEFAULT);
        Uri uri;
        if(uriString.equals(Keys.PREF_VALUE_FIND_PHONE_DEFAULT)) {
            uri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
            Runtime.log(context, TAG, "Used Notification tone.", Logger.INFO);
            Toast.makeText(context, "Used default Notification sound.", Toast.LENGTH_SHORT).show();
        } else {
            uri = Uri.parse(uriString);
            Runtime.log(context, TAG, "Using loaded Uri: " + uriString, Logger.INFO);
        }

        // Use user's path
        playSound(uri);
    }

    private void playSound(Uri uri) {
        Context context = getApplicationContext();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        final AudioManager audioManager = (AudioManager)context.getSystemService(Context.AUDIO_SERVICE);

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
                mediaPlayer.setLooping(false);
                mediaPlayer.prepare();
                mediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {

                    @Override
                    public void onCompletion(MediaPlayer mp) {
                        // Reset volume
                        audioManager.setStreamVolume(AudioManager.STREAM_ALARM, userVolume, AudioManager.FLAG_PLAY_SOUND);

                        mediaPlayer.reset();
                    }

                });
                mediaPlayer.start();
                Runtime.log(context, TAG, "Sound started.", Logger.INFO);
            }
        } catch (IOException e) {
            Toast.makeText(context, "Your Find Phone sound was unavailable. Resetting to default...", Toast.LENGTH_LONG).show();
            SharedPreferences.Editor ed = prefs.edit();
            ed.putString(Keys.PREF_KEY_FIND_PHONE_FILE, Keys.PREF_VALUE_FIND_PHONE_DEFAULT);
            ed.commit();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        firstTimeSetup();
	}

    @Override
	public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_landing, menu);
		return true;
	}

    @Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
			case R.id.action_settings:
                startActivity(new Intent(this, AboutActivity.class));
				break;
		}

		return super.onOptionsItemSelected(item);
	}

	private void firstTimeSetup() {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		SharedPreferences.Editor ed = prefs.edit();

		// Check this is the first time
		if (prefs.getBoolean(Keys.PREF_FIRST_RUN, true)) {
			resetConfiguration(ed);
			ed.putBoolean(Keys.PREF_FIRST_RUN, false);
		}

		ed.commit();
	}

	private static void resetConfiguration(SharedPreferences.Editor ed) {
		// Set up ordering prefs
		for (int i = 0; i < Build.NUM_TOGGLES; i++) {
			// Default layout
			ed.putInt(Keys.PREF_CONFIGURE_BASE + "" + i, i);
		}

        ed.commit();
	}

	private void saveConfig(Spinner modified, int newType) {
        Context context = getApplicationContext();

		// Selection Correction Logic (tm) - "IT LIIIVVVEEESSS!!"
		try {
			for(int i = 0; i < Build.NUM_TOGGLES; i++) {
				Spinner test = spinners.get(i);
				
				//If not the one that just changed
				if(test != modified) {
					//If duplicate of newly selected type
					if(test.getSelectedItemPosition() == newType) {
						//Make it the one that is missing from the whole list - genius
						test.setSelection(findMissing(), true);
					}
				}
			}
		} catch(Exception e) {
			Toast.makeText(context, "Error checking duplicates. Send log to dev.", Toast.LENGTH_LONG).show();
			Runtime.logStackTrace(context, e);
		}
		
		//Store current set
		SharedPreferences.Editor ed = PreferenceManager.getDefaultSharedPreferences(context).edit();
		for(int i = 0; i < Build.NUM_TOGGLES; i++) {
			ed.putInt(Keys.PREF_CONFIGURE_BASE + "" + i, spinners.get(i).getSelectedItemPosition());
		}
		ed.commit();
	}

	private void downloadNews() {
		newsProgressBar.setVisibility(View.VISIBLE);
		newsBody.setText("");
		new Thread(new Runnable() {
	
			@Override
			public void run() {
                final Context context = getApplicationContext();
				try {
					final JSONObject appsJson = Web.downloadJSON(NoCommit.APP_VERSIONS_JSON_URL);
                    JSONObject dashboardObj = appsJson.getJSONObject("dashboard");
                    final String newsString = dashboardObj.getString("news");
	
					handler.postDelayed(new Runnable() {

                        @Override
                        public void run() {
                            try {
                                // Hide progressbar
                                newsProgressBar.setVisibility(View.GONE);

                                // Show news
                                newsBody.setText(newsString);
                            } catch (Exception e) {
                                newsBody.setText("There was a problem downloading the news text.");
                                e.printStackTrace();
                                Runtime.log(context, TAG, "Exception fetching news.", Logger.ERROR);
                                Runtime.logStackTrace(context, e);
                            }
                        }

                    }, 1000);
				} catch (Exception e) {
					e.printStackTrace();
                    Runtime.log(context, TAG, "Exception fetching news.", Logger.ERROR);
                    Runtime.logStackTrace(context, e);
					handler.post(new Runnable() {
	
						@Override
						public void run() {
							newsBody.setText("There was a problem downloading the news text.");
						}
	
					});
				}
			}
	
		}).start();
	}

	private void showCard(final CardView card) {
		Animation showAnim = AnimationUtils.loadAnimation(getApplicationContext(), R.anim.slide_in_bottom);
		showAnim.setAnimationListener(new AnimationListener() {

            @Override
            public void onAnimationStart(Animation animation) {
                card.setVisibility(View.VISIBLE);
            }

            @Override
            public void onAnimationRepeat(Animation animation) { }
            public void onAnimationEnd(Animation animation) { }

        });
        card.startAnimation(showAnim);
	}

	private int findMissing() {
		//Mark which are found
		boolean[] founds = new boolean[Build.NUM_TOGGLES];
		
		for(int i = 0; i < Build.NUM_TOGGLES; i++) {
			int type = spinners.get(i).getSelectedItemPosition();
			if(!founds[type]) {
				//This type exists in array!
				founds[type] = true;
			}
		}
		
		//Find which was not found
		for(int i = 0; i < founds.length; i++) {
			if(!founds[i]) {
				return i;
			}
		}
		
		//All were found
		return -1;
	}

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        Context context = getApplicationContext();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences.Editor ed = prefs.edit();

        switch(requestCode) {
            case ADMIN_RESULT:
                // The user elected to enable Device Admin for Dashboard to enable the Lock Phone toggle
                if (resultCode == Activity.RESULT_OK) {
                    ed.putBoolean(Keys.PREF_KEY_ADMIN, true);
                    Toast.makeText(context, "Device Administrator status obtained", Toast.LENGTH_SHORT).show();
                    adminSwitch.setChecked(true);
                } else {
                    ed.putBoolean(Keys.PREF_KEY_ADMIN, false);
                    Toast.makeText(context, "Did not get Device Administrator status", Toast.LENGTH_SHORT).show();
                    adminSwitch.setChecked(false);
                }
                break;
            case RESULT_RINGTONE_URI:
                // The user chose a ringtone for the full charge notificaion
                if (resultCode == Activity.RESULT_OK) {
                    Uri uri = data.getParcelableExtra(RingtoneManager.EXTRA_RINGTONE_PICKED_URI);
                    if (uri != null) {
                        // Get the URI
                        String uriString = uri.toString();
                        ed.putString(Keys.PREF_KEY_FIND_PHONE_FILE, uriString);

                        // Get the title
                        Map<String, String> uris = getNotificationUris();
                        String title = uris.get(uriString);
                        if(title == null) {
                            Runtime.log(context, TAG, "Unable to get ringtone title for URI returned (!?)", Logger.ERROR);
                            title = uriString;
                        }
                        ed.putString(Keys.PREF_KEY_FIND_PHONE_TITLE, title);

                        TextView chosenSoundView = (TextView)findViewById(R.id.find_phone_body_current_file);
                        chosenSoundView.setText("Current: " + title);
                    }
                } else {
                    Runtime.log(context, TAG, "User cancelled ringtone file selection", Logger.INFO);
                }
                break;
        }
        ed.commit();
    }
}
