#include "sending_window.h"

#define FRAME_DELTA 33
#define EXIT_DELAY  3000

static Window *s_window;
static TextLayer *s_status_layer;
static Layer *s_icon_layer;

static GBitmap *s_sent_bitmap;
static BitmapLayer *s_bitmap_layer;

#if defined(PBL_SDK_3)
static GDrawCommandSequence *s_sequence;
static GDrawCommandImage *s_command_image;
#endif

static AppTimer *s_sequence_timer, *s_exit_timer;
static ContactType s_contact_type;
static int s_contact_index, s_sequence_index, s_response_index;

static void update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const int up_nudge = 10;

#if defined(PBL_SDK_3)
  GSize seq_bounds = gdraw_command_sequence_get_bounds_size(s_sequence);

  GDrawCommandFrame *frame = gdraw_command_sequence_get_frame_by_index(s_sequence, s_sequence_index);
  if(frame) {
    gdraw_command_frame_draw(ctx, s_sequence, frame, GPoint(
      (bounds.size.w - seq_bounds.w) / 2,
      ((bounds.size.h - seq_bounds.h) / 2) - up_nudge
    ));
  }
#endif
}

static void timer_handler(void *context) {
  layer_mark_dirty(s_icon_layer);

#if defined(PBL_SDK_3)
  int num_frames = gdraw_command_sequence_get_num_frames(s_sequence);
  s_sequence_index++;
  if(s_sequence_index < num_frames) {
    s_sequence_timer = app_timer_register(FRAME_DELTA, timer_handler, NULL);
  }
#endif
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_status_layer = text_layer_create(grect_inset(bounds, GEdgeInsets(PBL_IF_COLOR_ELSE(100, 90), 0, 0, 0)));
  text_layer_set_text_color(s_status_layer, GColorWhite);
  text_layer_set_background_color(s_status_layer, GColorClear);
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_status_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_status_layer));

#if defined(PBL_ROUND)
  text_layer_enable_screen_text_flow_and_paging(s_status_layer, 3);
#endif

#if defined(PBL_SDK_3)
  s_sequence = gdraw_command_sequence_create_with_resource(RESOURCE_ID_SENT_SEQUENCE);
#endif

  s_sent_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SENT_IMAGE);
  GSize bitmap_size = gbitmap_get_bounds(s_sent_bitmap).size;

  const int x_margin = (bounds.size.w - bitmap_size.w) / 2;
  const int y_margin = (bounds.size.h - bitmap_size.h) / 2;
  s_bitmap_layer = bitmap_layer_create(grect_inset(bounds, GEdgeInsets(y_margin - 20, x_margin, y_margin, x_margin)));
  bitmap_layer_set_bitmap(s_bitmap_layer, s_sent_bitmap);
  bitmap_layer_set_compositing_mode(s_bitmap_layer, PBL_IF_COLOR_ELSE(GCompOpSet, GCompOpAssign));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

  s_icon_layer = layer_create(bounds);
  layer_set_update_proc(s_icon_layer, update_proc);
  layer_add_child(window_layer, s_icon_layer);

#if defined(PBL_SDK_3)
  // Begin sequence after delay
  s_sequence_timer = app_timer_register(500, timer_handler, NULL);
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer), true);
#endif
}

#if defined(TEST)
static void test_success_handler(void *context) {
  sending_window_exit_after_result(SendingResultSuccess);
}
#endif

static void window_unload(Window *window) {
  text_layer_destroy(s_status_layer);
  layer_destroy(s_icon_layer);

  gbitmap_destroy(s_sent_bitmap);
  bitmap_layer_destroy(s_bitmap_layer);
#if defined(PBL_SDK_3)
  gdraw_command_sequence_destroy(s_sequence);
#endif

  window_destroy(s_window);
  s_window = NULL;
}

static void window_disappear(Window *window) {
  // Stop any timers if the user prematurely exits
  if(s_sequence_timer) {
    app_timer_cancel(s_sequence_timer);
  }
  s_sequence_timer = NULL;
  if(s_exit_timer) {
    app_timer_cancel(s_exit_timer);
  }
  s_exit_timer = NULL;
}

void sending_window_push(ContactType contact_type, int contact_index, int response_index, char *transcription) {
  s_contact_type = contact_type;
  s_contact_index = contact_index;
  s_response_index = response_index;

  s_sequence_index = 0;

  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack));
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      .disappear = window_disappear
    });
  }
  window_stack_push(s_window, true);

  static char s_buff[64];
  snprintf(s_buff, sizeof(s_buff), "Sending to:\n%s", data_get_contact_name(s_contact_type, s_contact_index));
  text_layer_set_text(s_status_layer, s_buff);

#if defined(TEST)
  app_timer_register(3000, test_success_handler, NULL);
#endif
  if(transcription) {
    comm_send_sms_with_transcription(s_contact_type, s_contact_index, transcription);
  } else {
    comm_send_sms(s_contact_type, s_contact_index, s_response_index);
  }
}

static void exit_handler(void *context) {
  window_stack_pop(true);
}

static void set_image(SendingResult result) {
  if(s_sent_bitmap) {
    gbitmap_destroy(s_sent_bitmap);
  }
  switch(result) {
    case SendingResultSuccess:
      s_sent_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SUCCESS);
      break;
    default:
      s_sent_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FAILED);
      break;
  }
  layer_set_hidden(s_icon_layer, true);
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer), false);
  bitmap_layer_set_bitmap(s_bitmap_layer, s_sent_bitmap);
}

void sending_window_exit_after_result(SendingResult result) {
  switch(result) {
    case SendingResultSuccess:
      window_set_background_color(s_window, PBL_IF_COLOR_ELSE(WRISTPONDER_THEME_COLOR, GColorBlack));
      text_layer_set_text(s_status_layer, "Success!");

      // TODO optional vibe
      vibes_short_pulse();

      s_exit_timer = app_timer_register(EXIT_DELAY, exit_handler, NULL);
      break;
    case SendingResultSMSFailed:
      window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorBlack));
      text_layer_set_text(s_status_layer, "SMS failed!");

      vibes_long_pulse();
      break;
    case SendingResultAppMessageFailed:
      window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorBlack));
      text_layer_set_text(s_status_layer, "Request failed!");

      vibes_long_pulse();
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown sending_window_exit_after_result result %d", (int)result);
      break;
  }
  set_image(result);
}
