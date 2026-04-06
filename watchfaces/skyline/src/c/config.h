#include <pebble.h>

#pragma once

#define OUTER_RING_W 4
#define OUTER_RING_INSET 16
#define TEMP_RING_W 8
#define TEMP_RING_INSET (OUTER_RING_INSET + OUTER_RING_W - 3)
#define INNER_RING_INSET (TEMP_RING_INSET + TEMP_RING_W + 2)
#define SEP_H 5