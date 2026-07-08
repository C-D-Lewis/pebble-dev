package uk.me.chrislewis.dashboard2.service

import android.util.Log
import io.rebble.pebblekit2.client.BasePebbleListenerService
import io.rebble.pebblekit2.client.DefaultPebbleSender
import io.rebble.pebblekit2.common.model.PebbleDictionary
import io.rebble.pebblekit2.common.model.PebbleDictionaryItem
import io.rebble.pebblekit2.common.model.ReceiveResult
import io.rebble.pebblekit2.common.model.WatchIdentifier
import uk.me.chrislewis.dashboard2.Config
import uk.me.chrislewis.dashboard2.Constants
import uk.me.chrislewis.dashboard2.Device
import uk.me.chrislewis.dashboard2.features.AutoSync
import uk.me.chrislewis.dashboard2.features.Battery
import java.util.UUID

private const val TAG = "PebbleReceiverService"

class PebbleReceiverService : BasePebbleListenerService() {
    private val sender = DefaultPebbleSender(this)

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

            if (type == Constants.TOGGLE_TYPE_AUTOSYNC) {
                AutoSync.toggleAutoSync(AutoSync.isAutoSyncEnabled())
                Log.d(TAG, "AutoSync now ${AutoSync.isAutoSyncEnabled()}")
                return handleSyncRequest()
            }

            Log.e(TAG, "Unknown toggle type: $type")
        }

        Log.e(TAG, "Unknown data: $data")
        return ReceiveResult.Ack
    }

    suspend fun handleSyncRequest(): ReceiveResult {
        // Assemble all sync data
        val dict = mapOf(
            Constants.MESSAGE_KEY_COMPAT_PROTOCOL_VERSION to Constants.COMPATIBLE_PROTOCOL_VERSION,
            Constants.MESSAGE_KEY_SYNC_TOGGLE_ORDER to PebbleDictionaryItem.Text(Config.getToggleOrderString()),
            Constants.MESSAGE_KEY_SYNC_DEVICE_NAME to PebbleDictionaryItem.Text(Device.getDeviceName(this)),
            Constants.MESSAGE_KEY_SYNC_BATTERY_PERC to PebbleDictionaryItem.Text(Battery.getBatteryLevel(this).toString()),
            Constants.MESSAGE_KEY_SYNC_FREE_SPACE to PebbleDictionaryItem.Text(Device.getFreeDiskSpace(this)),
            Constants.MESSAGE_KEY_SYNC_FREE_SPACE_PERC to PebbleDictionaryItem.Text(Device.getFreeDiskSpacePercentage().toString())
        )
        val result = sender.sendDataToPebble(Constants.APP_UUID, dict)
        Log.d(TAG, "Send $dict: $result")
        return ReceiveResult.Ack
    }

    override fun onAppOpened(watchappUUID: UUID, watch: WatchIdentifier) {
//        Log.d(TAG, "onAppOpened")
    }

    override fun onAppClosed(watchappUUID: UUID, watch: WatchIdentifier) {
//        Log.d(TAG, "onAppClosed")
    }

    override fun onDestroy() {
        super.onDestroy()
        sender.close()
    }
}