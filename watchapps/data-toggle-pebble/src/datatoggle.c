#include <pebble.h>

//Elements
static Window *window;
static TextLayer *title_layer, *output_layer, *wifi_status_layer, *data_status_layer;
static BitmapLayer *wifi_layer, *data_layer;
static GBitmap *wifi_bitmap, *data_bitmap;
static InverterLayer *title_div, *radio_div, *output_div, *wifi_highlight, *data_highlight;

static bool wifi_on = false;

//Set of enumerated keys with names and values. Identical to Android app keys
enum {
  KEY_MSG = 0,
  MSG_QUERY = 1,
  
  KEY_RADIO = 2,
  RADIO_WIFI = 3,
  RADIO_DATA = 4,
  
  STATUS_ON = 5,
  STATUS_OFF = 6
};

//Prototypes
void set_image(BitmapLayer *layer, GBitmap *bitmap, GRect frame);
void send_cmd(uint8_t key, uint8_t cmd);
void interpret_message_result(AppMessageResult app_message_error);
void applog(char* message);

/*************************** AppMessage Functions ******************************/

void process_tuple(Tuple *t)
{
	int key = t->key;
	int value = t->value->int32;
	
	if(key == RADIO_WIFI && value == STATUS_ON)
	{
		text_layer_set_text(output_layer, "Response received.");
		text_layer_set_text(wifi_status_layer, "ON");
		wifi_on = true;
		
		applog("Wifi on");
	}
	else if(key == RADIO_WIFI && value == STATUS_OFF)
	{
		text_layer_set_text(output_layer, "Response received.");
		text_layer_set_text(wifi_status_layer, "OFF");
		wifi_on = false;
		
		applog("Wifi off");
	}
	
	//Can't toggle data while wifi is on
	if(wifi_on == false)
	{
		if(key == RADIO_DATA && value == STATUS_ON)
		{
			text_layer_set_text(output_layer, "Response received.");
			text_layer_set_text(data_status_layer, "ON");
	
			applog("Data on");
		}
		else if(key == RADIO_DATA && value == STATUS_OFF)
		{
			text_layer_set_text(output_layer, "Response received.");
			text_layer_set_text(data_status_layer, "OFF");
	
			applog("Data off");
		}
	}
	else
	{
		text_layer_set_text(output_layer, "Response received.");
		text_layer_set_text(data_status_layer, "N/A");
	}
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) 
{
	 applog("out_failed_handler");
	 
	 interpret_message_result(reason);
	 
	 text_layer_set_text(output_layer, "SEND FAILED");
}

void in_received_handler(DictionaryIterator *iter, void *context) 
{
	(void) context;
	
	applog("Incoming received.");
	
	//Get data
	Tuple *t = dict_read_first(iter);
	if(t)
	{
		process_tuple(t);
	}
	
	//Get next
	while(t != NULL)
	{
		applog("Next tuple...");
	
		t = dict_read_next(iter);
		if(t)
		{
			process_tuple(t);
		}
	}
}

void in_dropped_handler(AppMessageResult reason, void *context) 
{
	applog("in_dropper_handler");
	 
	interpret_message_result(reason);
	 
	text_layer_set_text(output_layer, "INCOMING DROPPED");
}

/*************************** Timer callbacks **********************************/

static void wifi_timer_callback(void *data) {	
	layer_remove_from_parent(inverter_layer_get_layer(wifi_highlight)); 
}

static void data_timer_callback(void *data) {	
	layer_remove_from_parent(inverter_layer_get_layer(data_highlight)); 
}

/************************** Click functions ***********************************/

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	//Highlight
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(wifi_highlight));
	app_timer_register(300, (AppTimerCallback) wifi_timer_callback, 0);

	//Request change from Android
	text_layer_set_text(output_layer, "Toggling...");
	send_cmd(KEY_RADIO, RADIO_WIFI);

	applog("Requested toggle");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	//Highlight
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(data_highlight));
	app_timer_register(300, (AppTimerCallback) data_timer_callback, 0);
	
	//Can't toggle data when wifi is on
	if(wifi_on == false)
	{
		//Request change from Android
		text_layer_set_text(output_layer, "Toggling...");
		send_cmd(KEY_RADIO, RADIO_DATA);
	}
	else
	{
		text_layer_set_text(output_layer, "Disable Wi-Fi first!");
	}

	applog("Requested toggle");
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

/************************* Window Lifecycle ************************************/

