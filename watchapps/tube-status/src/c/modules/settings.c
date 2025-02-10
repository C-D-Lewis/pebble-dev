#include "settings.h"

static SubscriptionState s_state;

void settings_set_subscription_state(SubscriptionState state) {
  s_state = state;
}

SubscriptionState settings_get_subscription_state() {
  return s_state;
}

char* settings_get_subscription_string() {
  switch(s_state) {
    case SubscriptionStateUnsubscribed: return "Not Subscribed";
    case SubscriptionStateSubscribed:       
      return "Pins & Notifications";
    default: return "UNKNOWN SUBS STATE";
  }
}
