package uk.me.chrislewis.dashboard2.features

import android.content.Context
import android.media.AudioAttributes
import android.media.Ringtone
import android.media.RingtoneManager

object FindPhone {
    private var ringtone: Ringtone? = null

    private fun playDefaultAlarmSound(context: Context) {
        // Get the default ALARM sound URI
        val alarmUri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_ALARM)
            ?: RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION)

        ringtone = RingtoneManager.getRingtone(context, alarmUri)?.apply {
            // Direct audio to the ALARM stream (bypasses Silent mode on most devices)
            audioAttributes = AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_ALARM)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build()
            isLooping = true
            play()
        }
    }

    fun handleFindPhone(context: Context) {
        if (ringtone == null || !ringtone?.isPlaying!!) {
            playDefaultAlarmSound(context)
        } else {
            ringtone?.stop()
        }
    }

    fun stopPlaying() {
        ringtone?.stop()
    }

    fun isPlaying(): Boolean {
        if (ringtone == null) return false;
        return ringtone?.isPlaying!!
    }
}