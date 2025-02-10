# notif-layer

Simple top-down notification Layer subclass for Pebble.

![](screenshots/screenshot.gif)


## How to use

1. Create as with a normal `Layer`:

        NotifLayer *notif_layer = notif_layer_create();

2. Add the `NotifLayer` to the `Window` it will display within:

        notif_layer_add_to_window(some_parent_window);

3. Show a notification. Long text will automatically scroll:

        notif_layer_show(notif_layer, "This is an example notification!");

4. Destroy the `NotifLayer` when the parent `Window` is exiting:

        notif_layer_destroy(notif_layer);
