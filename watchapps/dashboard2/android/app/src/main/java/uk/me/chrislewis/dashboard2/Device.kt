package uk.me.chrislewis.dashboard2

import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.BatteryManager
import android.provider.Settings

object Device {
    /**
     * Get device name or else Bluetooth name.
     */
    fun getDeviceName(context: Context): String {
        return Settings.Global.getString(context.contentResolver, "device_name")
            ?: Settings.Secure.getString(context.contentResolver, "bluetooth_name")
            ?: "Android Device"
    }

    /**
     * Get battery level and scale.
     */
    fun getBatteryLevel(context: Context): Int {
        val status: Intent? = IntentFilter(Intent.ACTION_BATTERY_CHANGED).let { filter ->
            context.registerReceiver(null, filter)
        }
        val level: Int = status?.getIntExtra(BatteryManager.EXTRA_LEVEL, -1) ?: -1
        val scale: Int = status?.getIntExtra(BatteryManager.EXTRA_SCALE, -1) ?: -1

        return if (level != -1 && scale != -1) {
            (level.toFloat() / scale.toFloat() * 100).toInt()
        } else {
            -1 // Indicates an error or unknown state
        }
    }
}