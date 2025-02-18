package pebble;


import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.UUID;

import org.json.JSONException;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.os.IBinder;
import android.util.Log;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

public class PebbleService extends Service {
	//Configuration
	private final String TAG = PebbleService.class.getName();
	private static final UUID APP_UUID = UUID.fromString("23b109a6-4bd1-4e3d-90b5-92f4155cdb86");
	
	//Keys
	final private int
		KEY_MSG = 0,
		MSG_QUERY = 1,
		
		KEY_RADIO = 2,
		RADIO_WIFI = 3,
		RADIO_DATA = 4,
		  
		STATUS_ON = 5,
		STATUS_OFF = 6;	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		if(intent != null) {
			Log.d(TAG, "Service awake!");
			
			//Get JSON
			String jsonData = intent.getExtras().getString("json");
			
			try {
                final PebbleDictionary data = PebbleDictionary.fromJson(jsonData);
                
                //Collect data
                WifiManager wfManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
                NetworkInfo nInfo = ((ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE))
                						.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
                boolean wifi = wfManager.isWifiEnabled();
                boolean network = nInfo.getState() == NetworkInfo.State.CONNECTED 
                					|| nInfo.getState() == NetworkInfo.State.CONNECTING;
                
                //Is it a query?
                if(data.getUnsignedInteger(KEY_MSG) != null) {
	                int query = data.getUnsignedInteger(KEY_MSG).intValue();
	                if(query == MSG_QUERY) {
	                	Log.d(TAG, "Query received");
	                	
	                	//Send response to query
	                    PebbleDictionary dict = new PebbleDictionary();
	                    dict.addInt32(RADIO_WIFI, wifi ? STATUS_ON : STATUS_OFF);
	                    dict.addInt32(RADIO_DATA, network ? STATUS_ON : STATUS_OFF);
	                    PebbleKit.sendDataToPebble(getApplicationContext(), APP_UUID, dict);
	                    
	                    Log.d(TAG, "Sent query response Wi-Fi: " + wifi + " & Network: " + network);
	                }
                }
            
                //Get radio
                if(data.getUnsignedInteger(KEY_RADIO) != null) {
	                int radio = data.getUnsignedInteger(KEY_RADIO).intValue();
				
	                //Needed for both for full feedback
	                int wifiState = wfManager.getWifiState();

	                //Do the toggling
	                switch(radio) {
		                case RADIO_WIFI: {
		                	Log.d(TAG, "Wi-Fi received");
		                	
		            		switch(wifiState) {
		            			//It's off, turn it on
		                		case WifiManager.WIFI_STATE_DISABLED: {
		                			wfManager.setWifiEnabled(true);
		                			
		                			//Respond
		                			PebbleDictionary dict = new PebbleDictionary();
	                				dict.addInt32(RADIO_WIFI, STATUS_ON);
	                				dict.addInt32(RADIO_DATA, network ? STATUS_ON : STATUS_OFF);
									PebbleKit.sendDataToPebble(getApplicationContext(), APP_UUID, dict);
									
									Log.d(TAG, "Wifi now enabled sent.");
		                			break;		                			
		                		}
		            		
		                		//It's on, turn it off
		                		case WifiManager.WIFI_STATE_ENABLED: {
			            			wfManager.setWifiEnabled(false);
			            			
			            			//Respond
			            			PebbleDictionary dict = new PebbleDictionary();
		            				dict.addInt32(RADIO_WIFI, STATUS_OFF);
		            				dict.addInt32(RADIO_DATA, network ? STATUS_ON : STATUS_OFF);
									PebbleKit.sendDataToPebble(getApplicationContext(), APP_UUID, dict);
			            			
									Log.d(TAG, "Wifi now disabled sent.");
			            			break;
		                		}
		            		}
		            		
		                	break;
		                }
		                case RADIO_DATA: {
		                	Log.d(TAG, "Data received");
		                	
		            		//It's off, turn it on
		                	if(!network) {
	                			//Turn on
		                		setMobileDataEnabled(getApplicationContext(), true);
	                			
		                		//Respond
	                			PebbleDictionary dict = new PebbleDictionary();
                				dict.addInt32(RADIO_DATA, STATUS_ON);
                				dict.addInt32(RADIO_WIFI, wifiState == WifiManager.WIFI_STATE_ENABLED ? STATUS_ON : STATUS_OFF);
								PebbleKit.sendDataToPebble(getApplicationContext(), APP_UUID, dict);
								
								Log.d(TAG, "Data now enabled sent.");
	                			
                			//It's on, turn it off
		                	} else {
		                		setMobileDataEnabled(getApplicationContext(), false);
			            			
		                		//Respond
		            			PebbleDictionary dict = new PebbleDictionary();
	            				dict.addInt32(RADIO_DATA, STATUS_OFF);
	            				dict.addInt32(RADIO_WIFI, wifiState == WifiManager.WIFI_STATE_ENABLED ? STATUS_ON : STATUS_OFF);
								PebbleKit.sendDataToPebble(getApplicationContext(), APP_UUID, dict);
		            			
								Log.d(TAG, "Data now disabled sent");
	                		}
		                	break;
		                }
		                default: {
		                	Log.d(TAG, "Neither radio recieved: " + radio);
		                	break;
		                }
	                }
                }
			} catch (JSONException e) {
                e.printStackTrace();
            }
		} else {
			Log.d(TAG, "Intent is null!");
		}
		
		Log.d(TAG, "Service finished.");
		
		return super.onStartCommand(intent, flags, startId);
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}
	
	/**
	 * Source: http://stackoverflow.com/questions/12535101/how-can-i-turn-off-3g-data-programmatically-on-android
	 */
	@SuppressWarnings({ "rawtypes", "unchecked" })
	private void setMobileDataEnabled(Context context, boolean enabled) {
		try {
			final ConnectivityManager conman = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
			Class conmanClass;
			conmanClass = Class.forName(conman.getClass().getName());
			final Field iConnectivityManagerField = conmanClass.getDeclaredField("mService");
			iConnectivityManagerField.setAccessible(true);
			final Object iConnectivityManager = iConnectivityManagerField.get(conman);
			final Class iConnectivityManagerClass = Class.forName(iConnectivityManager.getClass().getName());
			final Method setMobileDataEnabledMethod = iConnectivityManagerClass.getDeclaredMethod("setMobileDataEnabled", Boolean.TYPE);
			setMobileDataEnabledMethod.setAccessible(true);

			setMobileDataEnabledMethod.invoke(iConnectivityManager, enabled);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

}
