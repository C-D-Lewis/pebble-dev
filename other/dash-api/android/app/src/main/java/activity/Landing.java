package activity;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarActivity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.ListView;
import android.widget.TextView;

import com.wordpress.ninedof.dashapi.R;

import java.util.ArrayList;
import java.util.UUID;

import data.AppAdapter;
import data.PermissionManager;

public class Landing extends ActionBarActivity {

    private static TextView noApps;
    private static ListView appList;

    /**
     * Release checklist
     * Manifest version, library compatible versions (both sides), prefs xml version string
     */

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_landing);

        appList = (ListView)findViewById(R.id.app_list);
        noApps = (TextView)findViewById(R.id.no_apps);

        ActionBar actionBar = getSupportActionBar();
        actionBar.setTitle("Dash API");
    }

    @Override
    protected void onResume() {
        super.onResume();
        reloadData(getApplicationContext());
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu_landing, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.about:
                Intent i = new Intent(getApplicationContext(), AboutActivity.class);
                startActivity(i);
                return true;
            case R.id.refresh:
                reloadData(getApplicationContext());
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    public void reloadData(Context context) {
        ArrayList<UUID> uuidArr = PermissionManager.getList(context);
        String[] uuids = new String[uuidArr.size()];
        for(int i = 0; i < uuidArr.size(); i++) {
            uuids[i] = uuidArr.get(i).toString();
        }
        AppAdapter adapter = new AppAdapter(getSupportActionBar().getThemedContext(), uuids);
        appList.setAdapter(adapter);

        if(uuidArr.size() > 0) {
            noApps.setVisibility(View.INVISIBLE);
        } else {
            noApps.setVisibility(View.VISIBLE);
        }
    }
}
