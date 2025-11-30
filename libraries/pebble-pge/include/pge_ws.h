/**
 * PGE Multiplayer module using WebSockets
 */

#pragma once

#include <pebble.h>

#define PGE_WS_LOGS true

#define PGE_WS_NOT_FOUND -7678942 // Unlikely number

// Developer-usable keys, MUST be declared in appinfo.json
#define PGE_WS_KEY_0  0
#define PGE_WS_KEY_1  1
#define PGE_WS_KEY_2  2
#define PGE_WS_KEY_3  3
#define PGE_WS_KEY_4  4
#define PGE_WS_KEY_5  5
#define PGE_WS_KEY_6  6
#define PGE_WS_KEY_7  7
#define PGE_WS_KEY_8  8
#define PGE_WS_KEY_9  9
#define PGE_WS_KEY_10 10
#define PGE_WS_KEY_11 11
#define PGE_WS_KEY_12 12
#define PGE_WS_KEY_13 13
#define PGE_WS_KEY_14 14
#define PGE_WS_KEY_15 15
#define PGE_WS_NUM_KEYS 16  // Update this if you add more keys to appinfo.json

#define PGE_WS_URL        100  // Send URL, receive confirmation
#define PGE_WS_CLIENT_ID  101  // Receive client ID from server
#define PGE_WS_READY      102  // PebbleKit JS is ready

typedef enum {
  PGEWSConnectionStateDisconnected,
  PGEWSConnectionStateConnecting,
  PGEWSConnectionStateConnected
} PGEWSConnectionState;

// Handler for connection result
typedef void (PGEWSConnectedHandler)(bool successful);

// Handlers for received values. Use pge_ws_get_value() to get your data
typedef void (PGEWSReceivedHandler)();

/**
 * Connect to server when JS ready event is fired.
 * NOTE: Must be called during app initialization (before app_event_loop() is called)
 * url - The URL of the remote server. E.g.: "ws://localhost:5000". Must be long-lived.
 * handler - The PGEWSConnectedHandler called when the connection attempt returns
 * recv_handler - The PGEWSReceivedHandler called when at least one PGE_WS_KEY has been received
 */
void pge_ws_begin(char *url, PGEWSConnectedHandler *handler, PGEWSReceivedHandler *recv_handler);

/**
 * Returns true if connected, else false
 */
bool pge_ws_is_connected();

/**
 * Returns the client ID from the server
 */
int pge_ws_get_client_id();

/*
 * Begin creating a PGE WS packet
 * Returns true if creation was successful
 */
bool pge_ws_packet_begin();

/**
 * Send the PGE WS packet
 * Returns true if send was successful
 */
bool pge_ws_packet_send();

/**
 * Add an int to the dictionary
 * key - The integer key associated with the value
 * value - The value to be added
 * Returns true if the add succeeded, false if it failed or pge_ws_packet_begin() has not been called
 */
bool pge_ws_add_int(int key, int value);

/**
 * Gets a value from the incoming packet, if called in a PGEWSReceivedHandler
 * Returns the value, PGE_WS_NOT_FOUND if not found
 */
int pge_ws_get_value(int key);
