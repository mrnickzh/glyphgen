#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "lib/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

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
	if (argc != 8) { printf("Usage: glyphgen.exe (-b/-i) atlas_width atlas_height size font.ttf (atlas.txt/atlas.png) data.txt\n"); return 0; }
	int type = 0;
	
	if (strcmp(argv[1], "-b") == 0) {
		type = 0;
	}
	else if (strcmp(argv[1], "-i") == 0) {
		type = 1;
	}
	else {
		printf("Wrong output type, pick either -b for binary or -i for image");
		return 1;
	}
	
	int ATLAS_WIDTH = atoi(argv[2]);
	int ATLAS_HEIGHT = atoi(argv[3]);
	
	font_data.size = atoi(argv[4]);
    font_data.atlas_width = ATLAS_WIDTH;
    font_data.atlas_height = ATLAS_HEIGHT;
    font_data.oversample_x = 2;
    font_data.oversample_y = 2;
    font_data.first_char = ' ';
    font_data.char_count = '~' - ' ';
	
	font_data.char_info = (stbtt_packedchar*)malloc(sizeof(stbtt_packedchar) * font_data.char_count);
	stbtt_pack_context context;
	unsigned char *atlas_data = (unsigned char*)malloc(ATLAS_WIDTH * ATLAS_HEIGHT);
	
	FILE *font_file = fopen(argv[5], "rb");
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
	
	if (!stbtt_PackBegin(&context, atlas_data, font_data.atlas_width, font_data.atlas_height, 0, 1, NULL)) {
		printf("failed to init stb pack\n");
		return 1;
	}
	stbtt_PackSetOversampling(&context, font_data.oversample_x, font_data.oversample_y);
    if (!stbtt_PackFontRange(&context, font_bin, 0, font_data.size, font_data.first_char, font_data.char_count, font_data.char_info)) {
        printf("failed to pack font\n");
		return 1;
	}
	stbtt_PackEnd(&context);
	
	if (type == 1) {
		if (!stbi_write_png(argv[6], ATLAS_WIDTH, ATLAS_HEIGHT, 1, atlas_data, ATLAS_WIDTH)) {
			printf("failed to write to png\n");
			return 1;
		}
		printf("png saved!\n");
	}
	else {
		FILE *atlas_file;
		atlas_file = fopen(argv[6], "w");
		if (atlas_file == NULL) {
			printf("error opening atlas file\n");
			return 1;
		}
		fprintf(atlas_file, "unsigned char font_atlas[%d] = {\n", ATLAS_WIDTH * ATLAS_HEIGHT);
		for (int i = 0; i < ATLAS_WIDTH * ATLAS_HEIGHT; i++) {
			fprintf(atlas_file, "0x%02x,", atlas_data[i]);
			if (i % 10 == 0) { fprintf(atlas_file, "\n"); }
		}
		fprintf(atlas_file, "};\n");
		printf("atlas saved!\n");
		fclose(atlas_file);
	}
	
	FILE *data_file;
	data_file = fopen(argv[7], "w");
	if (data_file == NULL) {
        printf("error opening data file\n");
        return 1;
    }
	fprintf(data_file, "float font_data[%d] = {\n", font_data.char_count * 10);
	for (int chr = 0; chr < font_data.char_count; chr++) {
		stbtt_aligned_quad quad;
		float offx = 0.0f, offy = 0.0f;
		float xadv = font_data.char_info[chr].xadvance;
		stbtt_GetPackedQuad(font_data.char_info, font_data.atlas_width, font_data.atlas_height, chr, &offx, &offy, &quad, 0);
		fprintf(data_file, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f,\n", xadv, offy, quad.x0, quad.x1, quad.y0, quad.y1, quad.s0, quad.s1, quad.t0, quad.t1);
	}
	fprintf(data_file, "};\n");
	printf("data saved!\n");
	fclose(data_file);
	
	return 0;
}