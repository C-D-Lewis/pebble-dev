#include "comm.h"

static void process_tuple(Tuple *t, DictionaryIterator *iter) {
  Tuple *packet_type_t = dict_find(iter, AppKeyPacketType);
  if(packet_type_t) {
    int packet_type = t->value->int32;
    switch(packet_type) {
      case PacketTypeContactRequest: {
        int type = dict_find(iter, AppKeyContactType)->value->int32;
        switch(type) {
          case ContactTypeSMSAndCall:
            // We got both
            data_set_contact_name(ContactTypeSMS, 0, dict_find(iter, ContactTypeSMS)->value->cstring);
            data_set_contact_name(ContactTypeCall, 0, dict_find(iter, ContactTypeCall)->value->cstring);

            contact_window_push();
            break;
        }
      } break;
      default: break;
    }
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);
  while(t) {
    process_tuple(t, iter);
    t = dict_read_next(iter);
  }
}

void comm_init(uint32_t inbox, uint32_t outbox) {
  app_message_open(inbox, outbox);
}

void comm_deinit() { }

static void prepare() {
  app_message_register_inbox_received(inbox_received_handler);
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
}

void comm_request_contacts(ContactType type) {
  prepare();

  if(packet_begin()) {
    packet_set_type(PacketTypeContactRequest);
    packet_add_int(AppKeyContactType, type);
    packet_send();
  }
}

void comm_request_initial_contacts() {
  prepare();

  if(packet_begin()) {
    packet_set_type(PacketTypeContactRequest);
    packet_add_int(AppKeyContactType, ContactTypeSMSAndCall);
    packet_add_int(AppKeyContactIndex, 0);  // Irrelevant
    packet_send();
  }
}

void comm_send_sms(ContactType contact_type, int contact_index, int response_index) {
  prepare();

  if(packet_begin()) {
    packet_set_type(PacketTypeSMSRequest);
    packet_add_int(AppKeyContactType, contact_type);
    packet_add_int(AppKeyContactIndex, contact_index);
    packet_add_int(AppKeyResponseIndex, response_index);
    packet_send();
  }
}

void comm_send_sms_with_transcription(ContactType contact_type, int contact_index, char *transcription) {
  prepare();

  if(packet_begin()) {
    packet_set_type(PacketTypeTranscriptSMS);
    packet_add_int(AppKeyContactType, contact_type);
    packet_add_int(AppKeyContactIndex, contact_index);
    packet_add_string(AppKeyTranscriptText, transcription);
  }
}
