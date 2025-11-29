#include "glance.h"

#if PBL_API_EXISTS(app_glance_reload)
void glance_handler(AppGlanceReloadSession *session, size_t limit, void *context) {
  if (limit < 1) return;

  static char s_buffer[32];
  if (data_calculate_days_remaining() == DATA_EMPTY) {
    snprintf(
      s_buffer,
      sizeof(s_buffer),
      "No data yet"
    );
  } else {
    snprintf(
      s_buffer,
      sizeof(s_buffer),
      "About %d days left",
      data_calculate_days_remaining()
    );
  }

  const AppGlanceSlice slice = (AppGlanceSlice) {
    .layout = {
      .subtitle_template_string = &s_buffer[0]
    },
    .expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION
  };
  const AppGlanceResult result = app_glance_add_slice(session, slice);
  if (result != APP_GLANCE_RESULT_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "AppGlance Error: %d", result);
  }
}
#endif
