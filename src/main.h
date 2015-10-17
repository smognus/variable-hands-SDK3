#include <pebble.h>
static void time_change_handler(struct tm *current_time, TimeUnits units_changed);
static bool determine_second_hand_draw();