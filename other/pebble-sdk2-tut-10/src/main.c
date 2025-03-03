#include <pebble.h>
 
static Window* window;
static TextLayer *bt_layer, *batt_layer, *accel_layer;
 
static void window_load(Window *window)
{
  //Setup BT Layer
  bt_layer = text_layer_create(GRect(5, 5, 144, 30));
  text_layer_set_font(bt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  if(bluetooth_connection_service_peek() == true)
  {
    text_layer_set_text(bt_layer, "BT: CONNECTED");
  }
  else
  {
    text_layer_set_text(bt_layer, "BT: DISCONNECTED");
  }
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(bt_layer));
  
  //Setup Battery Layer
  batt_layer = text_layer_create(GRect(5, 25, 144, 30));
  text_layer_set_font(batt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(batt_layer));
  
  //Get info, copy to long-lived buffer and display
  BatteryChargeState state = battery_state_service_peek();
  static char buffer[] = "Battery: 100/100";
  snprintf(buffer, sizeof("Battery: 100/100"), "Battery: %d/100", state.charge_percent);
  text_layer_set_text(batt_layer, buffer);
  
  //Setup Accel Layer
  accel_layer = text_layer_create(GRect(5, 45, 144, 30));
  text_layer_set_font(accel_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text(accel_layer, "Accel tap: N/A");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(accel_layer));
}
 
static void window_unload(Window *window)
{
  text_layer_destroy(bt_layer);
  text_layer_destroy(batt_layer);
  text_layer_destroy(accel_layer);
}

static void bt_handler(bool connected)
{
  if(connected == true)
  {
    text_layer_set_text(bt_layer, "BT: CONNECTED");
  }
  else
  {
    text_layer_set_text(bt_layer, "BT: DISCONNECTED");
  }
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction)
{
  switch(axis)
  {
  case ACCEL_AXIS_X:
    if(direction > 0)
    {
      text_layer_set_text(accel_layer, "Accel tap: X (+)");
    }
    else
    {
      text_layer_set_text(accel_layer, "Accel tap: X (-)");
    }
    break;
  case ACCEL_AXIS_Y:
    if(direction > 0)
    {
      text_layer_set_text(accel_layer, "Accel tap: Y (+)");
    }
    else
    {
      text_layer_set_text(accel_layer, "Accel tap: Y (-)");
    }
    break;
  case ACCEL_AXIS_Z:
    if(direction > 0)
    {
      text_layer_set_text(accel_layer, "Accel tap: Z (+)");
    }
    else
    {
      text_layer_set_text(accel_layer, "Accel tap: Z (-)");
    }
    break;
  }
}

static void accel_raw_handler(AccelData *data, uint32_t num_samples)
{
  static char buffer[] = "XYZ: 9999 / 9999 / 9999";
  snprintf(buffer, sizeof("XYZ: 9999 / 9999 / 9999"), "XYZ: %d / %d / %d", data[0].x, data[0].y, data[0].z);
  text_layer_set_text(accel_layer, buffer);
}
 
static void init()
{
  window = window_create();
  WindowHandlers handlers = {
    .load = window_load,
    .unload = window_unload
  };
  window_set_window_handlers(window, (WindowHandlers) handlers);
  window_stack_push(window, true);
  
  //Subscribe to BluetoothConnectionService
  bluetooth_connection_service_subscribe(bt_handler);
  
  //Subscribe to AccelerometerService (uncomment one to choose)
  //accel_tap_service_subscribe(accel_tap_handler);
  accel_data_service_subscribe(1, accel_raw_handler);
}
 
static void deinit()
{
  window_destroy(window);
}
 
int main(void)
{
  init();
  app_event_loop();
  deinit();
}