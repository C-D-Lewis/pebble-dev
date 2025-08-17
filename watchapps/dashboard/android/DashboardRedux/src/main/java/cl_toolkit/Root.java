package cl_toolkit;

import android.content.Context;
import android.util.Log;

import java.io.DataOutputStream;
import java.lang.reflect.Field;

public class Root {

    public static final String TAG = Root.class.getName();

    public static final String
        CMD_DATA_ENABLE = "settings put global mobile_data 1",
        CMD_DATA_DISABLE = "settings put global mobile_data 0";

    /**
     * Big thanks:
     * http://stackoverflow.com/a/27011670
     */
    public static boolean rootSetMobileDataEnabled(Context context, boolean newState) {
        try {
            Process process = java.lang.Runtime.getRuntime().exec("su");
            DataOutputStream out = new DataOutputStream(process.getOutputStream());

            // The magic recipe!
            String cmd = "service call phone ";
            int rootInt = getTransactionCode("com.android.internal.telephony.ITelephony", "setDataEnabled");
            cmd += rootInt;
            if(newState) {
                cmd += " i32 1";
            } else {
                cmd += " i32 0";
            }

            Log.d(TAG, "Root int: " + rootInt);

            out.writeBytes(cmd + "\n");
            out.writeBytes("exit\n");
            out.flush();
            int retVal = -99;   // Remember this is uninit value!
            synchronized (process) {
                retVal = process.waitFor();
            }

            Log.d(TAG, "Root mobileDataEnabled exited code " + retVal);

            if(retVal == 1) {
                // User must grant root permissions?
                return false;
            }

            return true;
        } catch(Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    /**
     * Thanks to Cygery from XDA
     * @param baseClassName
     * @param targetFieldName
     * @return
     * @throws Exception
     */
    public static int getTransactionCode(String baseClassName, String targetFieldName) throws Exception {
        Class classToInvestigate = Class.forName(baseClassName);
        Class[] innerClasses = classToInvestigate.getDeclaredClasses();
        for (Class c : innerClasses) {
            Field[] aClassFields = c.getDeclaredFields();
            for (Field f : aClassFields) {
                String fieldName = f.getName();
                if (fieldName != null && fieldName.equals("TRANSACTION_" + targetFieldName)) {
                    f.setAccessible(true); // that's important
                    return f.getInt(f);
                }
            }
        }

        // Not found
        throw new Exception("TRANSACTION Field not found for query { " + baseClassName + ", " + targetFieldName + " }");
    }

}
