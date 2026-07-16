package uk.me.chrislewis.dashboard2.features

import android.app.AlarmManager
import android.content.Context
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

object Alarms {
    fun getNextAlarmTime(context: Context): String {
        val alarmManager = context.getSystemService(Context.ALARM_SERVICE) as AlarmManager
        val nextAlarm = alarmManager.nextAlarmClock ?: return "-"
        val triggerTimeMs = nextAlarm.triggerTime
        val date = Date(triggerTimeMs)
        val formatter = SimpleDateFormat("EE HH:mm", Locale.getDefault())
        return formatter.format(date)
    }
}
