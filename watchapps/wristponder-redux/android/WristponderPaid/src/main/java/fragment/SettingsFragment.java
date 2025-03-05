package fragment;

import org.json.JSONObject;

import pebble.PebbleUtils;
import util.ContactFileDatabase;
import util.FileDatabase;
import activity.Landing;
import activity.LogViewer;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.widget.EditText;
import android.widget.Toast;
import cl_toolkit.Contact;
import cl_toolkit.Logger;
import cl_toolkit.UserInterface;
import cl_toolkit.Web;

import com.wordpress.ninedof.wristponder.R;

import config.Build;
import config.Keys;
import config.Runtime;

public class SettingsFragment extends PreferenceFragment {
	
	public static final String TAG = SettingsFragment.class.getName();
	
	private static final int
		RESULT_IMPORT = 3248;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		//Load prefs
		addPreferencesFromResource(R.xml.preferences);
		
		//Sort Mode
		ListPreference sortPref = (ListPreference)findPreference(Keys.PREF_SORT_STRING);
		sortPref.setEntries(R.array.human_sort_modes);
		sortPref.setEntryValues(R.array.array_sort_modes);
		String current = PreferenceManager.getDefaultSharedPreferences(getActivity()).getString(Keys.PREF_SORT_STRING, "atoz");
		if(current.equals(FileDatabase.SORT_ALPHABETIC_ASC)) {
			sortPref.setSummary("Alphabetic A-Z");
		} else if(current.equals(FileDatabase.SORT_ALPHABETIC_DESC)) {
			sortPref.setSummary("Alphabetic Z-A");
		}
		sortPref.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
			
