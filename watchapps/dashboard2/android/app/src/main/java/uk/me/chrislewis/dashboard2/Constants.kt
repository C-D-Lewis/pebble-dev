package uk.me.chrislewis.dashboard2

import io.rebble.pebblekit2.common.model.PebbleDictionaryItem
import java.util.UUID

object Constants {
    // Watchapp UUID
    val APP_UUID: UUID = UUID.fromString("7606dabf-67cf-466a-92a7-5e62598a1436")

    // Compatible watchapp protocol version minimum
    // Increment when protocol has breaking changes
    val COMPATIBLE_PROTOCOL_VERSION = PebbleDictionaryItem.Int32(1)

    // AppMessage keys
    const val MESSAGE_KEY_COMPAT_PROTOCOL_VERSION = 10000u
    const val MESSAGE_KEY_SYNC_REQUEST = 10001u
    const val MESSAGE_KEY_SYNC_TOGGLE_ORDER = 10002u
    const val MESSAGE_KEY_SYNC_DEVICE_NAME = 10003u
    const val MESSAGE_KEY_SYNC_BATTERY_PERC = 10004u
    const val MESSAGE_KEY_SYNC_FREE_SPACE = 10005u
    const val MESSAGE_KEY_SYNC_FREE_SPACE_PERC = 10006u
}
