package phone;

import util.ContactFileDatabase;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.telephony.SmsMessage;

import com.getpebble.android.kit.PebbleKit;

import config.Build;
import config.Keys;

public class SMSReceiver extends BroadcastReceiver {

	private static final String ACTION = "android.provider.Telephony.SMS_RECEIVED";

	@Override
	public void onReceive(Context context, Intent intent) {
		if (intent.getAction().equals(ACTION)) {
			//Add to soutgoing db count - K: smsAddress, V: count
			final ContactFileDatabase outgoingDb = new ContactFileDatabase(context, Build.INCOMING_DATABASE_NAME);

			/* The SMS-Messages are 'hiding' within the extras of the Intent. */
			Bundle bundle = intent.getExtras();
			if (bundle != null) {
				//Get messages
				//                 SmsMessage[] messages = android.telephony.gsm.SmsMessage.getMessagesFromIntent(intent);

				Object[] pdusObj = (Object[]) bundle.get("pdus"); 
				if ( pdusObj==null ) 
				{ 
					System.out.println("Message pdu's are null!"); 
					return; 
				} 
				SmsMessage[] messages = new SmsMessage[pdusObj.length]; 
				System.out.println("retrieving: "+pdusObj.length+" messages."); 
				for (int i = 0; i<pdusObj.length; i++) 
				{ 
					// create the message from the pdu 
					messages[i] = SmsMessage.createFromPdu ((byte[]) pdusObj[i]); 
					System.out.println("Message from: "+messages[i].getOriginatingAddress()); 

					for (SmsMessage currentMessage : messages) {
						if(currentMessage != null) {
							String smsAddress = currentMessage.getOriginatingAddress();
	
							//Query DB
							if(outgoingDb.contains(smsAddress)) {
								int currentValue = Integer.parseInt(outgoingDb.get(smsAddress));
								currentValue++;
								outgoingDb.put(smsAddress, "" + currentValue);
							} else {
								//Add a new entry
								outgoingDb.put(smsAddress, "" + 1);
							}
							
							//Auto start?
							SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
							
							boolean autoStart = prefs.getBoolean(Keys.PREF_AUTOSTART, false);
							if(autoStart) {
								PebbleKit.startAppOnPebble(context, Build.WATCHAPP_UUID);
							}
						}
					}
				}
			}
		}
	}

}
