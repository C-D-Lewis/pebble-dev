#include "data.h"

static bool
  s_battery_meter = true,
  s_bluetooth_alert = false,
  s_dashed_line = true,
  s_second_tick = true;
static GColor 
  s_background_color,
  s_date_color,
  s_time_color,
  s_bracket_color,
  s_line_color,
  s_complication_color;

void data_init() {
  if(persist_exists(MESSAGE_KEY_BatteryMeter)) {
    // Load existing data
    s_battery_meter = persist_read_bool(MESSAGE_KEY_BatteryMeter);
    s_bluetooth_alert = persist_read_bool(MESSAGE_KEY_BluetoothAlert);
    s_dashed_line = persist_read_bool(MESSAGE_KEY_DashedLine);
    s_second_tick = persist_read_bool(MESSAGE_KEY_SecondTick);
    
    s_background_color = (GColor){ .argb = persist_read_int(MESSAGE_KEY_BackgroundColor) };
    s_date_color = (GColor){ .argb = persist_read_int(MESSAGE_KEY_DateColor) };
    s_time_color = (GColor){ .argb = persist_read_int(MESSAGE_KEY_TimeColor) };
    s_bracket_color = (GColor){ .argb = persist_read_int(MESSAGE_KEY_BracketColor) };
    s_line_color = (GColor){ .argb = persist_read_int(MESSAGE_KEY_LineColor) };
    s_complication_color = (GColor){ .argb = persist_read_int(MESSAGE_KEY_ComplicationColor) };
  } else {
    // Save defaults
    s_battery_meter = true;
    s_bluetooth_alert = false;
    s_dashed_line = true;
    s_second_tick = true;
    persist_write_bool(MESSAGE_KEY_BatteryMeter, s_battery_meter);
    persist_write_bool(MESSAGE_KEY_BluetoothAlert, s_bluetooth_alert);
    persist_write_bool(MESSAGE_KEY_DashedLine, s_dashed_line);
    persist_write_bool(MESSAGE_KEY_SecondTick, s_second_tick);

    s_background_color = GColorWhite;
    s_date_color = GColorBlack;
    s_time_color = GColorBlack;
    s_bracket_color = GColorBlack;
    s_line_color = GColorDarkGray;
    s_complication_color = GColorBlack;
    persist_write_int(MESSAGE_KEY_BackgroundColor, s_background_color.argb);
    persist_write_int(MESSAGE_KEY_DateColor, s_date_color.argb);
    persist_write_int(MESSAGE_KEY_TimeColor, s_time_color.argb);
    persist_write_int(MESSAGE_KEY_BracketColor, s_bracket_color.argb);
    persist_write_int(MESSAGE_KEY_LineColor, s_line_color.argb);
    persist_write_int(MESSAGE_KEY_ComplicationColor, s_complication_color.argb);
  }
}

void data_deinit() { }

bool data_get_boolean(uint32_t key) {
  // Can't use switch with MESSAGE_KEY_ constants directly
  if (key == MESSAGE_KEY_BatteryMeter) {
    return s_battery_meter;
  } else if (key == MESSAGE_KEY_BluetoothAlert) {
    return s_bluetooth_alert;
  } else if (key == MESSAGE_KEY_DashedLine) {
    return s_dashed_line;
  } else if (key == MESSAGE_KEY_SecondTick) {
    return s_second_tick;
  } else {
    return false; // Unknown key
  }
}

void data_set_boolean(uint32_t key, bool value) {
  if (key == MESSAGE_KEY_BatteryMeter) {
    s_battery_meter = value;
    persist_write_bool(MESSAGE_KEY_BatteryMeter, value);
  } else if (key == MESSAGE_KEY_BluetoothAlert) {
    s_bluetooth_alert = value;
    persist_write_bool(MESSAGE_KEY_BluetoothAlert, value);
  } else if (key == MESSAGE_KEY_DashedLine) {
    s_dashed_line = value;
    persist_write_bool(MESSAGE_KEY_DashedLine, value);
  } else if (key == MESSAGE_KEY_SecondTick) {
    s_second_tick = value;
    persist_write_bool(MESSAGE_KEY_SecondTick, value);
  }
}

GColor data_get_color(uint32_t key) {
  if (key == MESSAGE_KEY_BackgroundColor) {
    return s_background_color;
  } else if (key == MESSAGE_KEY_DateColor) {
    return s_date_color;
  } else if (key == MESSAGE_KEY_TimeColor) {
    return s_time_color;
  } else if (key == MESSAGE_KEY_BracketColor) {
    return s_bracket_color;
  } else if (key == MESSAGE_KEY_LineColor) {
    return s_line_color;
  } else if (key == MESSAGE_KEY_ComplicationColor) {
    return s_complication_color;
  } else {
    return GColorBlack; // Default color for unknown keys
  }
}

void data_set_color(uint32_t key, GColor color) {
  if (key == MESSAGE_KEY_BackgroundColor) {
    s_background_color = color;
    persist_write_int(MESSAGE_KEY_BackgroundColor, color.argb);
  } else if (key == MESSAGE_KEY_DateColor) {
    s_date_color = color;
    persist_write_int(MESSAGE_KEY_DateColor, color.argb);
  } else if (key == MESSAGE_KEY_TimeColor) {
    s_time_color = color;
    persist_write_int(MESSAGE_KEY_TimeColor, color.argb);
  } else if (key == MESSAGE_KEY_BracketColor) {
    s_bracket_color = color;
    persist_write_int(MESSAGE_KEY_BracketColor, color.argb);
  } else if (key == MESSAGE_KEY_LineColor) {
    s_line_color = color;
    persist_write_int(MESSAGE_KEY_LineColor, color.argb);
  } else if (key == MESSAGE_KEY_ComplicationColor) {
    s_complication_color = color;
    persist_write_int(MESSAGE_KEY_ComplicationColor, color.argb);
  }
}