static void window_load(Window *window) {
	window_set_background_color(window, GColorBlack);

	//Title
	title_layer = text_layer_create(GRect(0, 0, 144, 40));
	text_layer_set_text_color(title_layer, GColorWhite);
	text_layer_set_background_color(title_layer, GColorClear);
	text_layer_set_text_alignment(title_layer, GTextAlignmentCenter);
	text_layer_set_text(title_layer, "DATA TOGGLE");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(title_layer));

	//Title divider
	title_div = inverter_layer_create(GRect(0, 18, 144, 2));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(title_div));

	//Font
	ResHandle f_handle = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_40);

	//Wifi logo
	wifi_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WIFI);
	wifi_layer = bitmap_layer_create(GRect(10, 29, 39, 31));
	set_image(wifi_layer, wifi_bitmap, GRect(10, 29, 39, 31));
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(wifi_layer));

	//Wifi status
	wifi_status_layer = text_layer_create(GRect(55, 15, 144, 50));
	text_layer_set_text_color(wifi_status_layer, GColorWhite);
	text_layer_set_background_color(wifi_status_layer, GColorClear);
	text_layer_set_text_alignment(wifi_status_layer, GTextAlignmentLeft);
	text_layer_set_font(wifi_status_layer, fonts_load_custom_font(f_handle));
	text_layer_set_text(wifi_status_layer, "?");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(wifi_status_layer));

	//Wifi highlight
	wifi_highlight = inverter_layer_create(GRect(0, 20, 144, 46));

	//Radio divider
	radio_div = inverter_layer_create(GRect(0, 66, 144, 2));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(radio_div));

	//Data logo
	data_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DATA);
	data_layer = bitmap_layer_create(GRect(15, 73, 25, 39));
	set_image(data_layer, data_bitmap, GRect(15, 73, 25, 39));
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(data_layer));

	//Data status
	data_status_layer = text_layer_create(GRect(55, 65, 144, 50));
	text_layer_set_text_color(data_status_layer, GColorWhite);
	text_layer_set_background_color(data_status_layer, GColorClear);
	text_layer_set_text_alignment(data_status_layer, GTextAlignmentLeft);
	text_layer_set_font(data_status_layer, fonts_load_custom_font(f_handle));
	text_layer_set_text(data_status_layer, "?");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(data_status_layer));

	//Data highlight
	data_highlight = inverter_layer_create(GRect(0, 68, 144, 49));

	//Output divider
	output_div = inverter_layer_create(GRect(0, 115, 144, 2));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(output_div));

	//Output
	output_layer = text_layer_create(GRect(5, 119, 144, 40));
	text_layer_set_text_color(output_layer, GColorWhite);
	text_layer_set_background_color(output_layer, GColorClear);
	text_layer_set_text_alignment(output_layer, GTextAlignmentLeft);
	text_layer_set_text(output_layer, "Requesting status");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(output_layer));

	//Get status
	send_cmd(KEY_MSG, MSG_QUERY);

	applog("Sent status query");
}

static void window_unload(Window *window) {
	//Free all elements
	text_layer_destroy(title_layer);
	inverter_layer_destroy(title_div);
	gbitmap_destroy(wifi_bitmap);
	text_layer_destroy(wifi_status_layer);
	inverter_layer_destroy(wifi_highlight);
	inverter_layer_destroy(radio_div);
	gbitmap_destroy(data_bitmap);
	text_layer_destroy(data_status_layer);
	inverter_layer_destroy(data_highlight);
	inverter_layer_destroy(output_div);
	text_layer_destroy(output_layer);
}

/********************************* App Lifecycle *****************************/

static void init(void) {
	window = window_create();
	window_set_click_config_provider(window, click_config_provider);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});

	//AppMessage
	const int inbound_size = 64;
	const int outbound_size = 64;
	app_message_open(inbound_size, outbound_size);

	//Register AppMessage
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_failed(out_failed_handler);						
								
	//Fast response
	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);

	//Finally
	window_stack_push(window, true);
}

static void deinit(void) {
	//Reset sniff to save power
	app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);

	//Finally
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

/******************************** Function Definitions ******************************/

/**
	*	Set a BmpContainer's image
	*/
void set_image(BitmapLayer *layer, GBitmap *bitmap, GRect frame) {
	//Deinit old
	layer_remove_from_parent((Layer*) layer);
	bitmap_layer_destroy(layer);

	//Init new
	layer = bitmap_layer_create(frame);

	//Set bitmap
	bitmap_layer_set_bitmap(layer, bitmap);

	//Add to window
	layer_add_child(window_get_root_layer(window), (Layer*) layer);
}

/*
 * Send a cmd key
 */
void send_cmd(uint8_t key, uint8_t cmd)
{
	DictionaryIterator *iter;
 	app_message_outbox_begin(&iter);
 	
 	Tuplet value = TupletInteger(key, cmd);
 	dict_write_tuplet(iter, &value);
 	
 	app_message_outbox_send();
}

/**
	* Convenience function to log
	*/
void applog(char* message)
{
	app_log(APP_LOG_LEVEL_INFO, "datatoggle.c", 0, message);
}

/**
	* Interpret AppMessageResult
	*/
void interpret_message_result(AppMessageResult app_message_error)
{
	if(app_message_error == APP_MSG_OK)
	{
		applog("APP_MSG_OK");
	} 

	else if(app_message_error == APP_MSG_SEND_TIMEOUT)
	{
		applog("APP_MSG_SEND_TIMEOUT");
	} 

	else if(app_message_error == APP_MSG_SEND_REJECTED)
	{
		applog("APP_MSG_SEND_REJECTED");
	}

	else if(app_message_error == APP_MSG_NOT_CONNECTED)
	{
		applog("APP_MSG_NOT_CONNECTED");
	}

	else if(app_message_error == APP_MSG_APP_NOT_RUNNING)
	{
		applog("APP_MSG_APP_NOT_RUNNING");
	}

	else if(app_message_error == APP_MSG_INVALID_ARGS)
	{
		applog("APP_MSG_INVALID_ARGS");
	}

	else if(app_message_error == APP_MSG_BUSY)
	{
		applog("APP_MSG_BUSY");
	}

	else if(app_message_error == APP_MSG_BUFFER_OVERFLOW)
	{
		applog("APP_MSG_BUFFER_OVERFLOW");
	}

	else if(app_message_error == APP_MSG_ALREADY_RELEASED)
	{
		applog("APP_MSG_ALREADY_RELEASED");
	}

	else if(app_message_error == APP_MSG_CALLBACK_ALREADY_REGISTERED)
	{
		applog("APP_MSG_CALLBACK_ALREADY_REGISTERED");
	}

	else if(app_message_error == APP_MSG_CALLBACK_NOT_REGISTERED)
	{
		applog("APP_MSG_CALLBACK_NOT_REGISTERED");
	}

	else if(app_message_error == APP_MSG_OUT_OF_MEMORY)
	{
		applog("APP_MSG_OUT_OF_MEMORY");
	}
}
