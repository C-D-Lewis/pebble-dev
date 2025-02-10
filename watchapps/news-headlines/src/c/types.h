#pragma once

#include <pebble.h>

typedef enum {
  SettingsTypeFirstLaunch = -1,
  SettingsTypeRegion = 0,
  SettingsTypeCategory,
  SettingsTypePins,
  SettingsTypeNumStories,
  SettingsTypeFontSize,
  SettingsTypeStatus,
  SettingsTypeAbout,
  SettingsTypeDonate,
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
  ServerStatusWaiting,
  ServerStatusUp,
  ServerStatusDown,
  ServerStatusTimeout
} ServerStatus;

typedef enum {
  PinSubscriptionTypeNotSubscribed = 0,
  PinSubscriptionTypeSubscribed
} PinSubscriptionType;

typedef enum {
  AppKeyTitle = 0,               // Story title
  AppKeyDescription,             // Story description
  AppKeyQuantity,                // Total number of stories
  AppKeyIndex,                   // Which story this is
  AppKeyFailed = 5,              // The pin story was not found
  AppKeyStatus = 9,              // Query pin server status
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
  AppKeySettingsCategory = 6,
  AppKeySettingsSubscription,
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
