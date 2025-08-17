package background;

import android.content.Context;
import android.telephony.PhoneStateListener;
import android.telephony.SignalStrength;

import cl_toolkit.Logger;
import config.Runtime;

// Get the phone GSM signal strength, approximately
public class SignalListener extends PhoneStateListener {

    private static final String TAG = SignalListener.class.getName();

    private static final int
        UNKNOWN_CODE = 99,
        MAX_SIGNAL_DBM_VALUE = 31;

    private PSLCallback callback;
    private Context context;

    public SignalListener(Context context, PSLCallback callback) {
        this.callback = callback;
        this.context = context;
    }

    private int calculateSignalStrengthInPercent(int signalStrength) {
        Runtime.log(context, TAG, "Raw GSM strength: " + signalStrength + "/31", Logger.INFO);
        return (int) ((float) signalStrength / MAX_SIGNAL_DBM_VALUE * 100);
    }

    @Override
    public void onSignalStrengthsChanged(SignalStrength signalStrength) {
        super.onSignalStrengthsChanged(signalStrength);

        int perc;
        if(signalStrength != null && signalStrength.getGsmSignalStrength() != UNKNOWN_CODE) {
            perc = calculateSignalStrengthInPercent(signalStrength.getGsmSignalStrength());
            Runtime.log(context, TAG, "Phone signal strength percent = " + perc, Logger.INFO);
        } else {
            Runtime.log(context, TAG, "Unable to get phone signal strength!", Logger.INFO);
            perc = 0;
        }

        callback.onPercentKnown(perc);
    }
}
