#ifndef __CONFIG_H__
#define __CONFIG_H__
  
#include <pebble.h>

extern GFont s_roman_font_30;
extern GFont s_roman_font_20;

void load_fonts();
void unload_fonts();
  
#endif