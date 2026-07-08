package uk.me.chrislewis.dashboard2.features

import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.BatteryManager

object Battery {
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
