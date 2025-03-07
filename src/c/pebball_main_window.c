#include <pebble.h>

#include "windows/pebball_game_window.h"
#include "utils/utils.h"

#define MAIN_MENU_SECTIONS 1
#define MAIN_MENU_ITEMS 2

// Splash vars
static Window *s_splash_window;
static TextLayer *s_splash_text;

// Main vars
static Window *s_main_menu;
static TextLayer *s_text_layer;
static SimpleMenuLayer *s_simple_main_menu;
static SimpleMenuSection s_simple_main_menu_sections[MAIN_MENU_SECTIONS];
static SimpleMenuItem s_simple_main_menu_items[MAIN_MENU_ITEMS];

void main_menu_transition(void * data) {
  // window_stack_push(s_main_menu, true);
  // window_stack_remove(s_splash_window, false);

  construct_outgoing_app_message(MESSAGE_KEY_WEBSOCKET_CONNECT_EVENT, 1);
}

static void main_menu_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_simple_main_menu = simple_menu_layer_create(bounds, s_main_menu, s_simple_main_menu_sections, MAIN_MENU_SECTIONS, NULL);
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_simple_main_menu));
}

static void main_menu_window_unload(Window *window) {
  text_layer_destroy(s_splash_text);
  simple_menu_layer_destroy(s_simple_main_menu);
}

static void splash_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_splash_text = text_layer_create(GRect(0, 62, bounds.size.w, 40));
  text_layer_set_text(s_splash_text, "Pebball!");
  text_layer_set_background_color(s_splash_text, GColorClear);
  text_layer_set_font(s_splash_text, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_splash_text, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_splash_text));
}

static void splash_window_unload(Window *window) {
  text_layer_destroy(s_splash_text);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *ack_event_tuple = dict_find(iterator, MESSAGE_KEY_WEBSOCKET_ACK_EVENT);
  Tuple *pitch_reach_time_tuple = dict_find(iterator, MESSAGE_KEY_PITCH_REACH_TIME);

  if (ack_event_tuple) {
    int converted_ack_event_tuple = ack_event_tuple->value->int8;

    APP_LOG(APP_LOG_LEVEL_INFO, "Got: %d", converted_ack_event_tuple);

    switch (converted_ack_event_tuple) {
      case 17:
        window_stack_push(s_main_menu, true);
        window_stack_remove(s_splash_window, false);
        break;
      case 18:
        batting_calibration_push();
        window_stack_remove(s_progress_hud_window, false);
        break;
      case 19:
        break;
      case 20:
        if (pitch_reach_time_tuple) {
          int converted_pitch_reach_time_tuple = pitch_reach_time_tuple->value->int8;

          progress_hud_push();
          window_stack_remove(s_batting_calibration_window, false);

          swing_event_timer = app_timer_register((converted_pitch_reach_time_tuple * 1000), batter_swing_event, NULL);
        }
        break;
      case 21:
        break;
    }
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void prv_init(void) {
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);

  s_splash_window = window_create();

  window_set_background_color(s_splash_window, GColorGreen);

  window_set_window_handlers(s_splash_window, (WindowHandlers) {
    .load = splash_window_load,
    .unload = splash_window_unload,
  });

  s_main_menu = window_create();

  s_simple_main_menu_items[0] = (SimpleMenuItem) {
    .title = "Start Game",
    .subtitle = NULL,
    .icon = NULL,
    .callback = game_start_transition
  };

  s_simple_main_menu_items[1] = (SimpleMenuItem) {
    .title = "Options",
    .subtitle = NULL,
    .icon = NULL,
    .callback = progress_hud_push
  };

  s_simple_main_menu_sections[0] = (SimpleMenuSection) {
    .title = NULL,
    .items = s_simple_main_menu_items,
    .num_items = MAIN_MENU_ITEMS
  };

  window_set_window_handlers(s_main_menu, (WindowHandlers) {
    .load = main_menu_window_load,
    .unload = main_menu_window_unload,
  });

  window_stack_push(s_splash_window, true);

  app_timer_register(1000, main_menu_transition, NULL);
}

static void prv_deinit(void) {
  window_destroy(s_splash_window);
  window_destroy(s_main_menu);
  window_destroy(s_batting_calibration_window);
  window_destroy(s_progress_hud_window);

  accel_data_service_unsubscribe();
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
