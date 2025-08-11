package activity;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import android.app.ActionBar;
import android.app.Activity;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.ColorDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;
import cl_toolkit.Logger;
import cl_toolkit.Storage;
import cl_toolkit.UserInterface;

import com.wordpress.ninedof.wristponder.R;

import config.Build;
import config.Runtime;

public class LogViewer extends Activity {
	
	private static final String TAG = LogViewer.class.getName();
	
	private TextView textView;
	private ScrollView containerView;
	private ProgressBar progressBar;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_log_viewer);
		
		//Setup ActionBar
		ActionBar aBar = getActionBar();
		aBar.setTitle("Debug Log");
		aBar.setBackgroundDrawable(new ColorDrawable(getResources().getColor(R.color.main_color)));
		
		textView = (TextView)findViewById(R.id.log_view);
		progressBar = (ProgressBar)findViewById(R.id.progress_bar);
		containerView = (ScrollView)findViewById(R.id.scroll_view);
		
		//Fill view
		refreshLog();
	}

	private void refreshLog() {
		//Hide
		containerView.setVisibility(View.GONE);
		progressBar.setVisibility(View.VISIBLE);
		
		new Thread(new Runnable() {

			@Override
			public void run() {
				try {
					readFile();
				} catch(Exception e) {
					//There may not be one yet
					Runtime.log(getApplicationContext(), TAG, "Began new log file.", Logger.INFO);
					try {
						readFile();
					} catch(Exception e1) {
						Runtime.log(getApplicationContext(), TAG, "That REALLY went wrong...", Logger.ERROR);
						Runtime.logStackTrace(getApplicationContext(), e1);
					}
				}

			}

		}).start();
	}
	
	private void readFile() throws FileNotFoundException, IOException {
		File logFile = new File(Storage.getAppStorage(getApplicationContext()) + "/" + Build.DEBUG_LOG_NAME);
		BufferedReader br = new BufferedReader(new FileReader(logFile));
		String next = br.readLine();
		StringBuilder content = new StringBuilder(Build.MAX_LOG_BYTES);
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
				progressBar.setVisibility(View.GONE);
				containerView.setVisibility(View.VISIBLE);
                containerView.post(new Runnable() {

                    @Override
                    public void run() {
                        containerView.fullScroll(View.FOCUS_DOWN);
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
				"You are about to send a copy of this log to the developer to help fix a problem.\n\n"
				+ "This may contain contact information, which will not be used for any purpose except to identify whether those parts of the app "
				+ "are working correctly. \n\nThanks for your co-operation!", 
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
		File attachment = new File(Storage.getAppStorage(getApplicationContext()) + "/" + Build.DEBUG_LOG_NAME);

		//Open email with attachment
		if(attachment != null && attachment.canRead()) {
			Intent emailIntent = new Intent(Intent.ACTION_SENDTO, Uri.fromParts("mailto","bonsitm@gmail.com", null));
			emailIntent.putExtra(Intent.EXTRA_SUBJECT, "Wristponder Debug Log File");
			emailIntent.putExtra(Intent.EXTRA_STREAM, Uri.fromFile(attachment));
			startActivity(Intent.createChooser(emailIntent, "Send email..."));
		} else {
			Toast.makeText(this, "Could not add attachment.", Toast.LENGTH_LONG).show();
		}
	}
	
}
