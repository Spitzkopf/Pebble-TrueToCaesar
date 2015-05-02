#include "config.h"

void load_fonts() { 
  s_roman_font_30 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AUGUSTUS_ROMAN_30));
  s_roman_font_20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AUGUSTUS_ROMAN_14));
}

void unload_fonts() {
  fonts_unload_custom_font(s_roman_font_30);
  fonts_unload_custom_font(s_roman_font_20);
}
