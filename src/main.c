#include <pebble.h>
#include "main.h"
#include "hand_paths.h"

#define tickSetting 0
#define daySetting 1
#define batterySetting 2
#define secondStartSetting 3
#define secondEndSetting 4
#define digitalSetting 5
#define windowColorSetting 6
#define windowBorderColorSetting 7
#define windowTextColorSetting 8

#define day_frame GRect(90,73,22,25)
#define digital_time_frame GRect(48,106,48,24)
#define battery_rect GRect(48,38,48,3)

static Layer *root_window_layer;  
static Layer *second_hand_layer;
static Layer *minute_hand_layer;
static Layer *hour_hand_layer;
static Layer *day_layer;
static Layer *battery_status_layer;
static Layer *digital_layer;
static BitmapLayer *background_layer;
static TextLayer *digital_numbers_layer;
static TextLayer *day_number_layer;
static Window *root_window;
static GBitmap *clockface_bitmap;
static GPath *second_hand_path;
static GPath *minute_hand_path;
static GPath *hour_hand_path;
static GPath *hand_highlight_path;

const int32_t trig_ninety = TRIG_MAX_ANGLE / 4;
const int32_t trig_one_eighty = TRIG_MAX_ANGLE / 2;
static int previous_hour;

static GColor infoWindowColor;
static GColor infoWindowBorderColor;
static GColor infoWindowTextColor;

