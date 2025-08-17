package fragment;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.widget.Toast;

import com.stericson.RootTools.RootTools;
import com.wordpress.ninedof.dashboard.R;

import activity.LogViewer;
import cl_toolkit.Logger;
import cl_toolkit.Platform;
import config.Build;
import config.Keys;
import config.Runtime;

public class About extends PreferenceFragment {

	private static final String TAG = About.class.getName();
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

        final Context context = getActivity().getApplicationContext();
        addPreferencesFromResource(R.xml.preferences);

        // Set version string
        Preference about = (Preference)findPreference("pref_key_version");
        about.setTitle("Dashboard v" + Build.VERSION + " by Chris Lewis");

        //Load prefs
        if(Platform.isLollipopOrAbove()) {
            // Enable Data toggle
            CheckBoxPreference dataPref = (CheckBoxPreference)findPreference(Keys.PREF_KEY_DATA_ENABLED);
            if(dataPref != null) {
                dataPref.setOnPreferenceClickListener(new OnPreferenceClickListener() {

                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        if (((CheckBoxPreference)preference).isChecked()) {
                            if(!testRoot()) {
                                Toast.makeText(context, "Enabled, but root access is not available. The Data toggle will not work.", Toast.LENGTH_LONG).show();
                            }
                        }
                        return true;
                    }

                });
            }

            Preference rootTestPref = findPreference(Keys.PREF_KEY_ROOT_ACCESS);
            if (rootTestPref != null) {
                rootTestPref.setOnPreferenceClickListener(new OnPreferenceClickListener() {

                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        boolean success = testRoot();
                        Toast.makeText(context, "Root access test " +
                            (success ? "succeeded!" : "failed! Device may not be rooted, or access was not granted."), Toast.LENGTH_LONG).show();
                        return true;
                    }

                });
            } else {
                Runtime.log(context, TAG, "rootTestPref null!", Logger.ERROR);
            }
        } else {
            // Not required, remove root stuff
            PreferenceScreen advancedScreen = (PreferenceScreen)findPreference("pref_screen");
            PreferenceCategory advancedCategory = (PreferenceCategory)findPreference("pref_category_advanced");
            advancedScreen.removePreference(advancedCategory);
        }

		//Debug log
		Preference logPref = findPreference(Keys.PREF_KEY_VERSION);
		logPref.setOnPreferenceClickListener(new OnPreferenceClickListener() {
			
			@Override
			public boolean onPreferenceClick(Preference preference) {
				// Show log viewer
                Intent i = new Intent(getActivity(), LogViewer.class);
				startActivity(i);
				return true;
			}
			
		});
	}

    private boolean testRoot() {
        Context context = getActivity().getApplicationContext();

        // Test root access
        boolean success;
        try {
            success = RootTools.isAccessGiven();
        } catch(Exception e) {
            Runtime.log(context, TAG, "Failed to get root access", Logger.ERROR);
            Runtime.logStackTrace(context, e);
            success = false;
        }

        Runtime.log(context, TAG, "Root test result: " + success, Logger.INFO);
        return success;
    }

}