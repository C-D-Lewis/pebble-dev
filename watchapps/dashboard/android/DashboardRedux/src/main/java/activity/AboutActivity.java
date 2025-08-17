package activity;

import android.app.ActionBar;
import android.app.Activity;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;

import com.wordpress.ninedof.dashboard.R;

import fragment.About;

public class AboutActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.settings_fragment);

		ActionBar ab = getActionBar();
		ab.setTitle("About Dashboard");
		ab.setBackgroundDrawable(new ColorDrawable(getResources().getColor(R.color.main_colour)));
		ab.setHomeButtonEnabled(true);
	}

	@Override
	protected void onResume() {
		super.onResume();

		getFragmentManager()
            .beginTransaction()
            .replace(R.id.fragment_layout, new fragment.About())
            .commit();
	}
}
