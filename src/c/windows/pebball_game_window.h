#pragma once

#include <pebble.h>

static Window *s_batting_calibration_window;
static Window *s_progress_hud_window;

AppTimer *swing_event_timer;
AppTimer *miss_timeout_event_timer;

// false = left, true = right
bool wrist_position;

void batting_calibration_push();
void progress_hud_push();
void game_start_transition();

void set_progress_hud_window_text();

void batter_swing_event(void * data);