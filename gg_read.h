#ifndef GG_READ_H
#define GG_READ_H

#include <stddef.h>

typedef struct {
	float uvx0; 
    float uvx1; 
    float uvy0;
    float uvy1; 

    float x0;
	float x1;
    float y0;
    float y1;
} glyph_data;

glyph_data gg_get_char_data(float *font_array, wchar_t ch, float* cursor_x, float* cursor_y);
int gg_get_char_index(wchar_t ch);

#endif