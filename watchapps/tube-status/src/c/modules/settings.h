#pragma once

#include <pebble.h>

typedef enum {
  SubscriptionStateUnsubscribed = 0,
  SubscriptionStateSubscribed,  // You get both, it's easier

  SubscriptionStateMax
} SubscriptionState;

void settings_set_subscription_state(SubscriptionState state);
SubscriptionState settings_get_subscription_state();

char* settings_get_subscription_string();

