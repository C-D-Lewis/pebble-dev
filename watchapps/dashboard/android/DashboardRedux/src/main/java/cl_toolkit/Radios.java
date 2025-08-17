package cl_toolkit;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Handler;
import androidx.annotation.RequiresApi;
import androidx.core.content.ContextCompat;
import android.util.Log;

import java.lang.reflect.Method;

/**
 * Methods for working with device radios
 *
 * An exercise in relying on maaany reflected APIs that could vanish at the drop of a hat
 * > Don't do this.
 *
 * @author Chris Lewis
 */
public class Radios {
	
	public static final int 
		WIFI_AP_STATE_UNKNOWN = -1,
		WIFI_AP_STATE_DISABLING = 0,
		WIFI_AP_STATE_DISABLED = 1,
		WIFI_AP_STATE_ENABLING = 2,
		WIFI_AP_STATE_ENABLED = 3,
		WIFI_AP_STATE_FAILED = 4;
	
	private static final String TAG = Radios.class.getName();

	/**
	 * Get the current WiFi state
	 * @return	true if WiFi is enabled
	 */
	public static boolean getWiFiEnabled(Context context) {
		return ((WifiManager)context.getSystemService(Context.WIFI_SERVICE)).isWifiEnabled();
	}
	
	/**
	 * Set the WiFi state
	 * @param newState	Desired new state
	 */
	public static boolean setWiFiState(Context context, boolean newState) {
		return ((WifiManager)context.getSystemService(Context.WIFI_SERVICE)).setWifiEnabled(newState);
	}
	
	/**
	 * Get the current data network state
	 * @return	true if the network radio is connected or connecting
	 */
	public static boolean getMobileDataEnabled(Context context) {
		NetworkInfo nInfo = ((ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE))
				.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
		return nInfo.getState() == NetworkInfo.State.CONNECTED 
		    || nInfo.getState() == NetworkInfo.State.CONNECTING;
	}
	
	/**
	 * Set new Mobile Data state
	 * 
	 * Adapted from source: 
	 * http://stackoverflow.com/questions/12535101/how-can-i-turn-off-3g-data-programmatically-on-android
	 * 
	 * @param newState	The new state
	 */
	public static boolean setMobileDataEnabled(Context context, boolean newState) {
        try {
            ConnectivityManager dataManager = (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
            Method dataMtd = ConnectivityManager.class.getDeclaredMethod("setMobileDataEnabled", boolean.class);

            dataMtd.setAccessible(true);
            dataMtd.invoke(dataManager, newState);
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
	}

    public static boolean setMobileDataEnabledL(Context context, boolean newState) {
        try {
            // root required ???
            return true;
        } catch(Exception e) {
            e.printStackTrace();
            return false;
        }
    }
	
	/**
	 * Get current BT state
	 * @return	true if Bluetooth is enabled
	 */
	public static boolean getBluetoothState() {
		return BluetoothAdapter.getDefaultAdapter().isEnabled();
	}
	
	/**
	 * Set the current BT state
	 * @param newState	The desired state
	 */
	public static boolean setBluetoothState(boolean newState) {
		BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
		
		if(newState) {
			return adapter.enable();
		} else {
			return adapter.disable();
		}
	}
	
	/**
	 * Set the WiFi Access Point mode state
	 */
	public static boolean setWifiApEnabled(Context context, boolean newState) {
		try {
			WifiManager manager = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
			

			if (android.os.Build.VERSION.SDK_INT > android.os.Build.VERSION_CODES.O) {
				if (ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED)
				{
					return setHotspot(manager, newState);
				} else {
					return false;
				}
			} else {
				if(newState) {
					//Disable WiFi first
					manager.setWifiEnabled(false);
				}

				Method method = manager.getClass().getMethod("setWifiApEnabled", WifiConfiguration.class, boolean.class);
				return (Boolean) method.invoke(manager, getWifiApConfiguration(context), newState);
			}
		} catch (Exception e) {
			Log.e(TAG, "Error setting Wifi AP state");
			e.printStackTrace();
			return false;
		}
	}

	/**
	 * Get the WiFi AP state
	 */
	private static int getWifiApState(Context context) {
		try {
			WifiManager manager = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
			Method method = manager.getClass().getMethod("getWifiApState");

			int stateNow = (Integer)method.invoke(manager);
			if(stateNow > 10) {
				stateNow -= 10; // Changes in Android 4.0 for some strange reason
            }

			return stateNow;
		} catch (Exception e) {
			Log.e(TAG, "Error getting Wifi AP state");
			e.printStackTrace();
			return -1;
		}
	}
	
	/**
	 * Simply get ON or OFF state
	 * If in doubt, use getWifiApState and check int value
	 */
	public static boolean getWifiApEnabled(Context context) {
		int state = getWifiApState(context);
		if(state == WIFI_AP_STATE_ENABLED || state == WIFI_AP_STATE_ENABLING) {
			return true;
		} else if(state == WIFI_AP_STATE_DISABLED || state == WIFI_AP_STATE_DISABLING) {
			return false;
		} else {
			Log.e(TAG, "Wifi AP error state: " + state);
			return false;
		}
	}

	/**
	 * Get the Wifi AP Configuration
	 */
	private static WifiConfiguration getWifiApConfiguration(Context context) {
		try {
			WifiManager manager = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
			Method method = manager.getClass().getMethod("getWifiApConfiguration");
			return (WifiConfiguration)method.invoke(manager);
		} catch (Exception e) {
			Log.e(TAG, "Error getting Wifi AP Configuration");
			e.printStackTrace();
			return null;
		}
	}

	@RequiresApi(android.os.Build.VERSION_CODES.O)
	private static boolean setHotspot(WifiManager wifiManager, boolean state) {
		if (state) {
			wifiManager.startLocalOnlyHotspot(new WifiManager.LocalOnlyHotspotCallback() {
				@Override
				public void onStarted(WifiManager.LocalOnlyHotspotReservation reservation) {
					super.onStarted(reservation);
					Log.v(TAG, "WiFi AP Started");
				}

				@Override
				public void onStopped() {
					super.onStopped();
					Log.v(TAG, "WiFi AP Stopped");
				}

				@Override
				public void onFailed(int reason) {
					super.onFailed(reason);
					Log.v(TAG, "WiFi AP failed to start");
				}
			}, new Handler());
		} else {
			try {
				Method method = wifiManager.getClass().getDeclaredMethod("cancelLocalOnlyHotspotRequest");
				method.invoke(wifiManager);
			} catch (Exception e) {
				Log.v(TAG, "WiFi AP failed to stop");
			}
		}
		return true;
	}
}
