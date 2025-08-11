package data;

import android.util.Log;

public class Meta {

    private static final String TAG = Meta.class.getName();

    private static final int
        VERSION_MAJOR = 1,
        VERSION_MINOR = 7;  // 1.7

    public static boolean isRemoteCompatible(String remoteVersion) {
        int remoteMajor = Integer.parseInt(remoteVersion.substring(0, remoteVersion.indexOf('.')));
        int remoteMinor = Integer.parseInt(remoteVersion.substring(remoteVersion.indexOf('.') + 1));

        // Very old
        if(remoteMajor != VERSION_MAJOR) {
            Log.e(TAG, "Client is very old (major not equal to local)");
            return false;
        }

        // Compare minors (1.1 client is compatible with 1.2 server)
        boolean minorCompatible = (remoteMinor <= VERSION_MINOR);
        if(!minorCompatible) {
            Log.e(TAG, "Client is slightly old (minor not less than or equal to local)");
        }
        return minorCompatible;
    }

    public static String getVersionString() {
        return "" + VERSION_MAJOR + "." + VERSION_MINOR;
    }

}
