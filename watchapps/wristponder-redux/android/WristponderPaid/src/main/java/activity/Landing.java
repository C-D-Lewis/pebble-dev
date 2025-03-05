package activity;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.wordpress.ninedof.wristponder.R;

import adapter.ResponseAdapter;
import cl_toolkit.Logger;
import cl_toolkit.UserInterface;
import config.Build;
import config.Keys;
import config.Runtime;
import pebble.PebbleUtils;
import util.FileDatabase;
import util.PermissionHelper;

public class Landing extends Activity {
	
	//Configuration
	public static final String TAG = Landing.class.getName();
	
	//Other members
	public static FileDatabase db;
	public ResponseAdapter adapter; 
	private Handler handler = new Handler();
	private TextView capLabel;
	private ProgressBar pBar;
	
	//State
	private static int capLevel;

	private void checkChangeLog() {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		if(prefs.getBoolean(Build.PREF_CHANGELOG_PREFIX + "" + Build.VERSION, true)) {

			/**
			 * RELEASE CHECKLIST
			 * - Android app version numbers MANIFEST - STRINGS - BUILD
			 * - Watchapp version numbers APPINFO - CONFIG.H
			 * - APP_VERSIONS.JSON .wristponder
			 * - Check asset .pbw is up to date!
			 */
			UserInterface.showDialog(this, "What's New (v" + Build.VERSION_STRING + "):",
                  "- Added search of contacts by first initial or by voice.\n"
                + "- Improved design for Material devices.\n"
                + "- Reworked protocol.\n",
                "Done", new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }

                },
                null, null
			);

