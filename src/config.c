#include "config.h"

extern GFont s_roman_font_30;
extern GFont s_roman_font_14;
  
void load_fonts() { 
  s_roman_font_30 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AUGUSTUS_ROMAN_30));
  s_roman_font_14 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AUGUSTUS_ROMAN_14));
}

void unload_fonts() {
  fonts_unload_custom_font(s_roman_font_30);
  fonts_unload_custom_font(s_roman_font_14);
}
