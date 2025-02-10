#include <pebble.h>

Window *window;
GBitmap *qr_bitmap;
BitmapLayer *qr_layer;

static void window_load(Window *window)
{
	//Load bitmap
	qr_bitmap = gbitmap_create_with_resource(RESOURCE_ID_QR);
	
	//Load layer and show code
	qr_layer = bitmap_layer_create(GRect(0, (168 - 16 - 144)/2, 144, 144));
	bitmap_layer_set_bitmap(qr_layer, qr_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(qr_layer));
}

static void window_unload(Window *window)
{
	//Free memory
	gbitmap_destroy(qr_bitmap);
	bitmap_layer_destroy(qr_layer);
}

static void init(void) {
	window = window_create();
	WindowHandlers handlers = {
		.load = window_load,
		.unload = window_unload
	};
	window_set_window_handlers(window, handlers);
	window_stack_push(window, true);
}

static void deinit(void) {
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}