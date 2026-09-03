#include "gg_read.h"

glyph_data gg_get_char_data(float *font_array, char c, float* cursor_x, float* cursor_y) {
    int first_char = ' '; 
    int char_index = c - first_char;
	
	glyph_data glyph;

    if (char_index < 0 || char_index >= 95) return glyph; 

    int base_idx = char_index * 10;

    float xadvance = font_array[base_idx + 0]; 
    float offy     = font_array[base_idx + 1];
    float x0       = font_array[base_idx + 2]; 
    float x1       = font_array[base_idx + 3]; 
    float y0       = font_array[base_idx + 4]; 
    float y1       = font_array[base_idx + 5]; 
    glyph.uvx0       = font_array[base_idx + 6]; 
    glyph.uvx1       = font_array[base_idx + 7]; 
    glyph.uvy0       = font_array[base_idx + 8]; 
    glyph.uvy1       = font_array[base_idx + 9]; 

    glyph.x0 = *cursor_x + x0;
    glyph.y0 = *cursor_y + y0;
    glyph.x1 = *cursor_x + x1;
    glyph.y1 = *cursor_y + y1;

    *cursor_x += xadvance; 
	return glyph;
}