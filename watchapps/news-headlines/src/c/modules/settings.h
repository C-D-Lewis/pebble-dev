#pragma once

#include <pebble.h>

#include "../types.h"
#include "../windows/settings_window.h"

// Initialize settings
void settings_init();

// Set/get category
void settings_set_category(Category category);
Category settings_get_category();

// Set/get pin setting
void settings_set_subscribed_type(PinSubscriptionType status);
PinSubscriptionType settings_get_subscribed_type();

// Set/get num stories
void settings_set_number_of_stories(int number);
int settings_get_number_of_stories();

// Seg/get font size
void settings_set_font_size(FontSize size);
FontSize settings_get_font_size();

// UI
char* settings_get_category_string();
char* settings_get_subscribed_string();
char* settings_get_num_stories_string();
char* settings_get_font_size_string();

// Region
void settings_set_region(Region region);
Region settings_get_region();
char* settings_get_region_string();