			@Override
			public boolean onPreferenceChange(Preference preference, Object newValue) {
				//Update summary
				String current = newValue.toString();
				if(current.equals(FileDatabase.SORT_ALPHABETIC_ASC)) {
					preference.setSummary("Alphabetic A-Z");
				} else if(current.equals(FileDatabase.SORT_ALPHABETIC_DESC)) {
					preference.setSummary("Alphabetic Z-A");
				}
				
				//Update DB and UI
				Landing.db.setSortMode(newValue.toString());
				Landing.db.commit();
				
				//Keep new value
				return true;
			}
			
		});
		
		//Import
		Preference importPref = findPreference(Keys.PREF_IMPORT);
		importPref.setOnPreferenceClickListener(new OnPreferenceClickListener() {
			
			@Override
			public boolean onPreferenceClick(Preference preference) {
				//Get path
				Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
			    intent.setType("file/*");
			    startActivityForResult(intent, RESULT_IMPORT);
			    
			    //Rest handled in on activity result
				return true;
			}
		});
		
		//Export
		Preference exportPref = findPreference(Keys.PREF_EXPORT);
		exportPref.setOnPreferenceClickListener(new OnPreferenceClickListener() {
			
			@Override
			public boolean onPreferenceClick(Preference preference) {
				//Get path
				AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
				AlertDialog dialog = builder.create();
				dialog.setTitle("Export Path");
				
				final EditText editText = new EditText(getActivity());
				editText.setText(Environment.getExternalStorageDirectory().getPath() + "/Wristponder Export.txt");
				dialog.setView(editText);
				dialog.setButton(AlertDialog.BUTTON_POSITIVE, "Export", new OnClickListener() {
					
					@Override
					public void onClick(DialogInterface thisDialog, int which) {
						String newResponse = editText.getText().toString();
						
						//Export
						Toast.makeText(getActivity(), 
									   (Landing.db.save(newResponse) ? "Export successful!" : "Export not successful. Check the path is accessible."), 
									   Toast.LENGTH_SHORT
									  ).show();
					}
					
				});
				
				dialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Cancel", new DialogInterface.OnClickListener() {
					
					@Override
					public void onClick(DialogInterface thisDialog, int which) {
						thisDialog.dismiss();
					}
					
				});
				
				dialog.show();
				return true;
			}
		});
		
		//Show most frequent top 5
		final Preference topFivePref = findPreference(Keys.PREF_TOPFIVE);
		final ContactFileDatabase outgoingDb = new ContactFileDatabase(getActivity(), Build.INCOMING_DATABASE_NAME);
		final String[] tops = outgoingDb.getTopFive();
		topFivePref.setSummary(
			  "#1: " + (tops[0] != null ? Contact.getContactName(getActivity(), tops[0], Build.MAX_NAME_LENGTH) + " (" + (outgoingDb.get(tops[0]) != null ? outgoingDb.get(tops[0]) : "") + ")" : "Unknown") + "\n"
			+ "#2: " + (tops[1] != null ? Contact.getContactName(getActivity(), tops[1], Build.MAX_NAME_LENGTH) + " (" + (outgoingDb.get(tops[1]) != null ? outgoingDb.get(tops[1]) : "") + ")" : "Unknown") + "\n"
			+ "#3: " + (tops[2] != null ? Contact.getContactName(getActivity(), tops[2], Build.MAX_NAME_LENGTH) + " (" + (outgoingDb.get(tops[2]) != null ? outgoingDb.get(tops[2]) : "") + ")" : "Unknown") + "\n"
			+ "#4: " + (tops[3] != null ? Contact.getContactName(getActivity(), tops[3], Build.MAX_NAME_LENGTH) + " (" + (outgoingDb.get(tops[3]) != null ? outgoingDb.get(tops[3]) : "") + ")" : "Unknown") + "\n"
			+ "#5: " + (tops[4] != null ? Contact.getContactName(getActivity(), tops[4], Build.MAX_NAME_LENGTH) + " (" + (outgoingDb.get(tops[4]) != null ? outgoingDb.get(tops[4]) : "") + ")" : "Unknown")
		);
		
		//Reset
		Preference resetPref = findPreference(Keys.PREF_RESET);
		resetPref.setOnPreferenceClickListener(new OnPreferenceClickListener() {
			
			@Override
			public boolean onPreferenceClick(Preference preference) {
				UserInterface.showDialog(
				getActivity(),
				"Confirm", 
			   "Are you sure you wish to reset the most frequent contacts?", 
			   "Reset", 
			    new DialogInterface.OnClickListener() {
		
					@Override
					public void onClick(DialogInterface dialog, int which) {
						//Reset database
						ContactFileDatabase outgoingDb = new ContactFileDatabase(getActivity(), Build.INCOMING_DATABASE_NAME);
						outgoingDb.clear();
						
						topFivePref.setSummary(
								  "#1: Unknown\n"
								+ "#2: Unknown\n"
								+ "#3: Unknown\n"
								+ "#4: Unknown\n"
								+ "#5: Unknown"
						);
						
						Toast.makeText(getActivity(), "Counts cleared!", Toast.LENGTH_SHORT).show();
					}
					
				}, 
				"Cancel", 
				new DialogInterface.OnClickListener() {
					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						dialog.dismiss();
					}
					
				}
				);
				
				return true;
			}
		});
		
		//Get latest news
		final Preference latestPref = findPreference(Keys.PREF_LATEST);
		latestPref.setSummary("Fetching...");
		new Thread(new Runnable() {

			@Override
			public void run() {
				try {
					final JSONObject app_versions = Web.downloadJSON("https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/app_versions.json");	
					getActivity().runOnUiThread(new Runnable() {

						@Override
						public void run() {
							try {
								latestPref.setSummary(app_versions.getString("wristpondernews"));
							} catch(Exception e) {
								Runtime.log(getActivity().getApplicationContext(), TAG, "Exception fetching latest: " + e.getLocalizedMessage(), Logger.ERROR);
								latestPref.setSummary("Error downloading latest news: " + e.getLocalizedMessage());
								e.printStackTrace();
							}
						}
					});

				} catch(Exception e) {
					//getActivity() appears to be null here
					latestPref.setSummary("Error downloading latest news: " + e.getLocalizedMessage());
					e.printStackTrace();
				}
			}

		}).start();
		
		//Debug log
		Preference logPref = findPreference(Keys.PREF_VERSION);
		logPref.setOnPreferenceClickListener(new OnPreferenceClickListener() {
			
			@Override
			public boolean onPreferenceClick(Preference preference) {
				Intent i = new Intent(getActivity(), LogViewer.class);
				startActivity(i);
				
				return true;
			}
			
		});
	}
	
	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		
		switch(requestCode) {
		case RESULT_IMPORT:
			if(resultCode == Activity.RESULT_OK) {
				//Import
				String path = data.getData().getPath();
				
				//Nasty static hack
				Toast.makeText(getActivity(), 
                    (Landing.db.load(path) ? "Import successful!" : "Import not successful! Check file specified is a valid text file."),
                    Toast.LENGTH_SHORT).show();
			}
			break;
		
		}
	}
	
}
