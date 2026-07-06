package uk.me.chrislewis.dashboard2

import android.util.Log
import io.rebble.pebblekit2.client.BasePebbleListenerService
import io.rebble.pebblekit2.client.DefaultPebbleSender
import io.rebble.pebblekit2.common.model.PebbleDictionary
import io.rebble.pebblekit2.common.model.PebbleDictionaryItem
import io.rebble.pebblekit2.common.model.ReceiveResult
import io.rebble.pebblekit2.common.model.WatchIdentifier
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

        // Toggle requests

        Log.e(TAG, "Unknown data: $data")
        return ReceiveResult.Ack
    }

    suspend fun handleSyncRequest(): ReceiveResult {
        // Assemble all sync data
        val dict = mapOf(
            Constants.MESSAGE_KEY_COMPAT_PROTOCOL_VERSION to Constants.COMPATIBLE_PROTOCOL_VERSION,
            Constants.MESSAGE_KEY_SYNC_TOGGLE_ORDER to PebbleDictionaryItem.Text(Config.getToggleOrderString()),
            Constants.MESSAGE_KEY_SYNC_DEVICE_NAME to PebbleDictionaryItem.Text(Device.getDeviceName(this)),
            Constants.MESSAGE_KEY_SYNC_BATTERY_PERC to PebbleDictionaryItem.Text(Device.getBatteryLevel(this).toString()),
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