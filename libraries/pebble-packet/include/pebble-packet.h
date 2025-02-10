#pragma once

#include <pebble.h>

// Callback when a send failed, but not when attempting the send.
typedef void(PacketFailedCallback)(void);

// Begin a new packet. Must be called before putting anything in it.
// Returns:
//   bool - true if the packet was successfully initialised, false otherwise.
//          Check app logs to see any error reason.
bool packet_begin();

// Put an integer into the packet.
// Parameters:
//   key   - The tuple's key.
//   value - The tuple's integer value.
// Returns:
//   bool - true if the integer was written to the packet, false otherwise.
bool packet_put_integer(int key, int value);

// Put a string into the packet.
// Parameters:
//   key    - The tuple's key.
//   string - The tuple's string value.
// Returns:
//   bool - true if the string was written to the packet, false otherwise.
bool packet_put_string(int key, char *string);

// Put a boolean value into the packet. Encoded as 1 (true) or 0 (false) for JS.
// Parameters:
//   key - The tuple's key.
//   b   - The tuple's boolean value.
// Returns:
//   bool - true if the boolean was written to the packet, false otherwise.
bool packet_put_boolean(int key, bool b);

// Send the packet, and optionally retry if an AppMessage failure occurs.
// Parameters:
//   cb - Callback to call when sending fails. Use this to rebuild and retry your message.
// Returns:
//   bool - true if the packet was successfully sent, false otherwise.
//          Check app logs to see any error reason.
bool packet_send(PacketFailedCallback *cb);

// Get the size of a received dictionary in bytes.
// Parameters:
//   inbox_iter - The DictionaryIterator received in AppMessageInboxReceived.
// Returns:
//   int - The size of the DictionaryIterator in bytes.
int packet_get_size(DictionaryIterator *inbox_iter);

// Determine if a received dictionary contains a key-value pair
// Parameters:
//   inbox_iter - The DictionaryIterator received in AppMessageInboxReceived.
//   key        - The key to look for.
// Returns:
//   bool - true if the key is present, false otherwise.
bool packet_contains_key(DictionaryIterator *inbox_iter, int key);

// Get an integer from a received dictionary.
// Parameters:
//   inbox_iter - The DictionaryIterator received in AppMessageInboxReceived.
//   key        - The key to look for.
// Returns:
//   int - The integer contained for the key specified, 0 if not present.
int packet_get_integer(DictionaryIterator *inbox_iter, int key);

// Get a string from a received dictionary.
// Parameters:
//   inbox_iter - The DictionaryIterator received in AppMessageInboxReceived.
//   key        - The key to look for.
// Returns:
//   char* - A pointer to the string, valid for the duration of AppMessageInboxReceived.
//           NULL if not present.
char* packet_get_string(DictionaryIterator *inbox_iter, int key);

// Get a boolean from a received dictionary. Should be encoded as 1 (true) or 0 (false) in JS.
// Parameters:
//   inbox_iter - The DictionaryIterator received in AppMessageInboxReceived.
//   key        - The key to look for.
// Returns:
//   bool - The boolean contained for the key specified, false if not present.
bool packet_get_boolean(DictionaryIterator *inbox_iter, int key);
