#include <pebble.h>

#include "windows/splash_window.h"
#include "modules/comm.h"
#include "modules/data.h"
#include "modules/settings.h"
#include "config.h"

#if PBL_API_EXISTS(app_glance_reload)
static void app_glance_callback(AppGlanceReloadSession *session, size_t limit, void *context) {
  if(limit < 1) {
    return;
  }

  const AppGlanceSlice entry = (AppGlanceSlice) {
    .layout = {
      .subtitle_template_string = &data_get_story(0)->title[0]
    },
    .expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION
  };

  const AppGlanceResult result = app_glance_add_slice(session, entry);
  if(result != APP_GLANCE_RESULT_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "AppGlance error: %d", result);
  }
}
#endif

static void init() {
  settings_init();
  data_init();
  comm_init();
  splash_window_push();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap free: %d", (int)heap_bytes_free());
}

static void deinit() {
  // Don't cache now, incase it's invalid
  data_deinit();

  // AppGlance oooh
#if PBL_API_EXISTS(app_glance_reload)
  app_glance_reload(app_glance_callback, NULL);
#endif
}

int main() {
  init();
  app_event_loop();
  deinit();
}
