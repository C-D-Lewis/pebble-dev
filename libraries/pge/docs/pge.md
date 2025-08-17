# PGE Documentation

## Starting and Stopping Your Game

1. Copy this repo to your project's `src` directory.

2. Add `#include "pge/pge.h"` to your project's main C file.

3. Implement one of `PGELogicHandler`, `PGERenderHandler` and
   `PGEClickHandler`:

        void logic() {

        }

        void draw(GContext *ctx) {

        }

        // Optional, can be NULL if only using pge_get_button_state()
        void click(int button_id, bool long_click) {

        }

4. Implement `pge_init()` and `pge_deinit()` for your app's setup and teardown
   respectivly, calling `pge_begin()` in `pge_init()`, supplying the callbacks
   from 3.

        pge_begin(logic, draw, click);

   Optionally keep a reference to the returned `Window` for adding other Layers:

        s_some_window = pge_get_window();

5. Furnish `logic()`, `render()` and `click()` to implement your own game items
   and logic.

6. Destroy the engine in `pge_deinit()` implementation:

        void pge_deinit() {
          // Destroy all game resources
          pge_finish();
        }


## Using Highscores

PGE supports basic highest-score recording that persists across app launches:

        int score = 9001;
        pge_title_set_highscore(score);

        int highscore = pge_title_get_highscore();

## Set a Game Background

You can easily set a background image for your game:

        pge_set_background(RESOURCE_ID_GAME_BACKGROUND);
