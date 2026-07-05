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

private val APP_UUID = UUID.fromString("7606dabf-67cf-466a-92a7-5e62598a1436")
private val MESSAGE_KEY_SYNC_REQUEST = 10000u
private val MESSAGE_KEY_SYNC_DATA = 10001u

class PebbleReceiverService : BasePebbleListenerService() {
    private val sender = DefaultPebbleSender(this)

    override suspend fun onMessageReceived(
        watchappUUID: UUID,
        data: PebbleDictionary,
        watch: WatchIdentifier
    ): ReceiveResult {
        Log.d(TAG, "onMessageReceived: $data")

        // SYNC_REQUEST
        if (data.containsKey(MESSAGE_KEY_SYNC_REQUEST)) {
            val dataToSend = mapOf(
                MESSAGE_KEY_SYNC_DATA to PebbleDictionaryItem.Text("000102030405"),
            )
            val result = sender.sendDataToPebble(APP_UUID, dataToSend)
            Log.d(TAG, "Message result: $result")
            return ReceiveResult.Ack
        }

        Log.e(TAG, "Unknown data: $data")
        return ReceiveResult.Ack
    }

    override fun onAppOpened(watchappUUID: UUID, watch: WatchIdentifier) {
        Log.d(TAG, "onAppOpened")
    }

    override fun onAppClosed(watchappUUID: UUID, watch: WatchIdentifier) {
        Log.d(TAG, "onAppClosed")
    }

    override fun onDestroy() {
        super.onDestroy()
        sender.close()
    }
}