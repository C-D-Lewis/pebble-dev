#include "settings.h"

static void write_defaults() {
  // Set default settings
  settings_set_category(CategoryHeadlines);
  settings_set_subscribed_type(PinSubscriptionTypeNotSubscribed);
  settings_set_number_of_stories(10);
  settings_set_font_size(FontSizeLarge);
  settings_set_region(RegionUK);
}

void settings_init() {
  if(!persist_exists(SettingsTypeFirstLaunch)) {
    persist_write_bool(SettingsTypeFirstLaunch, false);

    write_defaults();
  }

  // Nuke versions
  const int nuked_v_3_6 = 453786;
  if(!persist_exists(nuked_v_3_6)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Nuking persist for new version...");
    const int persist_max = 220; // desc base key (200) + 20 max stories
    for(int i = 0; i < persist_max; i++) {
      persist_delete(i);
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Nuke complete");
    persist_write_bool(nuked_v_3_6, true);
    write_defaults();
  }
}

void settings_set_category(Category category) {
  persist_write_int(SettingsTypeCategory, category);
}

Category settings_get_category() {
  return persist_read_int(SettingsTypeCategory);
}

void settings_set_number_of_stories(int number) {
  persist_write_int(SettingsTypeNumStories, number);
}

int settings_get_number_of_stories() {
  return persist_read_int(SettingsTypeNumStories);
}

void settings_set_font_size(FontSize size) {
  persist_write_int(SettingsTypeFontSize, size);
}

FontSize settings_get_font_size() {
  return persist_read_int(SettingsTypeFontSize);
}

char* settings_get_category_string() {
  switch(settings_get_category()) {
    case CategoryHeadlines:          return "Headlines";
    case CategoryWorld:              return "World";
    case CategoryUK:                 return "UK";
    case CategoryPolitics:           return "Politics";
    case CategoryHealth:             return "Health";
    case CategoryEducation:          return "Education";
    case CategoryScienceEnvironment: return "Science & Environment";
    case CategoryTechnology:         return "Technology";
    case CategoryEntertainment:      return "Entertainment & Arts";
    default: return "Unknown!";
  }
}

char* settings_get_subscribed_string() {
  switch(settings_get_subscribed_type()) {
    case PinSubscriptionTypeNotSubscribed: return "Not subscribed";
    case PinSubscriptionTypeSubscribed:    return "Subscribed";
    default:
      // Could have been with notif
      settings_set_subscribed_type(PinSubscriptionTypeNotSubscribed);
      return settings_get_subscribed_string();
      break;
  }
}

char* settings_get_num_stories_string() {
  switch(settings_get_number_of_stories()) {
    case 10: return "10 stories";
    case 20: return "20 stories";
    default: 
      settings_set_number_of_stories(10);
      return "10 stories (default)";
  }
}

char* settings_get_font_size_string() {
  switch(settings_get_font_size()) {
    case FontSizeSmall: return "Small (18)";
    case FontSizeLarge: return "Large (24)";
    default: 
      settings_set_font_size(FontSizeLarge);
      return "Large (24) (default)";
  }
}

void settings_set_subscribed_type(PinSubscriptionType status) {
  persist_write_int(SettingsTypePins, (int)status);
}

PinSubscriptionType settings_get_subscribed_type() {
  return (PinSubscriptionType)persist_read_int(SettingsTypePins);
}

void settings_set_region(Region region) {
  persist_write_int(SettingsTypeRegion, region);
}

Region settings_get_region() {
  return (Region)persist_read_int(SettingsTypeRegion);
}

char* settings_get_region_string() {
  switch(settings_get_region()) {
    case RegionUK: return "UK";
    case RegionAfrica: return "Africa";
    case RegionAsia: return "Asia";
    case RegionEurope: return "Europe";
    case RegionLatinAmerica: return "Latin America";
    case RegionMiddleEast: return "Middle East";
    case RegionUSAndCanada: return "US and Canada";
    case RegionEngland: return "England";
    case RegionNorthernIreland: return "Northern Ireland";
    case RegionScotland: return "Scotland";
    case RegionWales: return "Wales";
    default: 
      settings_set_region(RegionUK);
      return "UK (default)";
  }
}
