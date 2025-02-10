package dash;

import android.telephony.PhoneStateListener;
import android.telephony.SignalStrength;
import android.util.Log;

public abstract class SignalListener extends PhoneStateListener {

    private static final String TAG = SignalListener.class.getName();

    private static final int
        UNKNOWN_CODE = 99,
        MAX_SIGNAL_DBM_VALUE = 31;

    private int calculateSignalStrengthInPercent(int signalStrength) {
        return Math.round(((float)signalStrength / (float)MAX_SIGNAL_DBM_VALUE) * 100);
    }

    @Override
    public void onSignalStrengthsChanged(SignalStrength signalStrength) {
        super.onSignalStrengthsChanged(signalStrength);

        int percent = 0;
        if(signalStrength != null && signalStrength.getGsmSignalStrength() != UNKNOWN_CODE) {
            percent = calculateSignalStrengthInPercent(signalStrength.getGsmSignalStrength());
        } else {
            Log.e(TAG, "Unable to get phone signal strength!");
        }

        onPercentKnown(percent);
    }

    public abstract void onPercentKnown(int percent);

}
