package uk.me.chrislewis.dashboard2.service

import android.content.Context
import android.util.Log
import io.rebble.pebblekit2.client.BasePebbleListenerService
import io.rebble.pebblekit2.client.DefaultPebbleSender
import io.rebble.pebblekit2.common.model.PebbleDictionary
import io.rebble.pebblekit2.common.model.PebbleDictionaryItem
import io.rebble.pebblekit2.common.model.ReceiveResult
import io.rebble.pebblekit2.common.model.WatchIdentifier
import uk.me.chrislewis.dashboard2.Constants
import uk.me.chrislewis.dashboard2.features.Device
import uk.me.chrislewis.dashboard2.features.Alarms
import uk.me.chrislewis.dashboard2.features.Battery
import uk.me.chrislewis.dashboard2.features.FindPhone
import uk.me.chrislewis.dashboard2.features.Ringer
import java.util.UUID

private const val TAG = "PebbleReceiverService"

class PebbleReceiverService : BasePebbleListenerService() {
    private val sender = DefaultPebbleSender(this)

    /**
     * When an AppMessage is received.
     */
    override suspend fun onMessageReceived(
        watchappUUID: UUID,
        data: PebbleDictionary,
        watch: WatchIdentifier
    ): ReceiveResult {
        Log.d(TAG, "onMessageReceived: $data")

        // Settings and state sync
        if (data.containsKey(Constants.MESSAGE_KEY_SYNC_REQUEST)) return handleSyncRequest()

        // Toggle features
        if (data.containsKey(Constants.MESSAGE_KEY_TOGGLE_TYPE)) {
            val item: PebbleDictionaryItem? = data[Constants.MESSAGE_KEY_TOGGLE_TYPE]
            if (item == null) {
                Log.e(TAG, "Failed to get toggle type")
                return ReceiveResult.Nack
            }

            val type = item.value as Int
            Log.d(TAG, "Toggling type: $type ($item)")

            // Find Phone
            if (type == Constants.TOGGLE_TYPE_FIND_PHONE) {
                FindPhone.handleFindPhone(this)
                return handleSyncRequest()
            }

            // Cycle ringer
            if (type == Constants.TOGGLE_TYPE_CYCLE_RINGER) {
                Ringer.cycleRingerMode(this)
                return handleSyncRequest()
            }

            // More...

            Log.e(TAG, "Unknown toggle type: $type")
        }

        Log.e(TAG, "Unknown data: $data")
        return ReceiveResult.Ack
    }

    /**
     * Respond to a sync request with current state.
     */
    suspend fun handleSyncRequest(): ReceiveResult {
        // Assemble all sync data
        val dict = mapOf(
            Constants.MESSAGE_KEY_COMPAT_PROTOCOL_VERSION to PebbleDictionaryItem.Int32(Constants.COMPATIBLE_PROTOCOL_VERSION),
            Constants.MESSAGE_KEY_SYNC_TOGGLE_ORDER to PebbleDictionaryItem.Text(getToggleOrderString(this)),
            Constants.MESSAGE_KEY_SYNC_DEVICE_NAME to PebbleDictionaryItem.Text(Device.getDeviceName(this)),
            Constants.MESSAGE_KEY_SYNC_BATTERY_PERC to PebbleDictionaryItem.Text(Battery.getBatteryLevel(this).toString()),
            Constants.MESSAGE_KEY_SYNC_FREE_SPACE to PebbleDictionaryItem.Text(Device.getFreeDiskSpace()),
            Constants.MESSAGE_KEY_SYNC_FREE_SPACE_PERC to PebbleDictionaryItem.Text(Device.getFreeDiskSpacePercentage().toString()),
            Constants.MESSAGE_KEY_SYNC_NEXT_ALARM to PebbleDictionaryItem.Text(Alarms.getNextAlarmTime(this)),
            Constants.MESSAGE_KEY_SYNC_IS_CHARGING to PebbleDictionaryItem.Int32(if (Battery.isCharging(this)) 1 else 0),
        )

        val result = sender.sendDataToPebble(Constants.APP_UUID, dict)
        Log.d(TAG, "Send $dict: $result")
        return ReceiveResult.Ack
    }

    /**
     * Get toggle order as a string. Constructed based on current state.
     *
     * Format: 2 chars per toggle - [TYPE,STATE] with defined toggle types and state values.
     */
    fun getToggleOrderString(context: Context): String {
        return "${Constants.TOGGLE_TYPE_FIND_PHONE}" + (if (FindPhone.isPlaying()) Constants.TOGGLE_STATE_ON else Constants.TOGGLE_STATE_OFF) +
                "${Constants.TOGGLE_TYPE_CYCLE_RINGER}" + Ringer.getRingerModeProtocolValue(context)
    }

    /**
     * When watchapp opened.
     */
    override fun onAppOpened(watchappUUID: UUID, watch: WatchIdentifier) {
//        Log.d(TAG, "onAppOpened")
    }

    /**
     * When watchapp closed.
     */
    override fun onAppClosed(watchappUUID: UUID, watch: WatchIdentifier) {
//        Log.d(TAG, "onAppClosed")
    }

    /**
     * On service destroy.
     */
    override fun onDestroy() {
        super.onDestroy()
        sender.close()
        FindPhone.stopPlaying()
    }
}