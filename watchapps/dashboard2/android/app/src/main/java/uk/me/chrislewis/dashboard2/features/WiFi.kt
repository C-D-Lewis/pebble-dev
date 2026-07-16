package uk.me.chrislewis.dashboard2.features

import android.content.Context
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.net.wifi.WifiInfo
import android.net.wifi.WifiManager

object WiFi {
    /**
     * Get Wi-Fi network name.
     */
    fun getWifiSSID(context: Context): String {
        val connectivityManager = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        val activeNetwork = connectivityManager.activeNetwork ?: return "Not connected to any network"
        val capabilities = connectivityManager.getNetworkCapabilities(activeNetwork) ?: return "No network capabilities"

        if (capabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)) {
            val wifiInfo = capabilities.transportInfo as? WifiInfo

            if (wifiInfo != null) {
                val ssid = wifiInfo.ssid

                // If permissions are missing or GPS is off, Android returns "<unknown ssid>"
                if (ssid == WifiManager.UNKNOWN_SSID) {
                    return "?"
                }

                // Clean up the enclosing quotation marks from the returned string
                return ssid.replace("\"", "")
            }
        }

        return "-"
    }
}