package adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.wordpress.ninedof.dashboard.R;

public class ToggleSelector extends ArrayAdapter<String> {

    public static final String[] items = {
        "Wi-Fi",
        "Data",
        "BT",
        "Ringer",
        "Auto Sync",
        "Hotspot",
        "Find Phone",
        "Lock Phone",
        "Auto Brightness"
    };

    public static final int[] iconIds = {
        R.drawable.wifi,
        R.drawable.data,
        R.drawable.bt,
        R.drawable.loud,
        R.drawable.sync,
        R.drawable.ap,
        R.drawable.phone,
        R.drawable.lock,
        R.drawable.brightness
    };

	private Context context;

	public ToggleSelector(Context context) {
		super(context, R.layout.toggle_spinner_item, items);
		
		this.context = context;
	}
	
	@Override
	public View getDropDownView(int position, View convertView, ViewGroup parent) {
		return inflateView(position, parent);
	}
	
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		return inflateView(position, parent);
	}

	public View inflateView(int position, ViewGroup parent) {
		//Inflate view
		LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View rootView = inflater.inflate(R.layout.toggle_spinner_item, parent, false);

	    //Get items
	    ImageView icon = (ImageView)rootView.findViewById(R.id.icon);
	    TextView name = (TextView)rootView.findViewById(R.id.name);
	    
		icon.setImageResource(iconIds[position]);
		name.setText(items[position]);
		return rootView;
	}

}
