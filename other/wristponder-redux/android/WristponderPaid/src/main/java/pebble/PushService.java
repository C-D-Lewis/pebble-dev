package pebble;

import android.app.Activity;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.telephony.SmsManager;
import android.util.Log;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

import java.util.LinkedList;

import cl_toolkit.Contact;
import cl_toolkit.Logger;
import config.Build;
import config.Keys;
import config.Runtime;
import util.ContactBundle;
import util.ContactFileDatabase;
import util.FileDatabase;
import util.PermissionHelper;

public class PushService extends Service {
	
	//Constants
	public static  String TAG = PushService.class.getName();

    private boolean containsKey(PebbleDictionary dict, int key) {
        return dict.getInteger(key) != null;
    }

    private int namesIndexFromTypeAndIndex(int type, int index) {
        switch(type) {
            case Keys.ContactTypeSMS:
                return 0;
            case Keys.ContactTypeCall:
                return 1;
            case Keys.ContactTypeIncoming:
                return 2 + index;
            case Keys.ContactTypeFavorite:
                return 7 + index;
            default: return -1;
        }
    }

	private void processPacket(PebbleDictionary dict, SharedPreferences prefs) {
		// What type of packet?
        if(containsKey(dict, Keys.AppKeyPacketType)) {
            int packetType = dict.getInteger(Keys.AppKeyPacketType).intValue();
            switch(packetType) {
                case Keys.PacketTypeContactRequest: {
                    // Watch wants a contact
                    int type = dict.getInteger(Keys.AppKeyContactType).intValue();
                    int index = dict.getInteger(Keys.AppKeyContactIndex).intValue();

                    LinkedList<ContactBundle> names = loadNames(prefs);

                    PebbleDictionary out = new PebbleDictionary();
                    out.addInt32(Keys.AppKeyPacketType, Keys.PacketTypeContactRequest);
                    out.addInt32(Keys.AppKeyContactType, type);
                    switch(type) {
                        case Keys.ContactTypeSMSAndCall:
                            // Send both
                            int smsId = namesIndexFromTypeAndIndex(Keys.ContactTypeSMS, 0);
                            int callId = namesIndexFromTypeAndIndex(Keys.ContactTypeCall, 0);
                            out.addString(Keys.ContactTypeSMS, names.get(smsId).name);
                            out.addString(Keys.ContactTypeCall, names.get(callId).name);
                            break;
                        case Keys.ContactTypeFavorite:
                            // Send all favorites
                            // Send total
                            break;
                        case Keys.ContactTypeIncoming:
                            // Send all incoming
                            // Send total
                            break;
                        case Keys.ContactTypeQuery:
                            // Send all of this letter (up to 30)
                            // Send total
                            break;
                    }

                    // FIXME load up the dict with all appropriate contacts

                    sendDict(out);
                }   break;

                // Load all, send all responses
                case Keys.PacketTypeResponses: {
                    FileDatabase responseDb = loadResponseDb(prefs);
                    int total = responseDb.getArrayList().size();

                    PebbleDictionary out = new PebbleDictionary();
                    out.addInt32(Keys.AppKeyPacketType, Keys.PacketTypeResponses);
                    dict.addInt32(Keys.AppKeyTotalResponses, total);
                    for(int i = 0; i < total; i++) {
                        dict.addString(i, responseDb.getArrayList().get(i));
                    }

                    sendDict(out);
                }   break;

                // Watch wants to send a canned SMS
                case Keys.PacketTypeSMSRequest: {
                    FileDatabase responseDb = loadResponseDb(prefs);
                    LinkedList<ContactBundle> names = loadNames(prefs);

                    int contactType = dict.getInteger(Keys.AppKeyContactType).intValue();
                    int contactIndex = dict.getInteger(Keys.AppKeyContactIndex).intValue();
                    int responseIndex = dict.getInteger(Keys.AppKeyResponseIndex).intValue();

                    int dbIndex = namesIndexFromTypeAndIndex(contactType, contactIndex);
                    String number = names.get(dbIndex).number;
                    try {
                        String message = responseDb.getArrayList().get(responseIndex);
                        sendSMS(number, message);
                        publishToProvider(number, message);
                    } catch(Exception e) {
                        e.printStackTrace();
                    }
                }   break;

                // This is a new fangled transcription request
                case Keys.PacketTypeTranscriptSMS: {
                    LinkedList<ContactBundle> names = loadNames(prefs);

                    int contactType = dict.getInteger(Keys.AppKeyContactType).intValue();
                    int contactIndex = dict.getInteger(Keys.AppKeyContactIndex).intValue();
                    String response = dict.getString(Keys.AppKeyTranscriptText);

                    int dbIndex = namesIndexFromTypeAndIndex(contactType, contactIndex);
                    String number = names.get(dbIndex).number;
                    Runtime.log(getApplicationContext(), TAG, "Sending SMS with transcription '" + response + "' to " + number, Logger.INFO);

                    sendSMS(number, response);
                    publishToProvider(number, response);
                }   break;
            }
        }
	}
	
