package uk.me.chrislewis.dashboard2

import java.util.UUID

object Constants {
    // Watchapp UUID
    val APP_UUID: UUID = UUID.fromString("7606dabf-67cf-466a-92a7-5e62598a1436")

    // FIXME: Update these once watchapp is published
    val REBBLE_APPSTORE_URL = "https://apps.rebble.io/en_US/application/53ec8d840c3036447e000109?query=dashboard2&section=watchapps"
    val REPEBBLE_APPSTORE_URL = "apps.repebble.com/dashboard_53ec8d840c3036447e000109"

    // Compatible watchapp protocol version minimum
    // IMPORTANT NOTE: Increment when protocol has breaking changes!
    const val COMPATIBLE_PROTOCOL_VERSION = 3

    // AppMessage keys (same order as package.json)
    const val MESSAGE_KEY_COMPAT_PROTOCOL_VERSION = 10000u
    const val MESSAGE_KEY_TOGGLE_TYPE = 10001u
    const val MESSAGE_KEY_SYNC_REQUEST = 10002u
    const val MESSAGE_KEY_SYNC_TOGGLE_ORDER = 10003u
    const val MESSAGE_KEY_SYNC_DEVICE_NAME = 10004u
    const val MESSAGE_KEY_SYNC_BATTERY_PERC = 10005u
    const val MESSAGE_KEY_SYNC_FREE_SPACE = 10006u
    const val MESSAGE_KEY_SYNC_FREE_SPACE_PERC = 10007u
    const val MESSAGE_KEY_SYNC_NEXT_ALARM = 10008u
    const val MESSAGE_KEY_SYNC_IS_CHARGING = 10009u

    // ToggleTypes (must match watch constants.h)
    const val TOGGLE_TYPE_FIND_PHONE = 0
    const val TOGGLE_TYPE_CYCLE_RINGER = 1

    // Toggle states - must be strings for toggle order string comparison
    const val TOGGLE_STATE_OFF = "0"
    const val TOGGLE_STATE_ON = "1"
    const val TOGGLE_STATE_RINGER_LOUD = "2"
    const val TOGGLE_STATE_RINGER_VIBRATE = "3"
    const val TOGGLE_STATE_RINGER_SILENT = "4"
}
