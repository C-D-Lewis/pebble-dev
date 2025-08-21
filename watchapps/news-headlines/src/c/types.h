#pragma once

#include <pebble.h>

typedef enum {
  SettingsTypeFirstLaunch = -1,
  SettingsTypeRegion = 0,
  SettingsTypeCategory,
  SettingsTypeNumStories,
  SettingsTypeFontSize,
  SettingsTypeAbout,
} SettingsType;

typedef enum {
  CategoryHeadlines = 0,
  CategoryWorld,
  CategoryUK,
  CategoryPolitics,
  CategoryHealth,
  CategoryEducation,
  CategoryScienceEnvironment,
  CategoryTechnology,
  CategoryEntertainment,

  CategoryCount
} Category;

typedef enum {
  FontSizeSmall = 18,
  FontSizeLarge = 24
} FontSize;

typedef enum {
  AppKeyTitle = 0,          // Story title
  AppKeyDescription,        // Story description
  AppKeyQuantity,           // Total number of stories
  AppKeyIndex,              // Which story this is
  AppKeyReady,              // JS is ready
  AppKeySettingsCategory,   // Selected category
  AppKeySettingsNumStories, // Number of stories to show
  AppKeySettingsRegion      // Selected region
} AppKey;

typedef enum {
  RegionUK = 0,
  RegionAfrica,
  RegionAsia,
  RegionEurope,
  RegionLatinAmerica,
  RegionMiddleEast,
  RegionUSAndCanada,
  RegionEngland,
  RegionNorthernIreland,
  RegionScotland,
  RegionWales,

  RegionCount
} Region;
