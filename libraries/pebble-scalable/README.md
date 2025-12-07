# pebble-scalable

Package aiming to make it easy to make scaling layouts for different screen
sizes by defining their dimension only one.

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

Get values for layout dimensions based on screen size:

```c
// Get a percentage of the screen width and height
const int half_w = scalable_x(50);
const int half_h = scalable_y(50);

// Get a GRect based only on percentage width/height
const GRect center_rect = scalable_grect(33, 33, 33, 33);
```

If a percentage isn't precise enough, the x/y or w/h can be nudged by some pixels:

```c
// A rect nudged by 5px in x and 2px in y
const GRect precise = scalable_nudge_xy(scalable_grect(10, 10, 20, 20), 5, 2);
```

## Centering

A GRect can be centered in either the X or Y axis, or both:

```c
const GRect r = scalable_grect(10, 50, 20, 20);

// Center horizontally
const GRect centered_h = scalable_center_x(r);

// Center vertically
const GRect centered_v = scalable_center_y(r);

// Or both!
const GRect centered = scalable_center(r);
```

## Fonts

Use different fonts for different screen sizes, based on a category of 'small',
'medium', or 'large'.

```c
// Load fonts to use in each case
static GFont s_gothic_18, s_gothic_24;

// During init
s_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
s_gothic_24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);
```

Specify which screen sizes should use which fonts for each size

```c
// Regular screens use Gothic 18, Chalk is N/A, Emery uses Gothic 24
scalable_set_medium_fonts(&s_gothic_18, NULL, &s_gothic_24);
```

During layout or drawing, simply use the font by its size:

```c
// Text in the vertical middle third!
graphics_context_set_text_color(ctx, GColorBlack);
graphics_draw_text(
  ctx,
  "This text should appear in the middle third on any platform or screen size",
  scalable_get_medium_font(),
  scalable_grect(0, 33, 100, 33),
  GTextOverflowModeTrailingEllipsis,
  GTextAlignmentCenter,
  NULL
);
```

## TODO

- [ ] What other things need to be set based on size and/or platform aside from position and font size?
