#pragma once

#include <pebble.h>

#define SPLASH_LINE_HEIGHT 3
#define SPLASH_LOGO_Y 100

/**
 * Used to decide what to do after splash is complete
 * This MUST call pge_begin() or push another Window or the app will exit
 */
typedef void (PGESplashDoneHandler)();

/**
 * Show the PGE Splash animated Window
 * Represent!
 */
void pge_splash_show(PGESplashDoneHandler *handler);