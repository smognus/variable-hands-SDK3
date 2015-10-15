#include <pebble.h>
#include "hand_paths.h"
  
static Layer *root_window_layer;  
static Layer *second_hand_layer;
static Layer *minute_hand_layer;
static Layer *hour_hand_layer;
static BitmapLayer *background_layer;
static Window *root_window;
static GBitmap *clockface_bitmap;
static GPath *second_hand_path;
static GPath *minute_hand_path;
static GPath *hour_hand_path;
static GPath *hand_highlight_path;

int32_t trig_ninety = TRIG_MAX_ANGLE / 4;
int32_t trig_one_eighty = TRIG_MAX_ANGLE / 2;

static void deinit() {
  layer_destroy(second_hand_layer);
  layer_destroy(minute_hand_layer);
  layer_destroy(hour_hand_layer);
  bitmap_layer_destroy(background_layer);
  window_destroy(root_window);
  gpath_destroy(second_hand_path);
  gpath_destroy(minute_hand_path);
  gpath_destroy(hour_hand_path);
  gpath_destroy(hand_highlight_path);
}
static void time_change_handler(struct tm *current_time, TimeUnits units_changed) {
  layer_mark_dirty(root_window_layer);
  gpath_destroy(second_hand_path);
  gpath_destroy(minute_hand_path);
  gpath_destroy(hour_hand_path);
}

