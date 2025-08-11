package activity;

import java.util.ArrayList;

import fragment.DeleteFavoritePickerFragment;
import fragment.MultipleNumberPickerFragment;
import util.ContactBundle;
import adapter.FavouritesAdapter;
import android.app.ActionBar;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import cl_toolkit.Contact;
import cl_toolkit.Logger;

import com.wordpress.ninedof.wristponder.R;

import config.Build;
import config.Runtime;

public class Favourites extends Activity {
	
	public static final String TAG = Favourites.class.getName();
	
	private static final int CONTACT_PICKER_RESULT = 546;
	
	private ListView list;
	private int tempPosition;
    private FavouritesAdapter adapter;
    private ArrayList<ContactBundle> bundles;

    @Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.favourites_new);

		//Setup ActionBar
		ActionBar ab = getActionBar();
		ab.setBackgroundDrawable(getResources().getDrawable(R.color.main_color));
		ab.setTitle("Favourite Contacts");
		
		//Load all favourites from prefs
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		bundles = new ArrayList<ContactBundle>();
		for(int i = 0; i < 5; i++) {
			String number = prefs.getString(Build.PREFIX_FAVOURITE_NUMBER + "" + i, "Unknown");
			String name = Contact.getContactName(this, number, Build.MAX_NAME_LENGTH);
			
			bundles.add(new ContactBundle(name, number));
			Runtime.log(this, TAG, "Loaded favourite: " + name + ":" + number, Logger.INFO);
		}

		//Setup UI using LISTVIEW
		list = (ListView)findViewById(R.id.favourites_list);
		list.setDivider(new ColorDrawable(Color.parseColor("#00000000")));
		adapter = new FavouritesAdapter(this, bundles, list);
		list.setAdapter(adapter);
		list.setOnItemClickListener(new AdapterView.OnItemClickListener() {
			
			@Override
			public void onItemClick(AdapterView<?> parent, View view, final int position, long id) {
				//Store position for onResult
				tempPosition = position;
				
				//Launch contacts picker
				Intent i = new Intent(Intent.ACTION_PICK, Contacts.CONTENT_URI);
				startActivityForResult(i, CONTACT_PICKER_RESULT);
			}
			
		});


        list.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {

            @Override
            public boolean onItemLongClick(final AdapterView<?> parent, final View view, final int position, long id) {
                final TextView nameView = (TextView)list.getChildAt(position).findViewById(R.id.name);

                if(!nameView.getText().equals("Unknown")) {
                    // Ask for confirmation
                    DeleteFavoritePickerFragment frag = new DeleteFavoritePickerFragment() {

                        @Override
                        public void onDialogItemSelected() {
                            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
                            SharedPreferences.Editor ed = prefs.edit();
                            ed.remove("" + Build.PREFIX_FAVOURITE_NUMBER + position);
                            ed.commit();

                            TextView numberView = (TextView) list.getChildAt(position).findViewById(R.id.number);

                            // Favs are saved from number strings
                            nameView.setText("Unknown");
                            numberView.setText("Unknown");
                        }

                    };
                    frag.show(getFragmentManager(), "delete");
                }
                return true;
            }

        });
		
		//Just in case
		adapter.notifyDataSetChanged();
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		
		if(resultCode == RESULT_OK) {
            switch(requestCode) {
            case CONTACT_PICKER_RESULT:
            	//Extract data
            	Uri result = data.getData();
            	String id = result.getLastPathSegment();
            	
            	Cursor cursor = getContentResolver().query(
            	        Phone.CONTENT_URI, null,
            	        Phone.CONTACT_ID + "=?",
            	        new String[]{id}, null);

                ArrayList<String> numbers = new ArrayList<String>();
                String primarynumber = "";

                // Check there is at least one
            	if(cursor.moveToFirst()) {
            		int nameId = cursor.getColumnIndex(Phone.DISPLAY_NAME);
            	    int numberId = cursor.getColumnIndex(Phone.NUMBER);
            	    
            	    //Check contact has data
            	    if(nameId > 0 && numberId > 0) {
	            	    String name = cursor.getString(nameId);
	            	    primarynumber = cursor.getString(numberId);
                        numbers.add(primarynumber);
	            	    Runtime.log(this, TAG, "Contact picker got name/number: " + name + "/" + primarynumber, Logger.INFO);

                        // Get all numbers
                        while(cursor.moveToNext()) {
                            numberId = cursor.getColumnIndex(Phone.NUMBER);
                            String number = cursor.getString(numberId);

                            // Check not an API dupe
                            String primarySubstring = primarynumber.substring(primarynumber.length() - 4, primarynumber.length());
                            if(!number.contains(primarySubstring)) {
                                Runtime.log(this, TAG, "This contact also has this number; " + number, Logger.DEBUG);
                                numbers.add(number);
                            } else {
                                Runtime.log(this, TAG, "Discarded duplicate number " + primarynumber + " <-> " + number, Logger.DEBUG);
                            }
                        }
	            	    
	            	    //Update ListView
	            	    TextView nameView = (TextView)list.getChildAt(tempPosition).findViewById(R.id.name);
	            	    TextView numberView = (TextView)list.getChildAt(tempPosition).findViewById(R.id.number);

                        if(numbers.size() > 1) {
                            // Pick!
                            MultipleNumberPickerFragment frag = new MultipleNumberPickerFragment(numbers, nameView, name, numberView);
                            frag.show(getFragmentManager(), "numbers");
                        } else {
                            // Choose the only one
                            nameView.setText(name);
                            numberView.setText(numbers.get(0));
                        }
            	    } else {
            	    	Toast.makeText(this, "Could not get number from that contact.", Toast.LENGTH_LONG).show();
            	    	Runtime.log(this, TAG, "Could not get number from contact Cursor", Logger.ERROR);
            	    }
            	} else {
                    Toast.makeText(this, "Could not get number from that contact.", Toast.LENGTH_LONG).show();
                    Runtime.log(this, TAG, "Cursor was empty", Logger.ERROR);
                }
            	
            	cursor.close();
                break;
            }
        }
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.favs, menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
		case R.id.action_save:
			//Save the five choices as numbers
			SharedPreferences.Editor ed = PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).edit();
			
			//Save chosen NUMBERS
			for(int i = 0; i < 5; i++) {
				TextView numberView = (TextView)list.getChildAt(i).findViewById(R.id.number);
				
				String number = numberView.getText().toString();
				ed.putString(Build.PREFIX_FAVOURITE_NUMBER + "" + i, number);
				
				Runtime.log(this, TAG, "Storing favourite " + i + " as " + number, Logger.INFO);
			}
			
			//Finally
			ed.commit();
			Toast.makeText(getApplicationContext(), "Favourites saved!", Toast.LENGTH_SHORT).show();
			finish();
			break;
		}
		
		return false;
	}

}
