package adapter;

import java.util.ArrayList;

import util.FileDatabase;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import com.wordpress.ninedof.wristponder.R;

import config.Build;

public class ResponseAdapter extends ArrayAdapter<String> {
	//Configuration
	public static final String TAG = ResponseAdapter.class.getName();
	
	//Other members
	private Context context;
	private ArrayList<String> responses;
	
	public ResponseAdapter(Context c, FileDatabase inDb) {
		super(c, android.R.layout.activity_list_item, inDb.getArrayList());
		context = c;
		responses = inDb.getArrayList();
	}
	
	/**
	 * For later adding items
	 * @param context	Context of application
	 */
	public ResponseAdapter(Context context) {
		super(context, android.R.layout.activity_list_item);
		this.context = context;
		
		responses = new ArrayList<String>();
		responses.add("");
	}
	
	@Override
	public View getView(final int position, View convertView, ViewGroup parent) {
		//Inflate
		LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	    View rootView = inflater.inflate(R.layout.message_list_item, parent, false);
	    
	    //Setup UI
	    TextView body = (TextView) rootView.findViewById(R.id.body);
	    body.setText(responses.get(position));

	    //Show length
	    TextView lengthView = (TextView) rootView.findViewById(R.id.response_length);
	    lengthView.setText("" + responses.get(position).length() + "/" + Build.MAX_LENGTH + " characters");
	    
	    //Animate
	    Animation animation = AnimationUtils.loadAnimation(getContext(), R.anim.slide_in_bottom);
	    rootView.startAnimation(animation);
	    
	    //Finally return 
	    return rootView;
	}
}