static void second_hand_layer_draw(Layer *layer, GContext *ctx) {
  // Get the current time.
  time_t temp = time(NULL); 
  struct tm *current_time = localtime(&temp);
  
  // `second_angle' is the simple angle used to rotate the path
  // `trig_second_angle' is the value used to calculate the length that the path needs to be.
  int second_angle = current_time->tm_sec * 6;
  int trig_second_angle = (TRIG_MAX_ANGLE / 60) * current_time->tm_sec;

  // `center' will be the origin for the path.
  GRect rect = layer_get_frame(layer);
  GPoint center = grect_center_point(&rect);
  
  // The length of the hand for the left or right edge of the screen for seconds 7-23 and 37-53.
  if (((current_time->tm_sec >= 7 && current_time->tm_sec <= 23) ||
       (current_time->tm_sec >= 37 && current_time->tm_sec <= 53)) &&
      current_time->tm_sec != 0) {
    float second_length = (72 * TRIG_MAX_ANGLE) / (cos_lookup(trig_second_angle - trig_ninety));
    second_hand_path_points.points[4].y = (second_length > 0) ? -(second_length) : second_length;
    second_hand_path_points.points[3].y = (second_length > 0) ? -(second_length-5) : second_length+5;
    second_hand_path_points.points[5].y = (second_length > 0) ? -(second_length-5) : second_length+5;
    hand_highlight_path_points.points[1].y = (second_length > 0) ? -(second_length-10) : second_length+10;
  }
  // The length of the hand for the top or bottom edge of the screen for seconds 54-59, 0-6, and 24-36.
  else {
    float second_length = (84 * TRIG_MAX_ANGLE) / (cos_lookup(trig_second_angle - trig_one_eighty));
    second_hand_path_points.points[4].y = (second_length > 0) ? -(second_length) : second_length;
    second_hand_path_points.points[3].y = (second_length > 0) ? -(second_length-5) : second_length+5;
    second_hand_path_points.points[5].y = (second_length > 0) ? -(second_length-5) : second_length+5;
    hand_highlight_path_points.points[1].y = (second_length > 0) ? -(second_length-10) : second_length+10;
  }
  
  // Create and draw the path with the correct length as modified by the calculations above.
  second_hand_path = gpath_create(&second_hand_path_points);
  gpath_move_to(second_hand_path, center);
  gpath_rotate_to(second_hand_path, TRIG_MAX_ANGLE / 360 * second_angle);
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_context_set_stroke_color(ctx, GColorDarkCandyAppleRed);
  graphics_context_set_stroke_width(ctx, 1);
  gpath_draw_filled(ctx, second_hand_path);
  gpath_draw_outline(ctx, second_hand_path);
  hand_highlight_path = gpath_create(&hand_highlight_path_points);
  gpath_move_to(hand_highlight_path, center);
  gpath_rotate_to(hand_highlight_path, TRIG_MAX_ANGLE / 360 * second_angle);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  gpath_draw_outline(ctx, hand_highlight_path);  
}
static void minute_hand_layer_draw(Layer *layer, GContext *ctx) {
  // Get the current time.
  time_t temp = time(NULL); 
  struct tm *current_time = localtime(&temp);
  
  // `minute_angle' is the simple angle used to rotate the path
  // `trig_minute_angle' is the value used to calculate the length that the path needs to be.
  int minute_angle = current_time->tm_min * 6;
  int trig_minute_angle = (TRIG_MAX_ANGLE / 60) * current_time->tm_min;

  // `center' will be the origin for the path.
  GRect rect = layer_get_frame(layer);
  GPoint center = grect_center_point(&rect);
  
  // The length of the hand for the left or right edge of the screen for minutes 7-23 and 37-53.
  if (((current_time->tm_min >= 7 && current_time->tm_min <= 23) ||
       (current_time->tm_min >= 37 && current_time->tm_min <= 53)) &&
      current_time->tm_min != 0) {
    float minute_length = (72 * TRIG_MAX_ANGLE) / (cos_lookup(trig_minute_angle - trig_ninety));
    minute_hand_path_points.points[4].y = (minute_length > 0) ? -(minute_length) : minute_length;
    minute_hand_path_points.points[3].y = (minute_length > 0) ? -(minute_length-5) : minute_length+5;
    minute_hand_path_points.points[5].y = (minute_length > 0) ? -(minute_length-5) : minute_length+5;
    hand_highlight_path_points.points[1].y = (minute_length > 0) ? -(minute_length-10) : minute_length+10;
  }
  // The length of the hand for the top or bottom edge of the screen for minutes 54-59, 0-6, and 24-36.
  else {
    float minute_length = (84 * TRIG_MAX_ANGLE) / (cos_lookup(trig_minute_angle - trig_one_eighty));
    minute_hand_path_points.points[4].y = (minute_length > 0) ? -(minute_length) : minute_length;
    minute_hand_path_points.points[3].y = (minute_length > 0) ? -(minute_length-5) : minute_length+5;
    minute_hand_path_points.points[5].y = (minute_length > 0) ? -(minute_length-5) : minute_length+5;
    hand_highlight_path_points.points[1].y = (minute_length > 0) ? -(minute_length-10) : minute_length+10;
  }
  
  // Create and draw the path with the correct length as modified by the calculations above.
  minute_hand_path = gpath_create(&minute_hand_path_points);
  gpath_move_to(minute_hand_path, center);
  gpath_rotate_to(minute_hand_path, TRIG_MAX_ANGLE / 360 * minute_angle);
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 1);
  gpath_draw_filled(ctx, minute_hand_path);
  gpath_draw_outline(ctx, minute_hand_path);
  hand_highlight_path = gpath_create(&hand_highlight_path_points);
  gpath_move_to(hand_highlight_path, center);
  gpath_rotate_to(hand_highlight_path, TRIG_MAX_ANGLE / 360 * minute_angle);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  gpath_draw_outline(ctx, hand_highlight_path);  
}
static void hour_hand_layer_draw(Layer *layer, GContext *ctx) {
  // Get the current time.
  time_t temp = time(NULL); 
  struct tm *current_time = localtime(&temp);
  
  if (current_time->tm_hour > 12) {
    current_time->tm_hour-=12;
  }
  
  current_time->tm_hour = 10;
  current_time->tm_min = 30;
  
  // `hour_angle' is the simple angle used to rotate the path
  // `trig_hour_angle' is the value used to calculate the length that the path needs to be.
  int hour_angle = (current_time->tm_hour * 30) + (current_time->tm_min / 2);
  int trig_hour_angle = (TRIG_MAX_ANGLE / 360) * hour_angle;

  // `center' will be the origin for the path.
  GRect rect = layer_get_frame(layer);
  GPoint center = grect_center_point(&rect);
  
  // the number of pixels the hour hand will remain from the window edge
  int hour_length_offset = 35;

  if (((hour_angle >= 49 && hour_angle <= 139) ||
       (hour_angle >= 229 && hour_angle <= 311)) &&
      hour_angle != 0) {
    float hour_length = (72 * TRIG_MAX_ANGLE) / (cos_lookup(trig_hour_angle - trig_ninety));
    hour_hand_path_points.points[4].y = (hour_length > 0) ? -(hour_length-hour_length_offset) : hour_length+hour_length_offset;
    hour_hand_path_points.points[3].y = (hour_length > 0) ? -(hour_length-(hour_length_offset+5)) : hour_length+(hour_length_offset+5);
    hour_hand_path_points.points[5].y = (hour_length > 0) ? -(hour_length-(hour_length_offset+5)) : hour_length+(hour_length_offset+5);
    hand_highlight_path_points.points[1].y = (hour_length > 0) ? -(hour_length-(hour_length_offset+10)) : hour_length+(hour_length_offset+10);
  }
  else {
    float hour_length = (84 * TRIG_MAX_ANGLE) / (cos_lookup(trig_hour_angle - trig_one_eighty));
    hour_hand_path_points.points[4].y = (hour_length > 0) ? -(hour_length-hour_length_offset) : hour_length+hour_length_offset;
    hour_hand_path_points.points[3].y = (hour_length > 0) ? -(hour_length-(hour_length_offset + 5)) : hour_length+(hour_length_offset + 5);
    hour_hand_path_points.points[5].y = (hour_length > 0) ? -(hour_length-(hour_length_offset + 5)) : hour_length+(hour_length_offset + 5);
    hand_highlight_path_points.points[1].y = (hour_length > 0) ? -(hour_length-(hour_length_offset+10)) : hour_length+(hour_length_offset+10);
  }

  // Create and draw the path with the correct length as modified by the calculations above.
  hour_hand_path = gpath_create(&hour_hand_path_points);
  gpath_move_to(hour_hand_path, center);
  gpath_rotate_to(hour_hand_path, TRIG_MAX_ANGLE / 360 * hour_angle);
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 1);
  gpath_draw_filled(ctx, hour_hand_path);
  gpath_draw_outline(ctx, hour_hand_path);
  hand_highlight_path = gpath_create(&hand_highlight_path_points);
  gpath_move_to(hand_highlight_path, center);
  gpath_rotate_to(hand_highlight_path, TRIG_MAX_ANGLE / 360 * hour_angle);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  gpath_draw_outline(ctx, hand_highlight_path);  
}
static void background_layer_draw (Layer* layer, GContext* ctx) {
  graphics_draw_bitmap_in_rect(ctx, clockface_bitmap, layer_get_frame(layer));
}

