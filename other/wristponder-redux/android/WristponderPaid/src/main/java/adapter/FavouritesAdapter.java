package adapter;

import java.util.ArrayList;

import activity.Favourites;
import config.Build;
import util.ContactBundle;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.wordpress.ninedof.wristponder.R;

public class FavouritesAdapter extends ArrayAdapter<ContactBundle> {
	//Configuration
	public static final String TAG = FavouritesAdapter.class.getName();
	
	//Other members
	private Context context;
	private ArrayList<ContactBundle> bundles = new ArrayList<ContactBundle>();
    private ListView parentList;

	public FavouritesAdapter(Context context, ArrayList<ContactBundle> bundles, ListView parentList) {
		super(context, android.R.layout.activity_list_item, bundles);
		this.bundles = bundles;
		this.context = context;
        this.parentList = parentList;
	}
	
	@Override
	public View getView(final int position, View convertView, final ViewGroup parent) {
		//Inflate
		LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	    View rootView = inflater.inflate(R.layout.fav_spinner_2line, parent, false);
	    
	    //Setup UI
	    final TextView name = (TextView)rootView.findViewById(R.id.name);
	    name.setText(bundles.get(position).name);

	    //Show length
	    final TextView number = (TextView)rootView.findViewById(R.id.number);
	    number.setText(bundles.get(position).number);
	    
	    //Finally return
	    return rootView;
	}

}
