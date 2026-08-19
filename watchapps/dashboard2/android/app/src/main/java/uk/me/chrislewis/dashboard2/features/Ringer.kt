package uk.me.chrislewis.dashboard2.features

import android.app.NotificationManager
import android.content.Context
import android.content.Intent
import android.media.AudioManager
import android.provider.Settings
import android.util.Log
import uk.me.chrislewis.dashboard2.Constants

private const val TAG = "Ringer"

object Ringer {
    fun checkNotificationPolicyAccess(context: Context): Boolean {
        val notificationManager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        val granted = notificationManager.isNotificationPolicyAccessGranted
        Log.d(TAG, "Ringer notif policy access granted: $granted")
        return granted
    }

    fun requestNotificationPolicyAccess(context: Context) {
        // Access not granted - direct the user to the system settings page
        val intent = Intent(Settings.ACTION_NOTIFICATION_POLICY_ACCESS_SETTINGS)
        context.startActivity(intent)
    }

    /**
     * Cycle the ringer mode
     */
    fun cycleRingerMode(context: Context) {
        // Check access is granted
        if (!checkNotificationPolicyAccess(context)) {
            Log.e(TAG, "Tried to cycle ringer mode but missing permission")
            return
        }

        val audioManager = context.getSystemService(Context.AUDIO_SERVICE) as AudioManager
        val currentMode = audioManager.ringerMode

        val nextMode = when (currentMode) {
            AudioManager.RINGER_MODE_NORMAL -> AudioManager.RINGER_MODE_VIBRATE
            AudioManager.RINGER_MODE_VIBRATE -> AudioManager.RINGER_MODE_SILENT
            AudioManager.RINGER_MODE_SILENT -> AudioManager.RINGER_MODE_NORMAL
            else -> AudioManager.RINGER_MODE_NORMAL
        }
        Log.d(TAG, "Current ringer mode $currentMode, next is $nextMode")
        audioManager.ringerMode = nextMode
    }

    fun getRingerModeProtocolValue(context: Context): String {
        if (!checkNotificationPolicyAccess(context)) {
            Log.e(TAG, "Tried to get ringer mode but missing permission")
            return Constants.TOGGLE_STATE_RINGER_LOUD
        }

        val audioManager = context.getSystemService(Context.AUDIO_SERVICE) as AudioManager
        val currentMode = audioManager.ringerMode

        return when (currentMode) {
            AudioManager.RINGER_MODE_NORMAL -> Constants.TOGGLE_STATE_RINGER_LOUD
            AudioManager.RINGER_MODE_VIBRATE -> Constants.TOGGLE_STATE_RINGER_VIBRATE
            AudioManager.RINGER_MODE_SILENT -> Constants.TOGGLE_STATE_RINGER_SILENT
            else -> Constants.TOGGLE_STATE_RINGER_LOUD
        }
    }
}