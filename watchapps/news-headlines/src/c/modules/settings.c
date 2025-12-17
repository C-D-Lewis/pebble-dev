#include "settings.h"

static void write_defaults() {
  // Set default settings
  settings_set_category(CategoryHeadlines);
  settings_set_number_of_stories(10);
  settings_set_region(RegionUK);
}

void settings_init() {
  if (!persist_exists(SettingsTypeFirstLaunch)) {
    persist_write_bool(SettingsTypeFirstLaunch, false);

    write_defaults();
  }

  // Nuke versions
  const int nuked_v_3_6 = 453786;
  const int nuked_v_3_7 = 453787;
  if (!persist_exists(nuked_v_3_6) || !persist_exists(nuked_v_3_7)) {
    const int persist_max = 32;
    for(int i = 0; i < persist_max; i++) {
      persist_delete(i);
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Nuke complete");
    persist_write_bool(nuked_v_3_6, true);
    persist_write_bool(nuked_v_3_7, true);
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

char* settings_get_category_string() {
  switch(settings_get_category()) {
    case CategoryHeadlines:          return "Headlines";
    case CategoryWorld:              return "World";
    case CategoryUK:                 return "UK";
    case CategoryPolitics:           return "Politics";
    case CategoryHealth:             return "Health";
    case CategoryEducation:          return "Education";
    case CategoryScienceEnvironment: return "Science & Env.";
    case CategoryTechnology:         return "Technology";
    case CategoryEntertainment:      return "Entertainment";
    default: return "Unknown!";
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
