package activity;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.ColorDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import com.wordpress.ninedof.dashboard.R;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Timer;
import java.util.TimerTask;

import cl_toolkit.Logger;
import cl_toolkit.Storage;
import cl_toolkit.UserInterface;
import config.Build;
import config.Runtime;

public class LogViewer extends Activity {
	
	private static final String TAG = LogViewer.class.getName();

    private ActionBar actionBar;
	private TextView textView;
	private ScrollView scrollView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_log_viewer);
		
		//Setup ActionBar
		actionBar = getActionBar();
		actionBar.setTitle("Debug Log");
		actionBar.setBackgroundDrawable(new ColorDrawable(getResources().getColor(R.color.main_colour)));

        // Get UI members
		textView = (TextView)findViewById(R.id.log_view);
		scrollView = (ScrollView)findViewById(R.id.scroll_view);
		
		//Fill view
		refreshLog();
        beginTimer();
	}

    private void beginTimer() {
        Timer timer = new Timer();
        timer.scheduleAtFixedRate(new TimerTask() {

            @Override
            public void run() {
                runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
                        SimpleDateFormat format = new SimpleDateFormat("HH:mm:ss");
                        String time = format.format(Calendar.getInstance().getTime());
                        actionBar.setSubtitle(time);
                    }

                });
            }

        }, 0, 1000);
    }

	private void refreshLog() {
		new Thread(new Runnable() {

			@Override
			public void run() {
                Context context = getApplicationContext();

                try {
					readFile();
				} catch(Exception e) {
					Log.e(TAG, e.getMessage());

					//There may not be one yet
					Runtime.log(context, TAG, "Began new log file.", Logger.INFO);
					try {
						readFile();
					} catch(Exception e1) {
						Runtime.log(context, TAG, "That REALLY went wrong...", Logger.ERROR);
						Runtime.logStackTrace(context, e1);
					}
				}

			}

		}).start();
	}
	
	private void readFile() throws IOException {
		File logFile = new File(Storage.getAppStorage(getApplicationContext()) + "/" + Build.DEBUG_LOG_NAME);
		BufferedReader br = new BufferedReader(new FileReader(logFile));
		String next = br.readLine();
		StringBuilder content = new StringBuilder(Build.DEBUG_LOG_MAX_SIZE_BYTES);
		while(next != null) {
			content.append(next + "\n");
			next = br.readLine();
		}
		br.close();

		//Show it
		final String newString = content.toString();
		runOnUiThread(new Runnable() {

			@Override
			public void run() {
				textView.setText(newString);
                scrollView.post(new Runnable() {

                    @Override
                    public void run() {
                        scrollView.fullScroll(View.FOCUS_DOWN);
                    }
                });
			}

		});
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.menu_log_viewer, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
		case R.id.action_refresh:
			refreshLog();
			break;
		case R.id.action_report:
			UserInterface.showDialog(this, 
				"Log Reporting", 
				"You are about to send a copy of this log to the developer to help fix a problem.\n\nPlease include a description of the problem you are experiencing.\n\nThanks for your co-operation!",
				"Send Log", 
				new DialogInterface.OnClickListener() {
					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						sendDevMail();
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
			break;
		}
		
		//Finally
		return super.onOptionsItemSelected(item);
	}

	private void sendDevMail() {
		//Use the actual one
		File attachment = new File(Storage.getStorage() + "/" + Build.DEBUG_LOG_NAME);

		//Open email with attachment
		if(attachment != null && attachment.canRead()) {
			Intent emailIntent = new Intent(Intent.ACTION_SENDTO, Uri.fromParts("mailto","bonsitm@gmail.com", null));
			emailIntent.putExtra(Intent.EXTRA_SUBJECT, "Dashboard Debug Log File");
			emailIntent.putExtra(Intent.EXTRA_STREAM, Uri.fromFile(attachment));
			startActivity(Intent.createChooser(emailIntent, "Send email..."));
		} else {
			Toast.makeText(this, "Could not add attachment.", Toast.LENGTH_LONG).show();
		}
	}
	
}