static void deinit() {
  layer_destroy(second_hand_layer);
  layer_destroy(minute_hand_layer);
  layer_destroy(hour_hand_layer);
  bitmap_layer_destroy(background_layer);
  window_destroy(root_window);
}
static struct tm* get_current_time() {
    time_t temp = time(NULL); 
    struct tm *current_time = localtime(&temp);
  return current_time;
}
static void set_tick_update_interval(TimeUnits tickunit) {
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(tickunit, time_change_handler);
}
static bool determine_second_hand_draw() {
    time_t temp = time(NULL); 
    struct tm *current_time = localtime(&temp);
    int current_hour = current_time->tm_hour;
    int start_hour = persist_read_int(secondStartSetting);
    int end_hour = persist_read_int(secondEndSetting);
    if ((current_hour >= start_hour && current_hour <= end_hour) && persist_read_bool(tickSetting))  {
      APP_LOG(APP_LOG_LEVEL_INFO, "Setting updates to every second.");
      layer_set_hidden(second_hand_layer, false);
      set_tick_update_interval(SECOND_UNIT);
      return true;
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "Setting updates to every minute.");
      layer_set_hidden(second_hand_layer, true);
      set_tick_update_interval(MINUTE_UNIT);
      return false;
  }
}
static void time_change_handler(struct tm *current_time, TimeUnits units_changed) {
//  determine_second_hand_draw();
  APP_LOG(APP_LOG_LEVEL_INFO, "Tick handling.");
  int current_hour = current_time->tm_hour;
  if (current_hour != previous_hour) {
    determine_second_hand_draw();  
    previous_hour = current_hour;
    APP_LOG(APP_LOG_LEVEL_INFO, "New hour has elapsed.");
  }
  layer_mark_dirty(root_window_layer);
}
static void digital_numbers_layer_draw(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, infoWindowColor);
    graphics_context_set_stroke_color(ctx, infoWindowBorderColor);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_fill_rect(ctx, digital_time_frame, 5, GCornersAll);
    graphics_draw_round_rect(ctx, digital_time_frame, 5);
  
    static char string_time[16];
    clock_copy_time_string(string_time, sizeof(string_time));
    APP_LOG(APP_LOG_LEVEL_INFO, "%s.", string_time);
    text_layer_set_font(digital_numbers_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_background_color(digital_numbers_layer, GColorClear);
    text_layer_set_text_color(digital_numbers_layer, infoWindowTextColor);
    text_layer_set_text_alignment(digital_numbers_layer, GTextAlignmentCenter);
    text_layer_set_text(digital_numbers_layer, string_time);
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
  graphics_context_set_stroke_color(ctx, GColorRichBrilliantLavender);
  graphics_context_set_stroke_width(ctx, 1);
  gpath_draw_outline(ctx, hand_highlight_path);  
  gpath_destroy(second_hand_path);
  gpath_destroy(hand_highlight_path);
}
static void minute_hand_layer_draw(Layer *layer, GContext *ctx) {
  // Get the current time.
  time_t temp = time(NULL); 
  struct tm *current_time = localtime(&temp);
  
  //current_time->tm_min = 15;
  
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
  gpath_destroy(minute_hand_path);
  gpath_destroy(hand_highlight_path);
}
static void hour_hand_layer_draw(Layer *layer, GContext *ctx) {
  // Get the current time.
  time_t temp = time(NULL); 
  struct tm *current_time = localtime(&temp);
  
  if (current_time->tm_hour > 12) {
    current_time->tm_hour-=12;
  }
  
   //current_time->tm_hour = 4;
 // current_time->tm_min = 0;
  
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
  gpath_destroy(hour_hand_path);
  gpath_destroy(hand_highlight_path);
}
static void day_layer_draw (Layer* layer, GContext* ctx) {
  if (persist_read_bool(daySetting)) {
    time_t temp = time(NULL); 
    struct tm *current_time = localtime(&temp);
    
    static char day[] = "--";
    
    strftime(day, sizeof(day), "%e", current_time);
    
    graphics_context_set_fill_color(ctx, infoWindowColor);
    graphics_context_set_stroke_color(ctx, infoWindowBorderColor);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_fill_rect(ctx, day_frame, 5, GCornersAll);
    graphics_draw_rect(ctx, day_frame);
    
    text_layer_set_font(day_number_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_background_color(day_number_layer, GColorClear);
    text_layer_set_text_color(day_number_layer, infoWindowTextColor);
    text_layer_set_text_alignment(day_number_layer, GTextAlignmentCenter);
    text_layer_set_text(day_number_layer, day);
  }
}
static void battery_status_draw (Layer* layer, GContext* ctx) {
  int battery_bar_origin_x = 48;
  int battery_bar_destination_x = 96;
  int battery_bar_origin_y = 42;  
  BatteryChargeState charge_state = battery_state_service_peek(); 
  
  int current_charge = charge_state.charge_percent;
  int current_depletion = 100 - current_charge;
  // Shorten the green portion of the bar 5 pixels for every 10% reduction in battery state;
  int charge_destination_x = battery_bar_destination_x - (5 * (current_depletion / 10));
  
  graphics_context_set_stroke_width(ctx, 6);
  graphics_context_set_stroke_color(ctx, GColorLightGray); 
  graphics_draw_line(ctx, GPoint(battery_bar_origin_x, battery_bar_origin_y), GPoint(battery_bar_destination_x, battery_bar_origin_y));
  
  graphics_context_set_stroke_width(ctx, 3);
  graphics_context_set_stroke_color(ctx, GColorDarkGray); 
  graphics_draw_line(ctx, GPoint(battery_bar_origin_x, battery_bar_origin_y), GPoint(battery_bar_destination_x, battery_bar_origin_y));
  graphics_context_set_stroke_color(ctx, GColorGreen); 
  graphics_draw_line(ctx, GPoint(battery_bar_origin_x, battery_bar_origin_y), GPoint(charge_destination_x, battery_bar_origin_y));
}
static void background_layer_draw (Layer* layer, GContext* ctx) {
  graphics_draw_bitmap_in_rect(ctx, clockface_bitmap, layer_get_frame(layer));
}
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
   Tuple *tick_setting_tuple = dict_find(iterator, tickSetting);
    if(tick_setting_tuple && tick_setting_tuple->value->int32 > 0) {
      persist_write_bool(tickSetting, true);
    } else {
      persist_write_bool(tickSetting, false);
  }
  Tuple *day_setting_tuple = dict_find(iterator, daySetting);
  if(day_setting_tuple && day_setting_tuple->value->int32 > 0) {
    persist_write_bool(daySetting, true);
    layer_set_hidden(day_layer, false);
    layer_set_hidden(text_layer_get_layer(day_number_layer), false);
  } else {
    persist_write_bool(daySetting, false);
    layer_set_hidden(day_layer, true);
    layer_set_hidden(text_layer_get_layer(day_number_layer), true);
  }
  Tuple *battery_setting_tuple = dict_find(iterator, batterySetting);
  if(battery_setting_tuple && battery_setting_tuple->value->int32 > 0) {
    persist_write_bool(batterySetting, true);
    layer_set_hidden(battery_status_layer, false);
  } else {
    persist_write_bool(batterySetting, false);
    layer_set_hidden(battery_status_layer, true);
  }
  Tuple *digital_setting_tuple = dict_find(iterator, digitalSetting);
  if(digital_setting_tuple && digital_setting_tuple->value->int32 > 0) {
    persist_write_bool(digitalSetting, true);
    layer_set_hidden(digital_layer, false);
  } else {
    persist_write_bool(digitalSetting, false);
    layer_set_hidden(digital_layer, true);
  }
  Tuple *second_start_tuple = dict_find(iterator, secondStartSetting);
  Tuple *second_end_tuple = dict_find(iterator, secondEndSetting);
  if(second_start_tuple && second_end_tuple) {
    int second_start = second_start_tuple->value->int32;
    persist_write_int(secondStartSetting, second_start);
    int second_end = second_end_tuple->value->int32;
    persist_write_int(secondEndSetting, second_end);
  }
  Tuple *window_color_tuple = dict_find(iterator, windowColorSetting);
  if (window_color_tuple) {
    infoWindowColor = GColorFromHEX(window_color_tuple->value->int32);
    persist_write_int(windowColorSetting, window_color_tuple->value->int32);
  }
  Tuple *window_border_color_tuple = dict_find(iterator, windowBorderColorSetting);
 if (window_border_color_tuple) {
    infoWindowBorderColor = GColorFromHEX(window_border_color_tuple->value->int32);
    persist_write_int(windowColorSetting, window_border_color_tuple->value->int32);
  }
  Tuple *window_text_color_tuple = dict_find(iterator, windowTextColorSetting);
  if (window_text_color_tuple) {
    infoWindowTextColor = GColorFromHEX(window_text_color_tuple->value->int32);
    persist_write_int(windowColorSetting, window_text_color_tuple->value->int32);
  }
  determine_second_hand_draw();
  layer_mark_dirty(root_window_layer);
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
static void init() {
  infoWindowColor = (persist_exists(windowColorSetting)) ? GColorFromHEX(persist_read_int(windowColorSetting)) : GColorBlack;
  infoWindowBorderColor = (persist_exists(windowBorderColorSetting)) ? GColorFromHEX(persist_read_int(windowBorderColorSetting)) : GColorDarkGray;
  infoWindowTextColor = (persist_exists(windowTextColorSetting)) ? GColorFromHEX(persist_read_int(windowTextColorSetting)) : GColorWhite;
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  root_window = window_create();
  root_window_layer = window_get_root_layer(root_window);
  GRect bounds = layer_get_bounds(root_window_layer);
  
  second_hand_layer = layer_create(bounds);
  layer_set_update_proc(second_hand_layer, second_hand_layer_draw);
  
  minute_hand_layer = layer_create(bounds);
  layer_set_update_proc(minute_hand_layer, minute_hand_layer_draw);
  
  hour_hand_layer = layer_create(bounds);
  layer_set_update_proc(hour_hand_layer, hour_hand_layer_draw);
  
  day_layer = layer_create(bounds);
  layer_set_update_proc(day_layer, day_layer_draw);
  
  day_number_layer = text_layer_create(day_frame);
  
  digital_layer = layer_create(bounds);
  layer_set_update_proc(digital_layer, digital_numbers_layer_draw);
  
  digital_numbers_layer = text_layer_create(digital_time_frame);
  
  battery_status_layer = layer_create(bounds);
  layer_set_update_proc(battery_status_layer, battery_status_draw);
  
  clockface_bitmap = gbitmap_create_with_resource(RESOURCE_ID_clockface_bitmap);
  
  background_layer = bitmap_layer_create(bounds);
  layer_set_update_proc(bitmap_layer_get_layer(background_layer), background_layer_draw);
    
  layer_add_child(root_window_layer, bitmap_layer_get_layer(background_layer));
  
  layer_add_child(root_window_layer, battery_status_layer);
  
  layer_add_child(bitmap_layer_get_layer(background_layer), day_layer);
  
  layer_add_child(bitmap_layer_get_layer(background_layer), digital_layer);
  
  layer_add_child(hour_hand_layer, text_layer_get_layer(digital_numbers_layer)); 
  
  if (persist_read_bool(batterySetting)) {
    layer_set_hidden(battery_status_layer, false);
  } else {
    layer_set_hidden(battery_status_layer, true);
  }
  if (persist_read_bool(daySetting)) {
    layer_set_hidden(day_layer, false);
    layer_set_hidden(text_layer_get_layer(day_number_layer), false);
  } else {
    layer_set_hidden(day_layer, true);
    layer_set_hidden(text_layer_get_layer(day_number_layer), true);
  }
  if (persist_read_bool(digitalSetting)) {
    layer_set_hidden(digital_layer, false);
  } else {
    layer_set_hidden(digital_layer, true);
  }
  
  layer_add_child(root_window_layer, hour_hand_layer);
  layer_add_child(root_window_layer, minute_hand_layer);
  layer_add_child(day_layer, text_layer_get_layer(day_number_layer));
  layer_add_child(digital_layer, text_layer_get_layer(digital_numbers_layer));
  layer_add_child(root_window_layer, second_hand_layer);
  
  window_stack_push(root_window, true);
  set_tick_update_interval((determine_second_hand_draw()) ? SECOND_UNIT : MINUTE_UNIT);
  struct tm *current_time = get_current_time();
  previous_hour = current_time->tm_hour;
}

int main(void) {  
  init();
  app_event_loop();
  deinit();
}
