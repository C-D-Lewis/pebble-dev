package uk.me.chrislewis.dashboard2.features

import android.content.ContentResolver

object AutoSync {
    fun isAutoSyncEnabled(): Boolean {
        return ContentResolver.getMasterSyncAutomatically()
    }

    fun toggleAutoSync(enable: Boolean) {
        try {
            ContentResolver.setMasterSyncAutomatically(enable)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
}
