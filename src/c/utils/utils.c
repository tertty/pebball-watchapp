#include "utils.h"

void construct_outgoing_app_message(uint8_t message_id, int message_value) {
    DictionaryIterator *out_iter;

    AppMessageResult result = app_message_outbox_begin(&out_iter);

    if(result == APP_MSG_OK) {
        dict_write_uint8(out_iter, message_id, message_value);
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
    }

    result = app_message_outbox_send();

    if(result != APP_MSG_OK) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
}