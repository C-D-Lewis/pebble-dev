package uk.me.chrislewis.dashboard2.features

import android.app.ActivityManager
import android.content.Context
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
     * Get free RAM percentage.
     */
    fun getPercentageFreeRam(context: Context): Int {
        val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val memoryInfo = ActivityManager.MemoryInfo()
        activityManager.getMemoryInfo(memoryInfo)
        return ((memoryInfo.availMem.toDouble() / memoryInfo.totalMem.toDouble()) * 100).toInt()
    }

    /**
     * Get free RAM in MB.
     */
    fun getFreeRamInMB(context: Context): Long {
        val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val memoryInfo = ActivityManager.MemoryInfo()
        activityManager.getMemoryInfo(memoryInfo)

        // Convert bytes to Megabytes (1 MB = 1024 * 1024 bytes)
        return memoryInfo.availMem / (1024 * 1024)
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