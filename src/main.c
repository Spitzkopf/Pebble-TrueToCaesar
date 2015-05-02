#include <pebble.h>
#include <string.h>
  
#include "utils.h"
#include "config.h"

#define MAX_HOURS_BUFFER 16
#define MAX_MINUTES_BUFFER 16
#define MAX_BATTERY_BUFFER 16
  
static Window *s_main_window;

static Layer *s_hours_layer;
static Layer *s_minutes_layer;
static Layer *s_motivational_text_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_nero_bitmap;

static InverterLayer *s_battery_layer;

static Layer *s_statusbox_layer;
static Layer *s_statusbox_content_layer;

static void update_hours(Layer *this_layer, GContext *ctx) {
  static char hours_buff[MAX_HOURS_BUFFER] = {0};
  int hours_buffer_len = MAX_HOURS_BUFFER;
  
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  int hours_as_12 = tick_time->tm_hour ? tick_time->tm_hour : 12;
  hours_as_12 = (hours_as_12) > 12 ? hours_as_12 - 12 : hours_as_12;
  
  if (!int_to_roman(hours_as_12, hours_buff, &hours_buffer_len)) {
    TERMINATE_AND_LOG_ERROR("Failed converting hours to roman numerals. Hour: %d", hours_as_12);
  }
  
  draw_text_with_outline(ctx, hours_buff, s_roman_font_30, GRect(2, 2, PEBBLE_WIDTH, HOURS_LAYER_HEIGHT - 5), GTextOverflowModeFill, GTextAlignmentCenter, 1);
}

static void update_minutes(Layer *this_layer, GContext *ctx) {
  static char minutes_buff[MAX_MINUTES_BUFFER] = {0};
  int minutes_buffer_len = MAX_MINUTES_BUFFER;
  
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  if (tick_time->tm_min) {
      if (!int_to_roman(tick_time->tm_min, minutes_buff, &minutes_buffer_len)) {
        TERMINATE_AND_LOG_ERROR("Failed converting minutes to roman numerals. Minutes: %d", tick_time->tm_min);
      }
  } else {
    memset(minutes_buff, '\0', MAX_MINUTES_BUFFER);
  }
  
  draw_text_with_outline(ctx, minutes_buff, s_roman_font_30, GRect(2, 2, PEBBLE_WIDTH, MINUTE_LAYER_HEIGHT - 5), GTextOverflowModeFill, GTextAlignmentCenter, 1);
}

static void update_time() {
  // Display this time on the TextLayer
  layer_mark_dirty(s_hours_layer);
  layer_mark_dirty(s_minutes_layer);
}

