package cdlewis.pebbleaccelstream;

import java.util.UUID;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.PebbleKit.PebbleDataReceiver;
import com.getpebble.android.kit.util.PebbleDictionary;
import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.GraphView.GraphViewData;
import com.jjoe64.graphview.GraphViewSeries;
import com.jjoe64.graphview.GraphViewSeries.GraphViewSeriesStyle;
import com.jjoe64.graphview.LineGraphView;

public class Landing extends Activity {
	
	//Constants
	public static final String TAG = Landing.class.getName();
	private static final int NUM_SAMPLES = 15;
	private static final int GRAPH_HISTORY = 200;
	
	//State
	private int sampleCount = 0;
	private long lastAverageTime = 0;
	private int[] latest_data;
	private GraphViewSeries seriesX, seriesY, seriesZ;
	private int sampleCounter = 0;
	private int totalData = 0;
	
	//Layout members
	private TextView 
		xView,
		yView,
		zView,
		rateView;
	private Button startButton;
	private GraphView gView;
	
	//Other members
	private PebbleDataReceiver receiver;
	private UUID uuid = UUID.fromString("2893b0c4-2bca-4c83-a33a-0ef6ba6c8b17");
	private Handler handler = new Handler();
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_landing);
		
		xView = (TextView)findViewById(R.id.x_view);
		yView = (TextView)findViewById(R.id.y_view);
		zView = (TextView)findViewById(R.id.z_view);
		rateView = (TextView)findViewById(R.id.rate_view);
		startButton = (Button)findViewById(R.id.start_button);
		
		startButton.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				PebbleDictionary dict = new PebbleDictionary();
				dict.addInt32(0, 0);
				PebbleKit.sendDataToPebble(getApplicationContext(), uuid, dict);
			}
			
		});
		
		//Graph
		seriesX = new GraphViewSeries("X", new GraphViewSeriesStyle(Color.argb(255, 255, 0, 0), 2), new GraphViewData[] {
		      new GraphViewData(1, 0)
		});
		seriesY = new GraphViewSeries("Y", new GraphViewSeriesStyle(Color.argb(255, 0, 255, 0), 2), new GraphViewData[] {
		      new GraphViewData(1, 0)
		});
		seriesZ = new GraphViewSeries("Z", new GraphViewSeriesStyle(Color.argb(255, 0, 0, 255), 2), new GraphViewData[] {
		      new GraphViewData(1, 0)
		});
			 
		gView = new LineGraphView(this, "Pebble Accelerometer History");
		gView.setShowLegend(true);
		gView.setViewPort(0, GRAPH_HISTORY);
		gView.setScrollable(true);
		gView.addSeries(seriesX);
		gView.addSeries(seriesY);
		gView.addSeries(seriesZ);
		 
		LinearLayout layout = (LinearLayout) findViewById(R.id.graph_layout);
		layout.addView(gView);
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		receiver = new PebbleDataReceiver(uuid) {
			
			@Override
			public void receiveData(Context context, int transactionId, PebbleDictionary data) {
				PebbleKit.sendAckToPebble(getApplicationContext(), transactionId);
				
				//Count total data
				totalData += 3 * NUM_SAMPLES * 4;
				
				//Get data
				latest_data = new int[3 * NUM_SAMPLES];
//				Log.d(TAG, "NEW DATA PACKET");
				for(int i = 0; i < NUM_SAMPLES; i++) {
					for(int j = 0; j < 3; j++) {
						try {
							latest_data[(3 * i) + j] = data.getInteger((3 * i) + j).intValue();
						} catch(Exception e) {
							latest_data[(3 * i) + j] = -1;
						}
					}
//					Log.d(TAG, "Sample " + i + " data: X: " + latest_data[(3 * i)] + ", Y: " + latest_data[(3 * i) + 1] + ", Z: " + latest_data[(3 * i) + 2]);
				}
				
				//Show
				handler.post(new Runnable() {
					
					@Override
					public void run() {
						xView.setText("X: " + latest_data[0]);
						yView.setText("Y: " + latest_data[1]);
						zView.setText("Z: " + latest_data[2]);
					}
					
				});

				//Show on graph
				for(int i = 0; i < NUM_SAMPLES; i++) {
					seriesX.appendData(new GraphViewData(sampleCounter, latest_data[(3 * i)]), true, GRAPH_HISTORY);
					seriesY.appendData(new GraphViewData(sampleCounter, latest_data[(3 * i) + 1]), true, GRAPH_HISTORY);
					seriesZ.appendData(new GraphViewData(sampleCounter, latest_data[(3 * i) + 2]), true, GRAPH_HISTORY);
					sampleCounter++;
				}

				if(System.currentTimeMillis() - lastAverageTime > 1000) {
					lastAverageTime = System.currentTimeMillis();
					
					rateView.setText("" + sampleCount + " samples per second."
							+ "\n"
							+ data.size() + " * 4-btye int * " + sampleCount + " samples = " + (4 * data.size() * sampleCount) + " Bps."
									+ "\n"
									+ "Total data received: " + getTotalDataString());
					sampleCount = 0;
				} else {
					sampleCount++;
				}
			}
			
		};
		
		PebbleKit.registerReceivedDataHandler(this, receiver);
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		
		unregisterReceiver(receiver);
	}
	
	private String getTotalDataString() {
		if(totalData < 1000) {
			return "" + totalData + " Bytes.";
		} else if(totalData > 1000 && totalData < 1000000) {
			return "" + totalData / 1000 + " KBytes.";
		} else {
			return "" + totalData / 1000000 + " MBytes.";
		}
	}
}
