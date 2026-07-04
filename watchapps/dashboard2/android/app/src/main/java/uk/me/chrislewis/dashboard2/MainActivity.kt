package uk.me.chrislewis.dashboard2

import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.lifecycle.lifecycleScope
import io.rebble.pebblekit2.client.BasePebbleListenerService
import io.rebble.pebblekit2.client.DefaultPebbleSender
import io.rebble.pebblekit2.common.model.PebbleDictionary
import io.rebble.pebblekit2.common.model.PebbleDictionaryItem
import io.rebble.pebblekit2.common.model.ReceiveResult
import io.rebble.pebblekit2.common.model.WatchIdentifier
import kotlinx.coroutines.launch
import uk.me.chrislewis.dashboard2.ui.theme.Dashboard2Theme
import java.time.LocalTime
import java.util.UUID

private const val TAG = "MainActivity"

private val APP_UUID = UUID.fromString("7606dabf-67cf-466a-92a7-5e62598a1436")

private val MESSAGE_KEY_TEST_MESSAGE = 10000u

class MainActivity : ComponentActivity() {
    private val sender = DefaultPebbleSender(this)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            Dashboard2Theme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    Column (
                        modifier = Modifier
                            .fillMaxSize()
                            .padding(innerPadding)
                            .padding(16.dp)
                    ) {
                        Text(
                            text = "Press button to send AppMessage"
                        )

                        Button(onClick = ::testSendData) {
                            Text("Send message")
                        }
                    }
                }
            }
        }
    }

    override fun onDestroy() {
        sender.close()
        super.onDestroy()
    }

    fun testSendData() {
        lifecycleScope.launch {
            val dataToSend = mapOf(
                MESSAGE_KEY_TEST_MESSAGE to PebbleDictionaryItem.Text("Hello from Android!"),
            )
            val result = sender.sendDataToPebble(
                APP_UUID,
                dataToSend
            )

            Log.d(TAG, "Message sent. Result: $result")
        }
    }
}