			//Turn off flag
			SharedPreferences.Editor ed = prefs.edit();
			ed.putBoolean(Build.PREF_CHANGELOG_PREFIX + "" + Build.VERSION, false);
			ed.commit();
		}
	}

    private void checkMOnboarding() {
        if(android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.M) {
            // Old perms model, OK
            Runtime.log(this, TAG, "Not Android M, old permission model.", Logger.INFO);
            return;
        }

        // New perms model - onboarded yet?
        if(PermissionHelper.isPermitted(this)) {
            Runtime.log(this, TAG, "Android M, already onboarded", Logger.INFO);
            return;
        }

        // Not onboarded, show perms window
        Intent i = new Intent(this, MOnboarding.class);
        startActivity(i);
    }

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_landing);
		
		ActionBar ab = getActionBar();
		ab.setTitle("Wristponder");

        Button installButton = (Button)findViewById(R.id.install_button);
        installButton.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                PebbleUtils.sideloadInstall(getApplication(), Build.PBW_FILENAME);
            }

        });

		//Load strings from prefs
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		String sortMode = prefs.getString(Keys.PREF_SORT_STRING, FileDatabase.SORT_ALPHABETIC_ASC);
		db = new FileDatabase(this, Build.RESPONSE_DATABASE_NAME, sortMode);

		checkFirstRun();
        checkMOnboarding();
		
		//Setup list
		adapter = new ResponseAdapter(this, db);
		ListView list = (ListView)findViewById(R.id.response_list);
		list.setDivider(new ColorDrawable(Color.parseColor("#00000000")));
		list.setAdapter(adapter);
		final Context context = this;
		list.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
			
			@Override
			public boolean onItemLongClick(AdapterView<?> parent, View view, final int position, long id) {
				//Show edit dialog
				AlertDialog.Builder b = new AlertDialog.Builder(context);
				AlertDialog d = b.create();
				d.setTitle("Edit/Delete Response");
				
				final EditText et = new EditText(context);
				
				final String existing = db.getArrayList().get(position);
				et.setText(existing);
				d.setView(et);
				d.setButton(AlertDialog.BUTTON_POSITIVE, "Save", new DialogInterface.OnClickListener() {
					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						String newResponse = et.getText().toString();
						
						if(db.contains(newResponse)) {
							Toast.makeText(context, "Response already exists!", Toast.LENGTH_SHORT).show();
							return;
						} else if(newResponse.trim().equals("")) {
							Toast.makeText(context, "New response was blank!", Toast.LENGTH_SHORT).show();
							return;
						} else if(newResponse.length() > Build.MAX_LENGTH) {
							Toast.makeText(context, "Must be up to " + Build.MAX_LENGTH + " characters!", Toast.LENGTH_SHORT).show();
						} else {
							//Remove old
							db.remove(existing);
							adapter.remove(existing);
							
							//Add the new one
							db.put(newResponse);
						}
						
						//Reload
						adapter.notifyDataSetChanged();
					}
					
				});
				
				d.setButton(AlertDialog.BUTTON_NEUTRAL, "Cancel", new DialogInterface.OnClickListener() {
					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						dialog.dismiss();
					}
					
				});
				
				d.setButton(AlertDialog.BUTTON_NEGATIVE, "Delete", new DialogInterface.OnClickListener() {
					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						final String existing = db.getArrayList().get(position);
						
						AlertDialog.Builder builder = new AlertDialog.Builder(context);
						builder.setTitle("Confirm");
						builder.setMessage("Are you sure you wish to delete '" + existing + "'?");
						builder.setPositiveButton("Delete", new DialogInterface.OnClickListener() {
							
							@Override
							public void onClick(DialogInterface dialog, int which) {
								db.remove(existing);
								adapter.remove(existing);
								animateProgressBar();
							}
							
						});
						builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
							
							@Override
							public void onClick(DialogInterface dialog, int which) {
								dialog.dismiss();
							}
							
						});
						builder.show();
						dialog.dismiss();
					}
					
				});
				
				d.show();
				return true;
			}
			
		});
		adapter.notifyDataSetChanged();
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		
		Runtime.startNewSession(this);

		checkChangeLog();
		
		//Show usage
		pBar = (ProgressBar)findViewById(R.id.capacity_progress);
		pBar.setMax(Build.MAX_RESPONSES);
		capLabel = (TextView)findViewById(R.id.capacity_label);
		
		animateProgressBar();
		adapter.notifyDataSetChanged();
	}

	@Override
	protected void onPause() {
		super.onPause();
		
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.landing, menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
		case R.id.add_new:
			if(db.getArrayList().size() < Build.MAX_RESPONSES) {
				AlertDialog.Builder b = new AlertDialog.Builder(this);
				AlertDialog d = b.create();
				d.setTitle("New Response");
				
				final EditText et = new EditText(this);
				d.setView(et);
				d.setButton(AlertDialog.BUTTON_POSITIVE, "Add", new OnClickListener() {
					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						String newResponse = et.getText().toString();
						
						Log.d(TAG, "New repsonse: " + newResponse);
	
						if(db.contains(newResponse)) {
							Toast.makeText(getApplicationContext(), "Response already exists!", Toast.LENGTH_SHORT).show();
							Log.d(TAG, "Response: " + newResponse + " already saved!");
							return;
						} else if(newResponse.trim().equals("")) {
							Toast.makeText(getApplicationContext(), "New reponse was blank!", Toast.LENGTH_SHORT).show();
							Log.d(TAG, "Blank response attempted.");
							return;
						} else if(newResponse.length() > Build.MAX_LENGTH) {
							Toast.makeText(getApplicationContext(), "Must be up to " + Build.MAX_LENGTH + " characters!", Toast.LENGTH_SHORT).show();
							Log.d(TAG, "Must be less than 40 characters!");
						} else {
							//Add the new one
							db.put(newResponse);
						}
						
						//Reload
						adapter.notifyDataSetChanged();
						animateProgressBar();
					}
					
				});
				
				d.setButton(AlertDialog.BUTTON_NEGATIVE, "Cancel", new DialogInterface.OnClickListener() {
					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						dialog.dismiss();
					}
					
				});
				
				d.show();
			} else {
				Toast.makeText(this, "Maximum responses is " + Build.MAX_RESPONSES + ". Delete or edit an existing one.", Toast.LENGTH_SHORT).show();
			}
			break;
		case R.id.favs:
			Intent f = new Intent(getApplicationContext(), Favourites.class);
			startActivity(f);
			break;
		case R.id.settings:
			Intent i = new Intent(getApplicationContext(), Settings.class);
			startActivity(i);
			break;
		}
			
		//Continue as you were
		return false;
	}
	
	private void checkFirstRun() {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		if(prefs.getBoolean(Keys.PREF_FIRST_RUN, true)) {
			
			//Add defaults
			db.put("Hello from my Pebble!");
			db.put("Goodnight!");
			db.put("I'm currently unavailable.");
			db.put("I am currently in a meeting.");
			db.put("Be there soon.");
			db.put("Great!");
			db.put("OK");
			db.put("Ready?");
			
			//Testing full load
//			for(int i = 0; i < Config.MAX_RESPONSES; i++) {
//				String s = "";
//				
//				for(int j = 0; j < Config.MAX_LENGTH; j++) {
//					s = s + "" + (i % 10);
//				}
//				
//				db.put(s);
//			}
			
			//Set flag off
			SharedPreferences.Editor ed = prefs.edit();
			ed.putBoolean(Keys.PREF_FIRST_RUN, false);
			ed.commit();
		}
	}
	
	public void animateProgressBar() {
		final int currentUsage = db.getArrayList().size();
		
		//Animate filling up
		new Thread(new Runnable() {
			
			@Override
			public void run() {
				try {
					for(capLevel = 0; capLevel < currentUsage; capLevel++) {
						Thread.sleep(80);
						handler.post(new Runnable() {
							
							@Override
							public void run() {
								pBar.setProgress(capLevel); //10/32 = 9 on 0 to 9 scale 
								capLabel.setText(capLevel + "/" + Build.MAX_RESPONSES + " slots used");
							}
							
						});
					}
				} catch (Exception e) {
					Log.e(TAG, "Error animating progress bar");
					e.printStackTrace();
					handler.post(new Runnable() {
						
						@Override
						public void run() {
							pBar.setProgress(currentUsage - 1); //10/32 = 9 on 0 to 9 scale 
							capLabel.setText(currentUsage + "/" + Build.MAX_RESPONSES + " slots used");
						}
						
					});
				}
			}
			
		}).start();
	}

}
