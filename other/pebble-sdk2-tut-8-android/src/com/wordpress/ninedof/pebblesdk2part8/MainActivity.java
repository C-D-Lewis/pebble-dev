package com.wordpress.ninedof.pebblesdk2part8;

import java.util.UUID;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.widget.TextView;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.PebbleKit.PebbleDataReceiver;
import com.getpebble.android.kit.util.PebbleDictionary;

public class MainActivity extends Activity {

	private PebbleDataReceiver mReceiver;
	private TextView mButtonView;

	private static final int
	KEY_BUTTON_EVENT = 0,
	BUTTON_EVENT_UP = 1,
	BUTTON_EVENT_DOWN = 2,
	BUTTON_EVENT_SELECT = 3,
	KEY_VIBRATION = 4;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		mButtonView = new TextView(this);
		mButtonView.setText("No button yet!");

		setContentView(mButtonView);
	}

	@Override
	protected void onResume() {
		super.onResume();

		mReceiver = new PebbleDataReceiver(UUID.fromString("2fc99a5d-ee35-4057-aa9b-0d4dd8e35ef5")) {

			@Override
			public void receiveData(Context context, int transactionId, PebbleDictionary data) {
				//ACK the message
				PebbleKit.sendAckToPebble(context, transactionId);

				//Check the key exists
				if(data.getUnsignedIntegerAsLong(KEY_BUTTON_EVENT) != null) {
					int button = data.getUnsignedIntegerAsLong(KEY_BUTTON_EVENT).intValue();

					switch(button) {
					case BUTTON_EVENT_UP:
						//The UP button was pressed
						mButtonView.setText("UP button pressed!");
						break;
					case BUTTON_EVENT_DOWN:
						//The DOWN button was pressed
						mButtonView.setText("DOWN button pressed!");
						break;
					case BUTTON_EVENT_SELECT:
						//The SELECT button was pressed
						mButtonView.setText("SELECT button pressed!");
						break;
					}
				}
				
				//Make the watch vibrate
				PebbleDictionary dict = new PebbleDictionary();
				dict.addInt32(KEY_VIBRATION, 0);
				PebbleKit.sendDataToPebble(context, UUID.fromString("2fc99a5d-ee35-4057-aa9b-0d4dd8e35ef5"), dict);
			}

		};

		PebbleKit.registerReceivedDataHandler(this, mReceiver);
	}

	@Override
	protected void onPause() {
		super.onPause();

		unregisterReceiver(mReceiver);
	}

}
