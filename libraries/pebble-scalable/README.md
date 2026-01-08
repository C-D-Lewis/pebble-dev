# pebble-scalable

Package aiming to make it easy to make scaling layouts for different display
sizes by defining their dimensions only once.

- [Setting up](#setting-up)
- [Layout](#layout)
- [Centering](#centering)
- [Fonts](#fonts)
- [Images](#images)

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

## Layout

Get values for layout dimensions based on display size:

> To convert existing layouts, simply divide the raw coordinate by either the
> width or height of the preferred screen size to get a percentage.
> For example, a Y coordinate of 40px - 40 / 168 = 0.238 = 23.8%, so the correct
> call would be `scalable_y(235)`.

```c
// Get a percentage (thousands) of the display width and height
const int half_w = scalable_x(500);
const int half_h = scalable_y(500);

// Get a GRect based only on percentage width/height
const GRect center_rect = scalable_grect(330, 330, 330, 330);
```

If a single percentage isn't precise enough, use 'per platform' variants to
specify values up to 5% (50/1000 thousands) in precision for each platform
(order is 'regular' shape, then 'emery' shape):

> In actuality, thousandths values should allow per-pixel precision until a
> display shape is larger than 1000px in any direction.

```c
// Larger only on Emery (x/y: 10%, w/h per platform)
const GRect emery = scalable_grect_pp(
  GRect(100, 100, 200, 200),
  GRect(100, 100, 250, 250)
);

// Further to the right and down on Emery
const int x = scalable_x_pp(110, 120);
const int y = scalable_y_pp(450, 480);
```

The library also exports constants:

* `PS_DISP_W` - current display width.
* `PS_DISP_H` - current display height.

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

Use different fonts for different display sizes, based on a category of your
choosing. First, load the fonts, or use system fonts:

> For custom fonts, scaling up the raw font size by the corresponding increase
> in display size (such as 14% for basalt > emery) may not be enough.
> Experiment!

```c
// Load fonts to use in each case
static GFont s_gothic_14, s_gothic_18, s_gothic_24, s_gothic_28;

// During init
s_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
s_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
s_gothic_24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);
s_gothic_28 = fonts_get_system_font(FONT_KEY_GOTHIC_28);
```

Next, define identifiers for each of your size categories:

```c
typedef enum {
  MFS_Small = 0,
  MFS_Medium,
  MFS_Large
} MyFontSizes;
```

Now set which font to use per-platform shape for each category:

```c
// The small font - regular screens use Gothic 14, Emery uses Gothic 18
scalable_set_fonts(MFS_Small, &s_gothic_14, &s_gothic_18);

// Same with larger categories
scalable_set_fonts(MFS_Medium, &s_gothic_18, &s_gothic_24);
scalable_set_fonts(MFS_Large, &s_gothic_24, &s_gothic_28);
```

During layout or drawing, simply use the font by ID:

```c
// Text in the vertical middle third!
graphics_context_set_text_color(ctx, GColorBlack);
graphics_draw_text(
  ctx,
  "This text should appear in the middle third on any platform or display size",
  scalable_get_font(MFS_Small),
  scalable_grect(0, 330, 1000, 330),
  GTextOverflowModeTrailingEllipsis,
  GTextAlignmentCenter,
  NULL
);
```

## Images

Images can be selected per-platform using the SDK's
[resource tagging](https://developer.rebble.io/guides/app-resources/platform-specific/)
system. From there, scalable layout functions can be used to set the position
and size. For example, `icon.png` and `icon~emery.png`.

```c
// Draw image centered at the bottom - image is 24x24px as a base size, so about 15% width
graphics_draw_bitmap_in_rect(
  ctx,
  s_icon_bitmap,
  scalable_center_x(scalable_grect(0, 850, 150, 150))
);
```

## TODO

- [x] Support per-platform (per shape) values
- [x] Solution for picking bitmaps (to work with their scalable GRects)
- [ ] Find solution to 'ever expanding args' problem for new platforms
- [ ] Chalk support
- [ ] Gabbro support
