#include <pebble.h>

#pragma once

#define OUTER_RING_W 4
#define OUTER_RING_INSET 16
#define DAY_RING_W 8
#define DAY_RING_INSET (OUTER_RING_INSET + OUTER_RING_W - 3)
#define INNER_RING_INSET (DAY_RING_INSET + DAY_RING_W)
#define SEP_H 5