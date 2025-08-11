#pragma once

#include <pebble.h>

#include "data.h"
#include "../config.h"
#include "packet.h"
#include "../types.h"

#include "../common/contact_window.h"
#include "../common/responses_window.h"
#include "../common/sending_window.h"

void comm_init(uint32_t inbox, uint32_t outbox);

void comm_deinit();

void comm_request_contacts(ContactType type);

void comm_request_initial_contacts();

void comm_send_sms(ContactType contact_type, int contact_index, int response_index);

void comm_send_sms_with_transcription(ContactType contact_type, int contact_index, char *transcription);
