# pebble-pge-simple

Simple looping game engine for Pebble. Available on
[NPM](https://www.npmjs.com/package/pebble-pge-simple).

This is the simple version without all the other extra stuff the main module has.


## Installation

`pebble package install pebble-pge-simple`


## Features

- Automatic looping of developer-supplied per-frame logic and rendering.
- Up to 30 frames per second.
- Implement only your game code - `AppTimer`, `LayerUpdateProc`, `Clicks`,
  `Window` and `main` abstracted away.


## Basic Template App

To begin a new game watchapp, begin with the template file in `/docs/template.c.sample`:

> This is the bare minimum to make a looping game with PGE!

```
#include <pebble.h>
#include <pebble-pge-simple/pebble-pge-simple.h>

static void frame_logic() {
  // Per-frame game logic here
}

static void frame_draw(GContext *ctx) {
  // Per-frame game rendering here
}

static void button_click(int button_id, bool long_click) {
  // Process click events
}

void pge_init() {
  // Start the game
  pge_begin(frame_logic, frame_draw, button_click);
}

void pge_deinit() {
  // Finish the game
  pge_finish();
}
```
