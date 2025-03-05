package activity;

import com.wordpress.ninedof.wristponder.R;

import fragment.SettingsFragment;
import android.app.ActionBar;
import android.app.Activity;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;

public class Settings extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.settings_fragment);
		
		ActionBar ab = getActionBar();
		ab.setTitle("Settings");
		ab.setBackgroundDrawable(new ColorDrawable(getResources().getColor(R.color.main_color)));
		ab.setHomeButtonEnabled(true);
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		
		getFragmentManager().beginTransaction()
            .replace(R.id.fragment_layout, new SettingsFragment())
            .commit();
	}
}
