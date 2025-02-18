package entry;

import java.io.File;

import pebble.CLPebbleUtils;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;

import com.wordpress.ninedof.datatoggle.R;

public class MainActivity extends Activity {
	//Other members
	private File appFile;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		ActionBar aBar = getActionBar();
		aBar.setTitle("Installer");
		aBar.setBackgroundDrawable(new ColorDrawable(Color.argb(255, 119, 0, 0)));
		
		//Setup UI elements
		Button installButton = (Button) findViewById(R.id.install_button);
		installButton.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				CLPebbleUtils.installWatchapp(MainActivity.this, "datatoggle.pbw", "5315d09e3184b45799000192");
			}
			
		});
		
		ImageView email = (ImageView) findViewById(R.id.email_button);
		ImageView twitter = (ImageView) findViewById(R.id.twitter_button);
		ImageView blog = (ImageView) findViewById(R.id.blog_button);
		ImageView rate = (ImageView) findViewById(R.id.store_button);
		
		email.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				//Send email
				Intent mailto = new Intent(Intent.ACTION_SEND); 
		        mailto.setType("message/rfc822") ; 
		        mailto.putExtra(Intent.EXTRA_EMAIL, new String[]{"bonsitm@gmail.com"});
		        mailto.putExtra(Intent.EXTRA_SUBJECT,"Data Toggle Installer Feedback");
		        mailto.putExtra(Intent.EXTRA_TEXT,"");
		        startActivity(Intent.createChooser(mailto, "Complete action using"));
			}
			
		});
		
		twitter.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				Intent intent = new Intent(Intent.ACTION_VIEW);
		        intent.setData(Uri.parse("http://www.twitter.com/Chris_DL"));
		        startActivity(intent);
			}
			
		});
		
		blog.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				Intent intent = new Intent(Intent.ACTION_VIEW);
		        intent.setData(Uri.parse("http://ninedof.wordpress.com"));
		        startActivity(intent);
			}
			
		});
		
		rate.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				Intent intent = new Intent(Intent.ACTION_VIEW);
		        intent.setData(Uri.parse("https://play.google.com/store/apps/details?id=com.wordpress.ninedof.datatoggle"));
		        startActivity(intent);
			}
			
		});
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		
		checkChangelogs();
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		
		//Install cleanup
		if(appFile != null)
			appFile.delete();
	}
	
	private void checkChangelogs() {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		
		//v9 changes
		if(prefs.getBoolean("v2changelog", true)) {
			showDialog("What's New", 
					   "v1.1.0\n"
					   + "- Added Pebble Appstore link when installing to future proof and as a workaround for Beta 10 users experienceing the sideloading bug.",
					   "Done", new DialogInterface.OnClickListener() {
						
							@Override
							public void onClick(DialogInterface dialog, int which) {
								dialog.dismiss();						
							}
							
						}, 
						null, null
			);
			
			//Clear
			SharedPreferences.Editor ed = prefs.edit();
			ed.putBoolean("v2changelog", false);
			ed.commit();
		}
	}
	
	private void showDialog(String title, String message, 
			   String positiveLabel, DialogInterface.OnClickListener positiveListener, 
			   String negativeLabel, DialogInterface.OnClickListener negativeListener) {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle(title);
		builder.setMessage(message);
		
		if(negativeListener != null)
		builder.setNegativeButton(negativeLabel, negativeListener);
		
		if(positiveListener != null)
		builder.setPositiveButton(positiveLabel, positiveListener);
		
		AlertDialog dialog = builder.create();
		dialog.show();
	}

}
