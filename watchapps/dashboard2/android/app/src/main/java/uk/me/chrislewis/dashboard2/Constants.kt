package uk.me.chrislewis.dashboard2

import java.util.UUID

// Watchapp UUID
val APP_UUID: UUID = UUID.fromString("7606dabf-67cf-466a-92a7-5e62598a1436")

// Compatible watchapp version minimum
val COMPATIBLE_VERSION_MAJOR = 0
var COMPATIBLE_VERSION_MINOR = 1

// AppMessage keys
const val MESSAGE_KEY_SYNC_REQUEST = 10000u
const val MESSAGE_KEY_SYNC_TOGGLE_ORDER = 10001u
const val MESSAGE_KEY_SYNC_DEVICE_NAME = 10002u
const val MESSAGE_KEY_SYNC_BATTERY_LEVEL = 10003u

class Constants {}
