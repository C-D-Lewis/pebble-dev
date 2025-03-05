package util;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.support.v4.content.ContextCompat;

@TargetApi(23)
public class PermissionHelper {

    private static String[] PERMISSIONS = new String[]{
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_SMS,
            Manifest.permission.READ_CONTACTS,
            Manifest.permission.SEND_SMS,
            Manifest.permission.READ_PHONE_STATE,
            Manifest.permission.RECEIVE_SMS,
            "android.permission.WRITE_SMS",
            Manifest.permission.INTERNET
    };

    public static void ask(Activity parent) {
        parent.requestPermissions(PERMISSIONS, 0);
    }

    public static boolean isPermitted(Context context) {
        for(String permission : PERMISSIONS) {
            if(ContextCompat.checkSelfPermission(context, permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }
}