	/**
	 * Main abstraction
	 * @param incomingDict PebbleDictionary received
	 * @param prefs	SharedPreference object
	 */
	private void respond(PebbleDictionary incomingDict, SharedPreferences prefs) {
        //Version check?
        if(PebbleUtils.hasString(incomingDict, VersionCheck.KEY_VERSION_CHECK_VERSION)) {
            VersionCheck.check(getApplicationContext(), Build.WATCHAPP_UUID, incomingDict, Build.WATCHAPP_COMPATIBLE_VERSION_STRING);
        }

		// After version check, check we have permission
        else if(android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            if(!PermissionHelper.isPermitted(getApplicationContext())) {
                Runtime.log(getApplicationContext(), TAG, "Permissions not given!", Logger.ERROR);

                PebbleDictionary out = new PebbleDictionary();
                out.addInt32(Keys.PacketTypeError, Keys.AppKeyErrorNoPermissions);
                sendDict(out);
            } else {
                processPacket(incomingDict, prefs);
            }
		}
	}

	private void sendSMS(String thisSmsAddress, String message) {
		SmsManager sms = SmsManager.getDefault();
		sms.sendTextMessage(thisSmsAddress, null, message, createPendingIntent(), null);
	}

	private void publishToProvider(String thisSmsAddress, String message) {
		ContentValues values = new ContentValues();
		values.put("address", thisSmsAddress);
		values.put("body", message);
		getContentResolver().insert(Uri.parse("content://sms/sent"), values);
		Runtime.log(getApplicationContext(), TAG, "Message added to ContentProvider SMS", Logger.INFO);
		Runtime.log(getApplicationContext(), TAG, "Send sms to " + thisSmsAddress, Logger.INFO);
	}

	private PendingIntent createPendingIntent() {
		PendingIntent sendPi = PendingIntent.getBroadcast(getApplicationContext(), 0, new Intent("SENT"), 0);
		
		registerReceiver(new BroadcastReceiver() {
			
			@Override
			public void onReceive(Context context, Intent intent) {
				PebbleDictionary out = new PebbleDictionary();
                out.addInt32(Keys.AppKeyPacketType, Keys.PacketTypeSMSResult);

				switch(getResultCode()) {
				case Activity.RESULT_OK:
					//Send successful!
					out.addInt32(Keys.AppKeySMSResult, Keys.AppKeySMSResultSuccess);
					Runtime.log(getApplicationContext(), TAG, "Send success!", Logger.INFO);
					
					//Handle autoclose
					checkAutoClose();
					break;
				default:
					//Failure
                    out.addInt32(Keys.AppKeySMSResult, Keys.AppKeySMSResultFailed);
					Runtime.log(getApplicationContext(), TAG, "Error sending message: " + getResultCode(), Logger.INFO);
					break;
				}
				
				sendDict(out);

                // Self unregister
                try {
                    unregisterReceiver(this);
                } catch(Exception e) {
                    Runtime.log(getApplicationContext(), TAG, "Error unregistering SMS sent receiver: " + e.getLocalizedMessage(), Logger.ERROR);
                    e.printStackTrace();
                }
			}
			
		}, new IntentFilter("SENT"));
		return sendPi;
	}
	
	/**
	 * Construct array of 'Bundles that match ordering on Watchapp for ease of use
	 * @return LinkedList of NameAndNumberBundles in correct order
	 */
	private LinkedList<ContactBundle> loadNames(SharedPreferences prefs) {
		LinkedList<ContactBundle> names = new LinkedList<ContactBundle>();
		
		//Last SMS
		String smsName = Contact.getLastSMSName(this, Build.MAX_NAME_LENGTH);
		String smsAddress = Contact.getLastSMSNumber(this);
		names.add(new ContactBundle(smsName, smsAddress));
		
		//Last Caller
		String callerName = getLastCallerName();
		String callerAddress = getLastCallerAddress();
		names.add(new ContactBundle(callerName, callerAddress));
		
		//Top incoming
		ContactFileDatabase incomingDb = new ContactFileDatabase(getApplicationContext(), Build.INCOMING_DATABASE_NAME);
		String[] topNumbers = incomingDb.getTopFive();
		String name = null;
		for(int i = 0; i < Build.MAX_FAV_OR_INCOMING_NAMES; i++) {
			//If it exists
			if(topNumbers[i] != null) {
                // Look it up
				name = Contact.getContactName(getApplicationContext(), topNumbers[i], Build.MAX_NAME_LENGTH);
			} else {
                // Mark as unknown
				name = "Unknown";
			}

			//Add
			names.add(new ContactBundle(name, topNumbers[i]));
		}
		
		//Favourites
		for(int i = 0; i < Build.MAX_FAV_OR_INCOMING_NAMES; i++) {
			String favName = Contact.getContactName(getApplicationContext(), prefs.getString(Build.PREFIX_FAVOURITE_NUMBER + "" + i, "Unknown"), Build.MAX_NAME_LENGTH);
			names.add(new ContactBundle(favName, prefs.getString(Build.PREFIX_FAVOURITE_NUMBER + "" + i, "Unknown")));
		}
		
		//Read out
		for(int i = 0; i < Build.TOTAL_NAMES; i++) {
			Runtime.log(getApplicationContext(), TAG, "Name/Number " + i + ": " + names.get(i).name + "/" + names.get(i).number, Logger.INFO);
		}
		
		// Finally
		return names;
	}
	
