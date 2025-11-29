#pragma once

#include <pebble.h>

#include "data.h"

#if PBL_API_EXISTS(app_glance_reload)
void glance_handler(AppGlanceReloadSession *session, size_t limit, void *context);
#endif
