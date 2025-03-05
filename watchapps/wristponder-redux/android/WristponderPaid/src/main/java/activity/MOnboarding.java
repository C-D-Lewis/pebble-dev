package activity;

import android.annotation.TargetApi;
import android.app.ActionBar;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import com.wordpress.ninedof.wristponder.R;

import cl_toolkit.Logger;
import config.Runtime;
import util.PermissionHelper;

@TargetApi(23)
public class MOnboarding extends Activity {
	
	public static final String TAG = MOnboarding.class.getName();

    @Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.m_onboarding);

		//Setup ActionBar
		ActionBar ab = getActionBar();
		ab.setBackgroundDrawable(getResources().getDrawable(R.color.main_color));
		ab.setTitle("Permissions");
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.m_onboarding, menu);
		return true;
	}

    private boolean allPermsGranted(int[] grantResults) {
        for(int i : grantResults) {
            if(i != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
		case R.id.action_save:
            PermissionHelper.ask(this);
			break;
		}
		return false;
	}

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        // Check all permissions are granted
        if(allPermsGranted(grantResults)) {
            Runtime.log(getApplicationContext(), TAG, "All permissions granted!", Logger.INFO);
            finish();
        } else {
            Runtime.log(getApplicationContext(), TAG, "Some permissions not granted.", Logger.ERROR);
            Toast.makeText(getApplicationContext(), "All permissions must be granted to continue!", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void onBackPressed() {
        moveTaskToBack(true);
        android.os.Process.killProcess(android.os.Process.myPid());
        System.exit(1);
    }
}
