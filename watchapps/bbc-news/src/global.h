#include <pebble.h>

#ifndef GLOBAL_H
#define GLOBAL_H

//Libs
#include "libs/cl_util.h"
#include "libs/cl_progressbar.h"

//Components
#include "detail_window.h"
#include "title_window.h"

//Keys
#define KEY_TITLE 0
#define KEY_DESCRIPTION 1
#define KEY_QUANTITY 2
#define KEY_DETAIL_SIZE 3

//Configuration
#define CHROME_HEIGHT 16
#define MAX_LENGTH 32
#define MAX_STORIES 30

//Multiple access
//splash_window, titles_window, detail_window
extern char 
	*titles[MAX_STORIES],
	*descriptions[MAX_STORIES];
extern int
	current_item;

//splash_window, titles_window
extern MenuLayer *titles_menu_layer;
extern int
	max_quantity,
	current_quantity;

#endif