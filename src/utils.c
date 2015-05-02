#include "utils.h"
  
void draw_text_with_outline(GContext *ctx, const char * text, GFont const font, GRect coords, const GTextOverflowMode overflow_mode, const GTextAlignment alignment, int invert){
    graphics_context_set_text_color(ctx, invert ? GColorBlack : GColorWhite);
  
    GRect first_coords = coords;
    first_coords.origin.x -= 1;
    first_coords.origin.y -= 1;
  
    GRect second_coords = coords;
    second_coords.origin.x += 1;
    second_coords.origin.y -= 1;
  
    GRect third_coords = coords;
    third_coords.origin.x -= 1;
    third_coords.origin.y += 1;
  
    GRect fourth_coords = coords;
    fourth_coords.origin.x += 1;
    fourth_coords.origin.y += 1;
  
    graphics_draw_text(ctx, text, font, first_coords, overflow_mode, alignment, NULL);
    graphics_draw_text(ctx, text, font, second_coords, overflow_mode, alignment, NULL);
    graphics_draw_text(ctx, text, font, third_coords, overflow_mode, alignment, NULL);
    graphics_draw_text(ctx, text, font, fourth_coords, overflow_mode, alignment, NULL);
  
    graphics_context_set_text_color(ctx, invert ? GColorWhite : GColorBlack);
  
    graphics_draw_text(ctx, text, font, coords, overflow_mode, alignment, NULL);
}  
  
int int_to_roman(int number, char* result, int* result_buffer_len) {
  if ((number < 0) || (number > 3999)) {
    return 0;
  }
  
  int ints[] =   {1000, 900,  500, 400,  100,  90,  50,   40,  10,   9,    5,   4,    1};
  char *nums[] = {"M",  "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
  
  int temp_number = number;
  int i = 0;
  int size_for_roman_numerals = 1;
  
  for (i = 0; i < (int)(sizeof(ints) / sizeof(int)); ++i) {
    int count = temp_number / ints[i];
    int j;
    for (j=0; j < count; j++) {
       size_for_roman_numerals += strlen(nums[i]);
    }
    temp_number -= ints[i] * count;
  }
  
  if((!result) || (!result_buffer_len) || (*result_buffer_len < size_for_roman_numerals)) {
    return 0;
  }
  
  temp_number = number;
  memset(result, '\0', *result_buffer_len);
  char *temp_result = result;
  for (i = 0; i < (int)(sizeof(ints) / sizeof(int)); ++i) {
    int count = temp_number / ints[i];
    int j;
    for (j=0; j < count; j++) {
      strcpy(temp_result, nums[i]);
      temp_result += strlen(nums[i]);
    }
    temp_number -= ints[i] * count;
  }
 
  
  return 1;
}
