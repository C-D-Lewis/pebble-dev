package data;

import android.content.Context;
import android.support.v7.widget.SwitchCompat;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.wordpress.ninedof.dashapi.R;

import java.util.UUID;

public class AppAdapter extends ArrayAdapter<String> {

    private Context context;
    private String[] uuids;

    public AppAdapter(Context themedContext, String[] uuids) {
        super(themedContext, R.layout.app_list_item, uuids);

        this.context = themedContext;
        this.uuids = uuids;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        LayoutInflater inflater = (LayoutInflater)context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View rowView = inflater.inflate(R.layout.app_list_item, parent, false);
        final UUID thisUuid = UUID.fromString(uuids[position]);

        TextView nameView = (TextView)rowView.findViewById(R.id.app_name);
        nameView.setText(PermissionManager.getName(context, thisUuid));

        TextView uuidView = (TextView)rowView.findViewById(R.id.app_uuid);
        uuidView.setText(thisUuid.toString());

        SwitchCompat permSwitch = (SwitchCompat) rowView.findViewById(R.id.app_switch);
        boolean permitted = PermissionManager.isPermitted(context, thisUuid);
        permSwitch.setChecked(permitted);
        permSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                PermissionManager.setPermitted(context, thisUuid, isChecked);
            }
            
        });

        return rowView;
    }

}
