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
  AppKeyTitle = 0,               // Story title
  AppKeyDescription,             // Story description
  AppKeyQuantity,                // Total number of stories
  AppKeyIndex,                   // Which story this is
  AppKeyFailed,              // The pin story was not found
  AppKeyReady,                   // JS is ready
  AppKeyImageFailed,             // Failed to download image
  AppKeyImage,                   // Fetch an image
  AppKeyOffset,                  // Offset in an image
  AppKeyData,                    // Actual image data
  AppKeyImageDone,               // Image should be complete
  AppKeyChunkSize,               // Size of incoming image chunk
  AppKeyImageAvailabilityString  // String of '0' and '1' representing which stories have images
} AppKey;

typedef enum {
  AppKeySettingsCategory = 20,
  AppKeySettingsNumStories,
  AppKeySettingsRegion
} AppKeySettings;

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
