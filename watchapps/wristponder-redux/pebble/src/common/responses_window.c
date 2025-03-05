#include "responses_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;
static AppTimer *s_show_mic_timer;

#if defined(PBL_MIC)
static DictationSession *s_dictation_session;
static BitmapLayer *s_mic_layer;
static GBitmap *s_mic_bitmap;
static char s_dictation_buffer[DICT_BUFFER];
#endif

static ContactType s_contact_type;
static int s_contact_index, s_num_responses, s_menu_index;
static bool s_mic_hidden, s_animating;

#if defined(PBL_MIC)
static void dictation_session_callback(DictationSession *session, DictationSessionStatus status,
                                       char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
    snprintf(s_dictation_buffer, sizeof(s_dictation_buffer), "%s", transcription);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Got transcription!");
    sending_window_push(s_contact_type, s_contact_index, -1, &s_dictation_buffer[0]);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Dictation failed: %d", (int)status);
  }
}

static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
  s_animating = false;
}

static void animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay) {
  PropertyAnimation *prop_anim = property_animation_create_layer_frame(layer, &start, &finish);
  Animation *anim = property_animation_get_animation(prop_anim);
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_curve(anim, AnimationCurveLinear);
  animation_set_handlers(anim, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);

  animation_schedule(anim);
  s_animating = true;
}

static void toggle_mic_handler(void *context) {
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  Layer *bitmap_layer = bitmap_layer_get_layer(s_mic_layer);
  GRect start = layer_get_frame(bitmap_layer);
  GSize bitmap_size = gbitmap_get_bounds(s_mic_bitmap).size;

  GRect finish;
  if(s_mic_hidden) {
    // Show
    const int x_margin = (bounds.size.w - bitmap_size.w) / 2;
    const int y_margin = 10;
    finish = grect_inset(bounds,
      GEdgeInsets(y_margin, x_margin, bounds.size.h - bitmap_size.h - y_margin, x_margin));
  } else {
    // Hide
    finish = GRect(start.origin.x, -bitmap_size.h, start.size.w, start.size.h);
  }
  animate_layer(bitmap_layer, start, finish, 100, 0);

  s_mic_hidden = !s_mic_hidden;
}
#endif

/********************************* MenuLayer **********************************/

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return s_num_responses;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  menu_cell_basic_draw(ctx, cell_layer, data_get_response(cell_index->row), NULL, NULL);
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT,
    44);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) { }

static void selection_changed_callback(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *context) {
  s_menu_index = new_index.row;
}

/*********************************** Clicks ***********************************/

static void show_hide_mic_handler(void *context) {
#if defined(PBL_MIC)
  toggle_mic_handler(NULL);
#endif
  s_show_mic_timer = NULL;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Request SMS send!
  if(data_get_response_valid(s_menu_index)) {
    sending_window_push(s_contact_type, s_contact_index, s_menu_index, NULL);
  } else {
    printf("not valid");
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(s_menu_index > 0 && !s_animating){
    s_menu_index--;

    if(s_show_mic_timer) {
      app_timer_cancel(s_show_mic_timer);
    }
    if(s_menu_index == 0) {
      // Wait for menu to move out the way
      s_show_mic_timer = app_timer_register(150, show_hide_mic_handler, NULL);
    }

    // Manually move MenuLayer
    MenuIndex next = { .row = s_menu_index };
    menu_layer_set_selected_index(s_menu_layer, next, MenuRowAlignCenter, true);
  } else {
#if defined(PBL_MIC)
    dictation_session_start(s_dictation_session);
#endif
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  int old_index = s_menu_index;
  if(s_menu_index < s_num_responses - 1 && !s_animating) {
    s_menu_index++;
  }

  if(s_show_mic_timer) {
    app_timer_cancel(s_show_mic_timer);
  }

#if defined(PBL_MIC)
  if(s_menu_index == 1 && old_index == 0) {
    // Going down
    toggle_mic_handler(NULL);
  }
#endif

  // Manually move MenuLayer
  MenuIndex next = { .row = s_menu_index };
  menu_layer_set_selected_index(s_menu_layer, next, MenuRowAlignCenter, true);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  // menu_layer_set_click_config_onto_window(s_menu_layer, window);
#if defined(PBL_COLOR)
  menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, WRISTPONDER_THEME_COLOR, GColorWhite);
#endif
#if defined(PBL_MIC)
  menu_layer_set_center_focused(s_menu_layer, true);
#endif
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = get_cell_height_callback,
      .select_click = select_callback,
      .selection_changed = selection_changed_callback
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

#if defined(PBL_MIC)
  s_mic_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MIC);
  GSize bitmap_size = gbitmap_get_bounds(s_mic_bitmap).size;
  const int x_margin = (bounds.size.w - bitmap_size.w) / 2;
  const int y_margin = 10;
  s_mic_layer = bitmap_layer_create(grect_inset(bounds,
    GEdgeInsets(y_margin, x_margin, bounds.size.h - bitmap_size.h - y_margin, x_margin)));
  bitmap_layer_set_compositing_mode(s_mic_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_mic_layer, s_mic_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_mic_layer));

  s_mic_hidden = false;
#endif
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;

#if defined(PBL_MIC)
  bitmap_layer_destroy(s_mic_layer);
  gbitmap_destroy(s_mic_bitmap);
  dictation_session_destroy(s_dictation_session);
#endif

  window_destroy(s_window);
  s_window = NULL;
}

void responses_window_push(ContactType contact_type, int contact_index) {
  s_contact_type = contact_type;
  s_contact_index = contact_index;

  s_menu_index = 0;

  if(!s_window) {
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);

  // Manually move MenuLayer
  MenuIndex next = { .row = s_menu_index };
  menu_layer_set_selected_index(s_menu_layer, next, MenuRowAlignCenter, true);

#if defined(PBL_MIC)
  s_dictation_session = dictation_session_create(sizeof(s_dictation_buffer), dictation_session_callback, NULL);
#endif

  responses_window_reload_data();
}

void responses_window_reload_data() {
  if(s_menu_layer) { // !
#if defined(TEST)
    s_num_responses = 5;
    data_set_response(0, "TEST_RESPONSE_1");
    data_set_response(1, "TEST_RESPONSE_2");
    data_set_response(2, "TEST_RESPONSE_3");
    data_set_response(3, "TEST_RESPONSE_4");
    data_set_response(4, "TEST_RESPONSE_5");
#else
    s_num_responses = data_get_obtained_responses();
#endif
    menu_layer_reload_data(s_menu_layer);
  }
}
