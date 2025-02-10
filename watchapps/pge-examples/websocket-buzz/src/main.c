#include <pebble.h>

#include "pge/pge.h"
#include "pge/modules/pge_ws.h"

// UI
static Window *s_game_window;
static TextLayer *s_output_layer;

/******************************** WS Callbacks ********************************/

static void ws_connection_handler(bool successful) {
  APP_LOG(APP_LOG_LEVEL_INFO, "ws_connection_handler");
}

static void ws_received_handler() {
  int player_count = pge_ws_get_value(PGE_WS_KEY_0);
  if(player_count != PGE_WS_NOT_FOUND) {
    static char s_buff[64];
    snprintf(s_buff, sizeof(s_buff), "ID: %d\nPlayers: %d\n\nPress Select to buzz other players!", 
      pge_ws_get_client_id(), player_count
    );
    text_layer_set_text(s_output_layer, s_buff);
  }

  int will_buzz = pge_ws_get_value(PGE_WS_KEY_1);
  if(will_buzz != PGE_WS_NOT_FOUND) {
    vibes_short_pulse();
  }
}

/******************************** Engine callbacks ****************************/

static void logic() {

}

static void draw(GContext *ctx) {

}

static void click(int button_id, bool long_click) {
  switch(button_id) {
    case BUTTON_ID_SELECT:
      if(pge_ws_is_connected()) {
        // Send buzz
        if(pge_ws_packet_begin()) {
          pge_ws_add_int(PGE_WS_KEY_1, 1);
          pge_ws_packet_send();
        }
      }
      break;
  }
}

/********************************* App Foundation *****************************/

void pge_init() {
  // Begin game loop
  pge_begin(GColorBlack, logic, draw, click);

  pge_ws_begin("ws://192.168.1.15:5500", ws_connection_handler, ws_received_handler);

  // Create score layer
  s_output_layer = text_layer_create(GRect(0, 0, 144, 168));
  text_layer_set_text_color(s_output_layer, GColorWhite);
  text_layer_set_background_color(s_output_layer, GColorBlack);
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_get_root_layer(pge_get_window()), text_layer_get_layer(s_output_layer)); 
}

void pge_deinit() {
  // Destroy score layer
  text_layer_destroy(s_output_layer);

  // End game loop
  pge_finish();
}