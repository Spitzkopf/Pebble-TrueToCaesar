#ifndef __UTILS_H__
#define __UTILS_H__
#include <pebble.h>

//draw an outlined text, default is black text white outline, set invert to TRUE for white text with black outline
void draw_text_with_outline(GContext *ctx, const char * text, GFont const font, GRect coords, const GTextOverflowMode overflow_mode, const GTextAlignment alignment, int invert);

//convert a number between 1 and 3999 to roman numerals
int int_to_roman(int number, char* result, int* result_buffer_len);

#endif