#include <pebble.h>
#include "alert.h"

static Window *window;

static void window_load(Window *window) {
	alert_show(window, "Hello, World!", "This is an example alert that will last for 60 seconds.", 60000);
}

static void window_unload(Window *window) {
	alert_cancel();
}

static void init(void) {
	window = window_create();
	WindowHandlers handlers = {
		.load = window_load,
		.unload = window_unload,
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
