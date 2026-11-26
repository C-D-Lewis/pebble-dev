#pragma once

// README: Cleanup has been done, move to multi-source model requires:
//
//           - Generalized 'type' concept
//           - No ordering anywhere except JS
//           - Multi-source selection in Clay config page
//           - Config page!

// Inbox size
#define COMM_INBOX_SIZE 1024
// Outbox size
#define COMM_OUTBOX_SIZE 128
// Max reason length
#define REASON_MAX_LEN 512

// Order is very important - must match JS side
typedef enum {
  LineTypeBakerloo = 0,
  LineTypeCentral,
  LineTypeCircle,
  LineTypeDistrict,
  LineTypeDLR,
  LineTypeElizabeth,
  LineTypeHammersmithAndCity,
  LineTypeJubilee,
  LineTypeLiberty,
  LineTypeLioness,
  LineTypeMetropolitan,
  LineTypeMildmay,
  LineTypeNorthern,
  LineTypePiccadilly,
  LineTypeSuffragette,
  LineTypeVictoria,
  LineTypeWaterlooAndCity,
  LineTypeWeaver,
  LineTypeWindrush,

  LineTypeMax
} LineType;

typedef struct {
  int type;
  char state[32];
  char reason[REASON_MAX_LEN];
} LineData;
