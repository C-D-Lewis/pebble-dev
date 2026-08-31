package uk.me.chrislewis.dashboard2.features

import android.content.Context
import android.os.Environment
import android.os.StatFs
import android.provider.Settings
import java.util.Locale
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
     * Get readable free space.
     */
    fun getFreeDiskSpace(): String {
        val path = Environment.getDataDirectory()
        val stat = StatFs(path.path)
        val freeBytes = stat.availableBlocksLong * stat.blockSizeLong
        val freeGb = freeBytes.toDouble() / (1024 * 1024 * 1024)
        return String.format(Locale.getDefault(), "%.1f GB", freeGb)
    }

    /**
     * Get free disk space percentage.
     *
     * TODO: Some DRY with above function.
     */
    fun getFreeDiskSpacePercentage(): Int {
        val path = Environment.getDataDirectory()
        val stat = StatFs(path.path)
        val blockSize = stat.blockSizeLong
        val totalBytes = stat.blockCountLong * blockSize
        if (totalBytes <= 0) return 0

        // Get free percentage
        val availableBytes = stat.availableBlocksLong * blockSize
        val freePercentage = (availableBytes.toDouble() / totalBytes.toDouble()) * 100

        return freePercentage.roundToInt()
    }
}