#include "pebball_game_window.h"
#include "../utils/utils.h"

#define ACCEL_SAMPLES 3
#define MARGIN_OF_ERROR 50

// Batting calibration
static TextLayer *s_batter_up_text;

// Progress HUD
static TextLayer *s_hits_status_text;
static TextLayer *s_misses_status_text;
static TextLayer *s_combo_status_text;
int hit_count = 0;
int miss_count = 0;
int combo_count = 0;

// Accel
bool is_calibrating = true;
bool has_swung = false;
uint8_t calibration_count = 0;

time_t batter_swing_timer_event_timestamp;
time_t batter_swing_event_timestamp;

static int batter_swing_reaction_score() {
    int timestamp_difference = batter_swing_event_timestamp - batter_swing_timer_event_timestamp;

    // APP_LOG(APP_LOG_LEVEL_INFO, "time difference: %d", timestamp_difference);

    if ((timestamp_difference > 0) && (timestamp_difference < 10)) {
        hit_count++;
        combo_count++;

        return timestamp_difference;
    } else {
        miss_count++;
        combo_count = 0;
    
        return -1;
    }
}

void batter_swing_event(void * data) {
    vibes_short_pulse();
    batter_swing_timer_event_timestamp = time(NULL);
}

static void miss_timeout_event(void * data) {
    static const uint32_t const segments[] = { 200, 100, 200, 100, 200 };
    VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
    };
    vibes_enqueue_custom_pattern(pat);

    construct_outgoing_app_message(MESSAGE_KEY_WEBSOCKET_BATTER_MISS_EVENT, 1);

    miss_count++;
    combo_count = 0;

    batting_calibration_push();
}

static void accel_batting_calibration_handler(AccelData *data, uint32_t num_samples) {
    // Read sample 0's x, y, and z values
    int16_t x = data[0].x;
    int16_t y = data[0].y;
    int16_t z = data[0].z;

    // Determine if the sample occured during vibration, and when it occured
    uint64_t timestamp = data[0].timestamp;
    bool did_vibrate = false;

    for (int i = 0; i < ACCEL_SAMPLES; i++) {
        if (data[i].did_vibrate) {
            did_vibrate = true;
            // APP_LOG(APP_LOG_LEVEL_INFO, "DID VIBRATE!!!");
        }
    }

    if (!did_vibrate) {
        if (is_calibrating) {
            // APP_LOG(APP_LOG_LEVEL_INFO, "Batting Calibration - t: %llu, x: %d, y: %d, z: %d, cal: %d, pos: %d", data[0].timestamp, data[0].x, data[0].y, data[0].z, calibration_count, wrist_position);

            has_swung = false;

            // Left wrist calibration, Right wrist calibration, else reset
            if (!wrist_position && (((data[0].x <= -100) && (data[1].x <= -100)) && (abs(data[0].x - data[1].x) <= 50) && (abs(data[0].y - data[1].y) <= 50) && (abs(data[0].z - data[1].z) <= 50))) {
                calibration_count++;
            } else if (wrist_position && (((data[0].x >= 100) && (data[1].x >= 100)) && (abs(data[0].x - data[1].x) <= 50) && (abs(data[0].y - data[1].y) <= 50) && (abs(data[0].z - data[1].z) <= 50))) {
                calibration_count++;
            } else {
                calibration_count = 0;
            }

            if (calibration_count >= 10) {
                construct_outgoing_app_message(MESSAGE_KEY_WEBSOCKET_BATTER_POSITION_READY_EVENT, 1);

                // text_layer_set_text(s_batter_up_text, "Batter up!");

                vibes_double_pulse();

                calibration_count = 0;
            }
        } else if (!has_swung) {
            // APP_LOG(APP_LOG_LEVEL_INFO, "Swing Event - t: %llu, x: %d, y: %d, z: %d", timestamp, x, y, z);

            // Left wrist swing, right wrist swing
            if (!wrist_position && ((data[0].x > 150) && (data[1].x > 150) && (data[2].x > 150))) {
                batter_swing_event_timestamp = time(NULL);

                int batter_swing_score = batter_swing_reaction_score();

                if (batter_swing_score > -1) {
                    construct_outgoing_app_message(MESSAGE_KEY_WEBSOCKET_BATTER_HIT_EVENT, batter_swing_score);
                } else {
                    construct_outgoing_app_message(MESSAGE_KEY_WEBSOCKET_BATTER_MISS_EVENT, 1);
                }

                has_swung = true;

                app_timer_cancel(swing_event_timer);
                app_timer_cancel(miss_timeout_event_timer);

                vibes_long_pulse();
            } else if (wrist_position && ((data[0].x < -150) && (data[1].x < -150) && (data[2].x < -150))) {
                batter_swing_event_timestamp = time(NULL);

                int batter_swing_score = batter_swing_reaction_score();

                if (batter_swing_score > -1) {
                    construct_outgoing_app_message(MESSAGE_KEY_WEBSOCKET_BATTER_HIT_EVENT, batter_swing_score);
                } else {
                    construct_outgoing_app_message(MESSAGE_KEY_WEBSOCKET_BATTER_MISS_EVENT, 1);
                }

                has_swung = true;

                app_timer_cancel(swing_event_timer);
                app_timer_cancel(miss_timeout_event_timer);

                vibes_long_pulse();
            }

        }
    }
}

