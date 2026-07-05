package uk.me.chrislewis.dashboard2

import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.BatteryManager
import android.os.Environment
import android.os.StatFs
import android.provider.Settings
import android.text.format.Formatter
import kotlin.math.roundToInt

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

    /**
     * Get readable free space.
     */
    fun getFreeDiskSpace(context: Context): String {
        val path = Environment.getDataDirectory()
        val stat = StatFs(path.path)
        val freeBytes = stat.availableBlocksLong * stat.blockSizeLong
        return Formatter.formatFileSize(context, freeBytes)
    }

    /**
     * Get free disk space percentage.
     *
     * TODO: Some DRY with above function.
     */
    fun getUsedDiskSpacePercentage(): Int {
        val path = Environment.getDataDirectory()
        val stat = StatFs(path.path)
        val blockSize = stat.blockSizeLong
        val totalBytes = stat.blockCountLong * blockSize
        if (totalBytes <= 0) return 0

        // Get free percentage
        val availableBytes = stat.availableBlocksLong * blockSize
        val freePercentage = (availableBytes.toDouble() / totalBytes.toDouble()) * 100
        val usedPercentage = 100 - freePercentage

        return usedPercentage.roundToInt()
    }
}