#pragma once

#include <pebble.h>

void construct_outgoing_app_message(uint8_t message_id, int message_value);
void set_webserver_url(const char *webserver_url);