static void batting_calibration_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_batter_up_text = text_layer_create(GRect(PBL_IF_RECT_ELSE(0, 10), PBL_IF_RECT_ELSE(52, 62), PBL_IF_RECT_ELSE(bounds.size.w, bounds.size.w - 20), 60));
    text_layer_set_text(s_batter_up_text, "Get into batting position to start the game!");
    text_layer_set_background_color(s_batter_up_text, GColorClear);
    text_layer_set_font(s_batter_up_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_overflow_mode(s_batter_up_text, GTextOverflowModeWordWrap);
    text_layer_set_text_alignment(s_batter_up_text, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_batter_up_text));
}

static void batting_calibration_window_unload(Window *window) {
    text_layer_destroy(s_batter_up_text);
}

void set_progress_hud_window_text() {
    static char hit_count_buffer[15];
    static char miss_count_buffer[15];
    static char combo_count_buffer[15];

    snprintf(hit_count_buffer, sizeof(hit_count_buffer), "Hits: %d", hit_count);
    snprintf(miss_count_buffer, sizeof(miss_count_buffer), "Misses: %d", miss_count);
    snprintf(combo_count_buffer, sizeof(combo_count_buffer), "Streak: %d", combo_count);

    text_layer_set_text(s_hits_status_text, hit_count_buffer);
    text_layer_set_text(s_misses_status_text, miss_count_buffer);
    text_layer_set_text(s_combo_status_text, combo_count_buffer);
}

static void progress_hud_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_hits_status_text = text_layer_create(GRect(0, PBL_IF_RECT_ELSE(32, 42), bounds.size.w, 40));
    text_layer_set_background_color(s_hits_status_text, GColorClear);
    text_layer_set_font(s_hits_status_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_hits_status_text, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_hits_status_text));

    s_misses_status_text = text_layer_create(GRect(0, PBL_IF_RECT_ELSE(62, 72), bounds.size.w, 40));
    text_layer_set_background_color(s_misses_status_text, GColorClear);
    text_layer_set_font(s_misses_status_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_misses_status_text, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_misses_status_text));

    s_combo_status_text = text_layer_create(GRect(0, PBL_IF_RECT_ELSE(92, 102), bounds.size.w, 40));
    text_layer_set_background_color(s_combo_status_text, GColorClear);
    text_layer_set_font(s_combo_status_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_combo_status_text, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_combo_status_text));

    set_progress_hud_window_text();
}

static void progress_hud_window_unload(Window *window) {
  text_layer_destroy(s_hits_status_text);
  text_layer_destroy(s_misses_status_text);
  text_layer_destroy(s_combo_status_text);
}

void batting_calibration_push() {
    if (!s_batting_calibration_window) {
        s_batting_calibration_window = window_create();
        window_set_window_handlers(s_batting_calibration_window, (WindowHandlers) {
            .load = batting_calibration_window_load,
            .unload = batting_calibration_window_unload,
        });

        accel_data_service_subscribe(ACCEL_SAMPLES, accel_batting_calibration_handler);
    }

    is_calibrating = true;

    window_stack_push(s_batting_calibration_window, true);
}

void progress_hud_push() {
    if (!s_progress_hud_window) {
        s_progress_hud_window = window_create();
        window_set_window_handlers(s_progress_hud_window, (WindowHandlers) {
            .load = progress_hud_window_load,
            .unload = progress_hud_window_unload,
        });
    }

    miss_timeout_event_timer = app_timer_register(10000, miss_timeout_event, NULL);

    batter_swing_timer_event_timestamp = 0;
    batter_swing_event_timestamp = 0;

    is_calibrating = false;

    window_stack_push(s_progress_hud_window, true);
}

void game_start_transition() {
    static const uint32_t const segments[] = { 300, 300, 250, 150, 250, 150, 250, 100, 250, 100, 600, 150, 400 };
    VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
    };

    // swing_event_timer = app_timer_register(3000, batting_calibration_push, NULL);
    // vibes_enqueue_custom_pattern(pat);

    // L - 76
    // R - 82
    construct_outgoing_app_message(MESSAGE_KEY_WEBSOCKET_START_GAME_EVENT, 76);
}