	private int checksum(String args[]) {
		int result = 0;
		
		for(String s : args) {
			char arr[] = s.toCharArray();
			for(int i = 0; i < s.length(); i++) {
				result += arr[i];
			}
		}

        Runtime.log(getApplicationContext(), TAG, "Generated sync checksum: " + result, Logger.INFO);
		return result;
	}
	
	private void checkAutoClose() {
		if(PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getBoolean(Keys.PREF_AUTOCLOSE, false)) {
			new Thread(new Runnable() {
				
				@Override
				public void run() {
					try {
						Thread.sleep(3000);
						PebbleKit.closeAppOnPebble(getApplicationContext(), Build.WATCHAPP_UUID);

                        Runtime.log(getApplicationContext(), TAG, "Autoclosing watchapp...", Logger.DEBUG);
					} catch (Exception e) {
						Log.e(TAG, "Error sending close command.");
						e.printStackTrace();
					}
				}
				
			}).start();
		} else {
            Runtime.log(getApplicationContext(), TAG, "Not autocolosing watchapp.", Logger.DEBUG);
        }
	}
	
	private String getLastCallerName() {
		//Get from when stored by listening receiver
		String num = PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getString(Keys.PREF_CALLER_NAME, null);
        String name;
        if(num != null) {
            // Designed to never be null
            name = Contact.getContactName(getApplicationContext(), num, Build.MAX_NAME_LENGTH);
        } else {
            Runtime.log(getApplicationContext(), TAG, "getLastCallerName pref returned null. No last caller yet?", Logger.INFO);
            name = "Unknown";
        }

		return name;
	}
	
	private String getLastCallerAddress() {
		//Get from when stored by listening receiver
		String name = PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getString(Keys.PREF_CALLER_NAME, null);

        Runtime.log(getApplicationContext(), TAG, "getLastCallerAddress pref returned null. No last caller yet?", Logger.INFO);
        return name != null ? name : "Unknown";
	}

	private FileDatabase loadResponseDb(SharedPreferences prefs) {
		//Load Response DB
		String sortModePref = prefs.getString(Keys.PREF_SORT_STRING, FileDatabase.SORT_ALPHABETIC_ASC);
		return new FileDatabase(getApplicationContext(), Build.RESPONSE_DATABASE_NAME, sortModePref);
	}

	private void sendDict(PebbleDictionary dict) {
        if(Build.DEBUG) {
            Runtime.log(getApplicationContext(), TAG, "Outgoing: " + dict.toJsonString(), Logger.DEBUG);
        }
		PebbleKit.sendDataToPebble(getApplicationContext(), Build.WATCHAPP_UUID, dict);
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		try {
			//Sanity checks
			if(intent != null) {
				Runtime.log(getApplicationContext(), TAG, "PebblePushService awake!", Logger.INFO);
				SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());

				//Get JSON
				String jsonData = intent.getExtras().getString("json");
				PebbleDictionary dict = PebbleDictionary.fromJson(jsonData);

                // Don't print the protocol!
                if(Build.DEBUG) {
                    Runtime.log(getApplicationContext(), TAG, "Incoming dict: " + jsonData, Logger.INFO);
                }

				// Issue response
				respond(dict, prefs);
			}
			
			Runtime.log(getApplicationContext(), TAG, "PebblePushService finished!", Logger.INFO);
		} catch(Exception e) {
			Runtime.log(getApplicationContext(), TAG, "Push Exception: " + e.getLocalizedMessage(), Logger.ERROR);
			e.printStackTrace();
		}
		
		return super.onStartCommand(intent, flags, startId);
	}

	@Override
	public IBinder onBind(Intent i) { return null; }
	
}
