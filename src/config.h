#ifndef __CONFIG_H__
#define __CONFIG_H__
  
#include <pebble.h>

#define PEBBLE_WIDTH 144
#define PEBBLE_HEIGHT 168

#define HOURS_LAYER_HEIGHT 35
#define MINUTE_LAYER_HEIGHT 40
#define MOTIVATIONAL_LAYER_HEIGHT 40
  
#define HOURS_LAYER_X 0
#define HOURS_LAYER_Y 98
  
#define MINUTES_LAYER_X 0
#define MINUTES_LAYER_Y (HOURS_LAYER_Y + 30)

#define MOTIVATIONAL_LAYER_X 0
#define MOTIVATIONAL_LAYER_Y 5

GFont s_roman_font_30;
GFont s_roman_font_14;

void load_fonts();
void unload_fonts();
  
#endif