static void init() {
  root_window = window_create();
  root_window_layer = window_get_root_layer(root_window);
  GRect bounds = layer_get_bounds(root_window_layer);
  
  second_hand_layer = layer_create(bounds);
  layer_set_update_proc(second_hand_layer, second_hand_layer_draw);
  
  minute_hand_layer = layer_create(bounds);
  layer_set_update_proc(minute_hand_layer, minute_hand_layer_draw);
  
  hour_hand_layer = layer_create(bounds);
  layer_set_update_proc(hour_hand_layer, hour_hand_layer_draw);
  
  clockface_bitmap = gbitmap_create_with_resource(RESOURCE_ID_clockface_bitmap);
  
  background_layer = bitmap_layer_create(bounds);
  layer_set_update_proc(bitmap_layer_get_layer(background_layer), background_layer_draw);
    
  // The order here is important for it to look right.
  layer_add_child(root_window_layer, bitmap_layer_get_layer(background_layer));
  layer_add_child(root_window_layer, hour_hand_layer);
  layer_add_child(root_window_layer, minute_hand_layer);
  layer_add_child(root_window_layer, second_hand_layer);

  window_stack_push(root_window, true);
  
  tick_timer_service_subscribe(SECOND_UNIT, time_change_handler);
}

int main(void) {  
  init();
  app_event_loop();
  deinit();
}
