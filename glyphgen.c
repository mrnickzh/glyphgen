#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "lib/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

#define ATLAS_WIDTH 512
#define ATLAS_HEIGHT 512

struct {
    int size;
    unsigned int atlas_width;
    unsigned int atlas_height;
    unsigned int oversample_x;
    unsigned int oversample_y;
    unsigned int first_char;
    unsigned int char_count;
    stbtt_packedchar *char_info;
} font_data;

int main(int argc, char **argv) {
	if (argc < 4) { printf("Usage: glyphgen.exe font.ttf atlas.png glyps.txt\n"); return 0; }
	
	font_data.size = 20;
    font_data.atlas_width = ATLAS_WIDTH;
    font_data.atlas_height = ATLAS_HEIGHT;
    font_data.oversample_x = 2;
    font_data.oversample_y = 2;
    font_data.first_char = ' ';
    font_data.char_count = '~' - ' ';
	
	font_data.char_info = (stbtt_packedchar*)malloc(sizeof(stbtt_packedchar) * font_data.char_count);
	stbtt_pack_context context;
	unsigned char atlas_data[ATLAS_WIDTH * ATLAS_HEIGHT];
	
	FILE *font_file = fopen(argv[1], "rb");
	if (font_file == NULL) { printf("cant open file\n"); return 1; }
	fseek(font_file, 0, SEEK_END);
	long file_size = ftell(font_file);
	rewind(font_file);
    unsigned char *font_bin = malloc(file_size);
	size_t bytes_read = fread(font_bin, 1, file_size, font_file);
    if (bytes_read < (size_t)file_size) {
        if (ferror(font_file)) {
            perror("error reading file\n");
            free(font_bin);
            fclose(font_file);
            return 1;
        }
    }
	fclose(font_file);
	
	if (!stbtt_PackBegin(&context, &atlas_data[0], font_data.atlas_width, font_data.atlas_height, 0, 1, NULL)) {
		printf("failed to init stb pack\n");
		return 1;
	}
	
	stbtt_PackSetOversampling(&context, font_data.oversample_x, font_data.oversample_y);
    if (!stbtt_PackFontRange(&context, font_bin, 0, font_data.size, font_data.first_char, font_data.char_count, font_data.char_info)) {
        printf("failed to pack font\n");
		return 1;
	}
	
	stbtt_PackEnd(&context);
	
	if (!stbi_write_png(argv[2], ATLAS_WIDTH, ATLAS_HEIGHT, 1, &atlas_data[0], ATLAS_WIDTH)) {
		printf("failed to write to png\n");
		return 1;
	}
	printf("png saved!\n");
	
	FILE *out_file;
	out_file = fopen(argv[3], "w");
	if (out_file == NULL) {
        printf("error opening out file\n");
        return 1;
    }
	
	fprintf(out_file, "float font_array[%d] = {\n", font_data.char_count * 10);
	for (int chr = 0; chr < font_data.char_count; chr++) {
		stbtt_aligned_quad quad;
		float offx = 0.0f, offy = 0.0f;
		float xadv = font_data.char_info[chr].xadvance;
		stbtt_GetPackedQuad(font_data.char_info, font_data.atlas_width, font_data.atlas_height, chr, &offx, &offy, &quad, 0);
		fprintf(out_file, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f,\n", xadv, offy, quad.x0, quad.x1, quad.y0, quad.y1, quad.s0, quad.s1, quad.t0, quad.t1);
	}
	fprintf(out_file, "}\n");
	printf("array save!\n");
	
	return 0;
}