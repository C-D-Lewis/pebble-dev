package data;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import java.util.ArrayList;
import java.util.UUID;

/**
 * Name:
 * K-V: NAME_UUID -> Name
 * Permitted:
 * K-V: PERM_UUID -> bool
 * List:
 * K-V: LIST -> uuid;uuid;uuid;...
 * Single list:
 * K-V: LIST -> uuid
 */
public class PermissionManager {

    private static final String TAG = PermissionManager.class.getName();
    
    private static final String
        KEY_NAME = "NAME_",
        KEY_PERMITTED = "PERMITTED_",
        KEY_LIST = "LIST";

    private static final String
        LIST_SEP = ";";

    public static boolean isPermitted(Context context, UUID uuid) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        String uuidPermString = KEY_PERMITTED + uuid.toString();
        if(!prefs.contains(uuidPermString)) {
            //Log.d(TAG, "Permission string does not yet exist");
            return false;
        }
        boolean permitted = prefs.getBoolean(uuidPermString, false);
        //Log.d(TAG, "Permitted? " + permitted);
        return permitted;
    }

    public static void setName(Context context, UUID uuid, String name) {
        SharedPreferences.Editor ed = PreferenceManager.getDefaultSharedPreferences(context).edit();
        String uuidKeyString = KEY_NAME + uuid.toString();
        ed.putString(uuidKeyString, name);
        ed.commit();
        //Log.d(TAG, "Name set to " + name);
    }

    public static String getName(Context context, UUID uuid) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        String uuidKeyString = KEY_NAME + uuid.toString();
        if(!prefs.contains(uuidKeyString)) {
            //Log.d(TAG, "Name not yet stored");
            return null;
        }
        String name = prefs.getString(uuidKeyString, "Unknown app");
        //Log.d(TAG, "Got name " + name);
        return name;
    }
    
    public static void setPermitted(Context context, UUID uuid, boolean permitted) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        String uuidString = uuid.toString();

        // Save
        SharedPreferences.Editor ed = prefs.edit();
        ed.putBoolean(KEY_PERMITTED + uuidString, permitted);

        // Add to list
        if(!prefs.contains(KEY_LIST)) {
            // First app!
            StringBuilder builder = new StringBuilder();
            builder.append(uuidString);
            ed.putString(KEY_LIST, builder.toString());
            ed.commit();

            //Log.d(TAG, "The first app: " + builder.toString());
            return;
        }

        // Already exist?
        String list = prefs.getString(KEY_LIST, "");
        if(list.contains(uuidString)) {
            //Log.d(TAG, "uuid " + uuidString + " is already in LIST " + list);
            ed.commit();
            return;
        }

        // Append
        list = list + LIST_SEP + uuidString;
        ed.putString(KEY_LIST, list);
        ed.commit();
        //Log.d(TAG, "LIST now " + list);
    }

    public static ArrayList<UUID> getList(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        ArrayList<UUID> list = new ArrayList<>();
        if(!prefs.contains(KEY_LIST)) {
            return list;
        }

        String listEncoded = prefs.getString(KEY_LIST, "");
        String[] items = listEncoded.split(";");
        for(int i = 0; i < items.length; i++) {
            //Log.d(TAG, "Read UUID " + items[i]);
            list.add(UUID.fromString(items[i]));
        }

        return list;
    }

}
