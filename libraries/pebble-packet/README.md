# pebble-packet

Simple wrapper for AppMessage integer and string types with built in error
reporting. Never directly use `DictionaryIterator` or `Tuple` again!

Available on [NPM](https://www.npmjs.com/package/pebble-packet).

## How to use

1. Install the Pebble packages:

  ```
  $ pebble package install pebble-packet
  $ pebble package install pebble-events
  ```

2. Add the includes at the top of your source.

  ```c
  #include <pebble-packet/pebble-packet.h>
  #include <pebble-events/pebble-events.h>
  ```

3. Ensure `AppMessage` is open via `pebble-events`:

  ```c
  events_app_message_request_inbox_size(512);
  events_app_message_request_outbox_size(512);
  events_app_message_open();
  ```

4. Begin, build, and send a packet:

  ```c
  if (packet_begin()) {
    packet_put_integer(AppKeyInteger, 42);
    packet_put_string(AppKeyString, "Don't talk to me about life.");
    packet_send(failed_handler);
  }
  ```

5. Get data from a received dictionary:

  ```c
  // in your setup AFTER the app message open call
  events_app_message_register_inbox_received(inbox_received_handler, NULL);
  ```

  ```c
  
  static void in_recv_handler(DictionaryIterator *iter, void *context) {
    if(packet_contains_key(iter, MESSAGE_KEY_Integer)) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Got int: %d", packet_get_integer(iter, MESSAGE_KEY_Integer));
    }

    if(packet_contains_key(iter, MESSAGE_KEY_String)) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Got string: %s", packet_get_string(iter, MESSAGE_KEY_String));
    }

    if(packet_contains_key(iter, MESSAGE_KEY_Boolean)) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Got int: %s", packet_get_boolean(iter, MESSAGE_KEY_Boolean) ? "true" : "false");
    }

    APP_LOG(APP_LOG_LEVEL_INFO, "Size: %d", packet_get_size(iter));
  }
  ```


## Documentation

See `include/pebble-packet.h` for function documentation.


## Changelog

**1.0**
- Initial version

**1.1**
- Add `packet_get_size()`, `packet_contains_key()`, `packet_put_boolean()`, 
  `packet_get_boolean()`, `packet_get_integer()`, and `packet_get_string()`.

**1.2**
- Build for Diorite platform on SDK 4.0

**1.3**
- Add failed handler so client apps can retry their packets.

**1.3.1**
- Add `NULL` check for retry callbacks.

**1.3.2**
- Build for Emery