static void update_motivational_text(Layer *this_layer, GContext *ctx) {
  GRect battery_layer_rect = layer_get_frame(inverter_layer_get_layer(s_battery_layer));
  
  //if the inversion layer is in the middle of the text +-, inverse the coloring
  if (battery_layer_rect.size.h >= (MOTIVATIONAL_LAYER_Y + (MOTIVATIONAL_LAYER_HEIGHT / 2))) {
    draw_text_with_outline(ctx, MOTIVATIONAL_TEXT, s_roman_font_14, GRect(2, 2, PEBBLE_WIDTH, MOTIVATIONAL_LAYER_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, 1);
  } else {
    draw_text_with_outline(ctx, MOTIVATIONAL_TEXT, s_roman_font_14, GRect(2, 2, PEBBLE_WIDTH, MOTIVATIONAL_LAYER_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, 0);
  }
}

static void battery_state_subscriber(BatteryChargeState charge) {
  int battery_tens = 0;
  double cover_percentage = 0;
  if (charge.is_plugged) {
    layer_set_frame(inverter_layer_get_layer(s_battery_layer), GRect(0, 0, 0, 0));
  } else if(0 == (charge.charge_percent % 10)) {
    battery_tens = charge.charge_percent / 10;
    cover_percentage = ((10.0 - battery_tens) / 10);
    layer_set_frame(inverter_layer_get_layer(s_battery_layer), GRect(0, 0, PEBBLE_WIDTH,PEBBLE_HEIGHT * cover_percentage));
  }
  //the motivational text may need to recolor due to the inversion
  layer_mark_dirty(s_motivational_text_layer);
  
  struct StatusBarInfo* sb_info = layer_get_data(s_statusbox_content_layer);
  
  memcpy(&sb_info->charge, &charge, sizeof(BatteryChargeState));
  layer_mark_dirty(s_statusbox_content_layer);
}

static void bluetooth_state_subscriber(bool connected) {
  struct StatusBarInfo* sb_info = layer_get_data(s_statusbox_content_layer);
  
  if (connected != sb_info->bt_connected) {
    vibes_short_pulse();
  }
  
  memcpy(&sb_info->bt_connected, &connected, sizeof(bool));
  layer_mark_dirty(s_statusbox_content_layer);
}

static void draw_status_box_layer(Layer *this_layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(this_layer);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx,bounds, 0, GCornersAll);
}

static Layer* initialize_status_box() {
  Layer* statusbox_layer = layer_create(GRect(0, PEBBLE_HEIGHT - 15, PEBBLE_WIDTH, 15));
  layer_set_update_proc(statusbox_layer, draw_status_box_layer);
  return statusbox_layer;
}

static void draw_statusbox_content(Layer* this_layer, GContext *ctx) {
  static char battery[MAX_BATTERY_BUFFER] = {0};
  int battery_buffer_len = MAX_BATTERY_BUFFER;
  
  struct StatusBarInfo* sb_info = NULL;
  GRect bounds = layer_get_bounds(this_layer);
  
  sb_info = (struct StatusBarInfo*)layer_get_data(this_layer);
  
  if (!int_to_roman(sb_info->charge.charge_percent, battery, &battery_buffer_len)) {
      TERMINATE_AND_LOG_ERROR("Failed converting battery to roman numerals. Battery charge: %d", (int)sb_info->charge.charge_percent);
  }
  
  strcat(battery, "%");
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_draw_text(ctx, battery, s_roman_font_14, bounds, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, sb_info->bt_connected ? BT_ON_TEXT : BT_OFF_TEXT, s_roman_font_14, bounds, GTextOverflowModeFill, GTextAlignmentRight, NULL);
}

static Layer* initialize_status_box_content(Layer* status_box_layer) {
  GRect bounds = layer_get_frame(status_box_layer);
  Layer* content_layer = layer_create_with_data(bounds, sizeof(struct StatusBarInfo));
  
  //set initial bt info
  struct StatusBarInfo* sb_info = (struct StatusBarInfo*)layer_get_data(content_layer);
  sb_info->bt_connected = bluetooth_connection_service_peek();
  
  layer_set_update_proc(content_layer, draw_statusbox_content);
  return content_layer;
}

static void main_window_load(Window *window) {
  //initialize background
  s_background_layer = bitmap_layer_create(GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT));
  s_nero_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NERO_WHITE_ON_BLACK);
  bitmap_layer_set_bitmap(s_background_layer, s_nero_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  //initialize background inverter layer
  s_battery_layer = inverter_layer_create(GRect(0, 0, 0, 0));
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(s_battery_layer));
  
  //initialize statusbox layer
  s_statusbox_layer = initialize_status_box();
  layer_add_child(window_get_root_layer(window), s_statusbox_layer);
  s_statusbox_content_layer = initialize_status_box_content(s_statusbox_layer);
  layer_add_child(window_get_root_layer(window), s_statusbox_content_layer);
                                          
  //initialize hour layer
  s_hours_layer = layer_create(GRect(HOURS_LAYER_X, HOURS_LAYER_Y, PEBBLE_WIDTH, HOURS_LAYER_HEIGHT));
  layer_set_update_proc(s_hours_layer, update_hours);
  layer_add_child(window_get_root_layer(window), s_hours_layer);
  
  //initialize minutes layer
  s_minutes_layer = layer_create(GRect(MINUTES_LAYER_X, MINUTES_LAYER_Y, PEBBLE_WIDTH, MINUTE_LAYER_HEIGHT));
  layer_set_update_proc(s_minutes_layer, update_minutes);
  layer_add_child(window_get_root_layer(window), s_minutes_layer);
  
  //initialize motivational text
  s_motivational_text_layer = layer_create(GRect(MOTIVATIONAL_LAYER_X, MOTIVATIONAL_LAYER_Y, PEBBLE_WIDTH, MOTIVATIONAL_LAYER_HEIGHT)); 
  layer_set_update_proc(s_motivational_text_layer, update_motivational_text);
  layer_add_child(window_get_root_layer(window), s_motivational_text_layer);
  
  //render the static layers
  layer_mark_dirty(s_motivational_text_layer);
  layer_mark_dirty(s_statusbox_layer);
  
  //add battery and bluetooth subscribers
  battery_state_service_subscribe(battery_state_subscriber);
  bluetooth_connection_service_subscribe(bluetooth_state_subscriber);
  
  battery_state_subscriber(battery_state_service_peek());
  bluetooth_state_subscriber(bluetooth_connection_service_peek());
}

static void main_window_unload(Window *window) {
  layer_destroy(s_hours_layer);
  layer_destroy(s_minutes_layer);
  layer_destroy(s_motivational_text_layer);
  layer_destroy(s_statusbox_layer);
  layer_destroy(s_statusbox_content_layer);
  
  gbitmap_destroy(s_nero_bitmap);
  bitmap_layer_destroy(s_background_layer);
  inverter_layer_destroy(s_battery_layer);
  
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
  load_fonts();
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
  unload_fonts();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}