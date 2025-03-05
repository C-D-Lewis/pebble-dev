#pragma once

#include <pebble.h>

typedef enum {
  SendingResultSuccess = 0,
  SendingResultSMSFailed = 1,
  SendingResultAppMessageFailed = 2
} SendingResult;

typedef enum {
  ContactTypeSMS = 0,
  ContactTypeCall = 1,
  ContactTypeIncoming = 2,
  ContactTypeFavorite = 3,
  ContactTypeSMSAndCall = 4,
  ContactTypeQuery = 5
} ContactType;

// Numerical lists means no key can be 0 - 30
typedef enum {
  AppKeyPacketType = 100,

  PacketTypeContactRequest = 30,
  AppKeyContactType = 31,           // Response will be numerical list with a type attached
  AppKeyContactQueryLetter = 32,
  AppKeyContactQueryTotal = 33,

  PacketTypeSMSRequest = 40,
  // AppKeyContactType
  AppKeyContactIndex = 41,
  AppKeyResponseIndex = 42,

  PacketTypeTranscriptSMS = 50,
  // AppKeyContactType
  // AppKeyContactIndex
  AppKeyTranscriptText = 51,

  PacketTypeError = 60,
  AppKeyErrorNoPermissions = 61,

  PacketTypeResponses = 70,  // Response will be numerical key list
  AppKeyTotalResponses = 71,

  PacketTypeSMSResult = 80,
  AppKeySMSResult = 81,
  AppKeySMSResultSuccess = 82,
  AppKeySMSResultFailed = 83

} AppKey;
