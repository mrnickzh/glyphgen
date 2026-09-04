#include "gg_read.h"

int gg_get_char_index(wchar_t ch) {
    if (ch >= 32 && ch <= 127) {
        int local_index = ch - 32;
        return local_index;
    }
    
    if (ch >= 0x0410 && ch <= 0x044F) {
        int local_index = ch - 0x0410;
        return (96 + local_index); 
    }
    
    if (ch == 0x0401) {
        return 160;
    }
    
    if (ch == 0x0451) {
        return 161;
    }

    return (' ' - 32); 
}

glyph_data gg_get_char_data(float *font_array, wchar_t ch, float* cursor_x, float* cursor_y) {
	glyph_data glyph;

    int base_idx = gg_get_char_index(ch) * 10;

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