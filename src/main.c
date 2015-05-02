#include <pebble.h>
#include <string.h>
  
#include "utils.h"
#include "config.h"

#define MAX_HOURS_BUFFER 16
#define MAX_MINUTES_BUFFER 16
  
static Window *s_main_window;

static Layer *s_hours_layer;
static Layer *s_minutes_layer;
static Layer *s_motivational_text_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_nero_bitmap;

static InverterLayer *s_battery_layer;

static void update_hours(Layer *this_layer, GContext *ctx) {
  static char hours_buff[MAX_HOURS_BUFFER] = {0};
  int hours_buffer_len = MAX_HOURS_BUFFER;
  
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  int hours_as_12 = tick_time->tm_hour ? tick_time->tm_hour : 12;
  hours_as_12 = (hours_as_12) > 12 ? hours_as_12 - 12 : hours_as_12;
  
  int_to_roman(hours_as_12, hours_buff, &hours_buffer_len);
  
  draw_text_with_outline(ctx, hours_buff, s_roman_font_30, GRect(2, 2, PEBBLE_WIDTH, HOURS_LAYER_HEIGHT - 5), GTextOverflowModeFill, GTextAlignmentCenter, 1);
}

static void update_minutes(Layer *this_layer, GContext *ctx) {
  static char minutes_buff[MAX_MINUTES_BUFFER] = {0};
  int minutes_buffer_len = MAX_MINUTES_BUFFER;
  
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  if (tick_time->tm_min) {
      int_to_roman(tick_time->tm_min, minutes_buff, &minutes_buffer_len);
  } else {
    memset(minutes_buff, '\0', MAX_MINUTES_BUFFER);
  }
  
  draw_text_with_outline(ctx, minutes_buff, s_roman_font_30, GRect(2, 2, PEBBLE_WIDTH, MINUTE_LAYER_HEIGHT - 5), GTextOverflowModeFill, GTextAlignmentCenter, 1);
}

static void update_motivational_text(Layer *this_layer, GContext *ctx) {
  static char motivational_text[] = "Roma delenda est";
  GRect battery_layer_rect = layer_get_frame(inverter_layer_get_layer(s_battery_layer));

  /*APP_LOG(APP_LOG_LEVEL_INFO, "updating motivatinal text");
    APP_LOG(APP_LOG_LEVEL_INFO, "battery layer: %hu", battery_layer_rect.size.h);
    APP_LOG(APP_LOG_LEVEL_INFO, "motivatinal: %d", (int)(MOTIVATIONAL_LAYER_Y + (MOTIVATIONAL_LAYER_HEIGHT / 2)));*/

  //if the inversion layer is in the middle of the text +-, inverse the coloring
  if (battery_layer_rect.size.h >= (MOTIVATIONAL_LAYER_Y + (MOTIVATIONAL_LAYER_HEIGHT / 2))) {
    draw_text_with_outline(ctx, motivational_text, s_roman_font_14, GRect(2, 2, PEBBLE_WIDTH, MOTIVATIONAL_LAYER_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, 1);
  } else {
    draw_text_with_outline(ctx, motivational_text, s_roman_font_14, GRect(2, 2, PEBBLE_WIDTH, MOTIVATIONAL_LAYER_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, 0);
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
}

static void update_time() {
  // Display this time on the TextLayer
  layer_mark_dirty(s_hours_layer);
  layer_mark_dirty(s_minutes_layer);
}


static void main_window_load(Window *window) {
  s_nero_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NERO_WHITE_ON_BLACK);
  s_background_layer = bitmap_layer_create(GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT));
  
  bitmap_layer_set_bitmap(s_background_layer, s_nero_bitmap);
  
  s_battery_layer = inverter_layer_create(GRect(0, 0, 0, 0));
  
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(s_battery_layer));
  
  battery_state_service_subscribe(battery_state_subscriber);
                                          
  // Create time TextLayer
  s_hours_layer = layer_create(GRect(HOURS_LAYER_X, HOURS_LAYER_Y, PEBBLE_WIDTH, HOURS_LAYER_HEIGHT));
  s_minutes_layer = layer_create(GRect(MINUTES_LAYER_X, MINUTES_LAYER_Y, PEBBLE_WIDTH, MINUTE_LAYER_HEIGHT));
  s_motivational_text_layer = layer_create(GRect(MOTIVATIONAL_LAYER_X, MOTIVATIONAL_LAYER_Y, PEBBLE_WIDTH, MOTIVATIONAL_LAYER_HEIGHT));
  
  layer_set_update_proc(s_hours_layer, update_hours);
  layer_set_update_proc(s_minutes_layer, update_minutes);
  layer_set_update_proc(s_motivational_text_layer, update_motivational_text);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), s_hours_layer);
  layer_add_child(window_get_root_layer(window), s_minutes_layer);
  layer_add_child(window_get_root_layer(window), s_motivational_text_layer);
  layer_mark_dirty(s_motivational_text_layer);
  //update to battery at launch
  battery_state_subscriber(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
  layer_destroy(s_hours_layer);
  layer_destroy(s_minutes_layer);
  layer_destroy(s_motivational_text_layer);
  
  gbitmap_destroy(s_nero_bitmap);
  bitmap_layer_destroy(s_background_layer);
  inverter_layer_destroy(s_battery_layer);
  battery_state_service_unsubscribe();
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