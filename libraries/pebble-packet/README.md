# pebble-packet

Simple wrapper for AppMessage integer, boolean, and string types with built in
error reporting. Never directly use `DictionaryIterator` or `Tuple` again!

Available on [NPM](https://www.npmjs.com/package/pebble-packet).

* [Setup](#setup)
* [How to use](#how-to-use)
* [Documentation](#documentation)

## Setup

Install the Pebble package:

```
$ pebble package install pebble-packet
```

Add the include at the top of your source.

```c
#include <pebble-packet/pebble-packet.h>
```

Prepare the library, and ensure `AppMessage` is open and ready to receive:

```c
packet_init();

app_message_register_inbox_received(inbox_received_handler);
app_message_open(256, 256);
```

## How to use

Begin, build, and send a packet:

```c
if (packet_begin()) {
  packet_put_integer(AppKeyInteger, 42);
  packet_put_string(AppKeyString, "Don't talk to me about life.");

  // Send, with optional handler for send failure
  packet_send(failed_handler);
}
```

Get data from a received dictionary:

```c
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Size: %d", packet_get_size(iter));

  if (packet_contains_key(iter, MESSAGE_KEY_Integer)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Got int: %d", packet_get_integer(iter, MESSAGE_KEY_Integer));
  }

  if (packet_contains_key(iter, MESSAGE_KEY_String)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Got string: %s", packet_get_string(iter, MESSAGE_KEY_String));
  }

  if (packet_contains_key(iter, MESSAGE_KEY_Boolean)) {
    APP_LOG(
      APP_LOG_LEVEL_INFO,
      "Got boolean: %s",
      packet_get_boolean(iter, MESSAGE_KEY_Boolean) ? "true" : "false"
    );
  }
}
```

You can use `packet_result_to_string()` to get a readable string version of an
integer `AppMessageResult`:

```c
const AppMessageResult r = app_message_open(256, 256);
if (r != APP_MSG_OK) {
  // APP_MSG_OUT_OF_MEMORY -> "Out of memory" etc.
  APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to open AppMessage: %s", packet_result_to_string(r));
}
```

## Documentation

See `include/pebble-packet.h` for function documentation.
