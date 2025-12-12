# pebble-scalable

Package aiming to make it easy to make scaling layouts for different display
sizes by defining their dimension only once.

- [Setting up](#setting-up)
- [Dimensions](#dimensions)
- [Centering](#centering)
- [Fonts](#fonts)

See `include/pebble-scalable.h` for function documentation.

## Setting up

Install the Pebble packages:

```
$ pebble package install pebble-scalable
```

Add the includes at the top of your source.

```c
#include <pebble-scalable/pebble-scalable.h>
```

## Dimensions

Get values for layout dimensions based on display size:

```c
// Get a percentage (thousands) of the display width and height
const int half_w = scalable_x(500);
const int half_h = scalable_y(500);

// Get a GRect based only on percentage width/height
const GRect center_rect = scalable_grect(330, 330, 330, 330);
```

If a percentage isn't precise enough, the x/y or w/h can be nudged by some pixels:

```c
// A rect nudged by 5px in x and 2px in y
const GRect precise = scalable_nudge_xy(scalable_grect(100, 100, 200, 200), 5, 2);
```

## Centering

A GRect can be centered in either the X or Y axis, or both:

```c
const GRect r = scalable_grect(100, 500, 200, 200);

// Center horizontally
const GRect centered_h = scalable_center_x(r);

// Center vertically
const GRect centered_v = scalable_center_y(r);

// Or both!
const GRect centered = scalable_center(r);
```

## Fonts

Use different fonts for different display sizes, based on a category of 'small',
'medium', or 'large'.

```c
// Load fonts to use in each case
static GFont s_gothic_18, s_gothic_24;

// During init
s_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
s_gothic_24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);
```

Specify which fonts to use for each display size:

```c
// The 'small font' set ID
#define FONT_ID_SMALL 0

...

// The small font - regular screens use Gothic 18, Chalk N/A Emery uses Gothic 24
scalable_set_fonts(FONT_ID_SMALL, &s_gothic_18, NULL, &s_gothic_24);
```

During layout or drawing, simply use the font by ID:

```c
// Text in the vertical middle third!
graphics_context_set_text_color(ctx, GColorBlack);
graphics_draw_text(
  ctx,
  "This text should appear in the middle third on any platform or display size",
  scalable_get_font(FONT_ID_SMALL),
  scalable_grect(0, 330, 1000, 330),
  GTextOverflowModeTrailingEllipsis,
  GTextAlignmentCenter,
  NULL
);
```
