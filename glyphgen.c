#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "lib/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

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
	
	int atlas_width = atoi(argv[2]);
	int atlas_height = atoi(argv[3]);
	
	float font_size = atof(argv[4]);
	
    int oversample_x = 2;
    int oversample_y = 2;
	
    stbtt_packedchar *ascii_data = (stbtt_packedchar*)malloc(sizeof(stbtt_packedchar) * 96);
    stbtt_packedchar *russian_data = (stbtt_packedchar*)malloc(sizeof(stbtt_packedchar) * 64);
    stbtt_packedchar *yo_data = (stbtt_packedchar*)malloc(sizeof(stbtt_packedchar) * 1);
    stbtt_packedchar *yo_small_data = (stbtt_packedchar*)malloc(sizeof(stbtt_packedchar) * 1);

	stbtt_pack_range ranges[4];
    // ASCII
    ranges[0].font_size = font_size;
    ranges[0].first_unicode_codepoint_in_range = 32;
	ranges[0].array_of_unicode_codepoints = NULL;
    ranges[0].num_chars = 96;
    ranges[0].chardata_for_range = ascii_data;

    // rus
    ranges[1].font_size = font_size;
    ranges[1].first_unicode_codepoint_in_range = 0x0410; 
	ranges[1].array_of_unicode_codepoints = NULL;
    ranges[1].num_chars = 64; 
    ranges[1].chardata_for_range = russian_data;

    // YO
    ranges[2].font_size = font_size;
    ranges[2].first_unicode_codepoint_in_range = 0x0401;
	ranges[2].array_of_unicode_codepoints = NULL;
    ranges[2].num_chars = 1;
    ranges[2].chardata_for_range = yo_data;

    // yo
    ranges[3].font_size = font_size;
    ranges[3].first_unicode_codepoint_in_range = 0x0451;
	ranges[3].array_of_unicode_codepoints = NULL;
    ranges[3].num_chars = 1;
    ranges[3].chardata_for_range = yo_small_data;
	
	stbtt_pack_context context;
	unsigned char *atlas_data = (unsigned char*)malloc(atlas_width * atlas_height);
	
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
	
	if (!stbtt_PackBegin(&context, atlas_data, atlas_width, atlas_height, 0, 1, NULL)) {
		printf("failed to init stb pack\n");
		return 1;
	}
	stbtt_PackSetOversampling(&context, oversample_x, oversample_y);

    if (!stbtt_PackFontRanges(&context, font_bin, 0, ranges, 4)) {
        printf("failed to pack font\n");
		return 1;
    }

	stbtt_PackEnd(&context);
	
	if (type == 1) {
		if (!stbi_write_png(argv[6], atlas_width, atlas_height, 1, atlas_data, atlas_width)) {
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
		fprintf(atlas_file, "unsigned char font_atlas[%d] = {\n", atlas_width * atlas_height);
		for (int i = 0; i < atlas_width * atlas_height; i++) {
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
	fprintf(data_file, "float font_data[%d] = {\n", (96 + 64 + 1 + 1) * 10);
	/* ASCII char data */
	for (int chr = 0; chr < 96; chr++) {
		stbtt_aligned_quad quad;
		float offx = 0.0f, offy = 0.0f;
		float xadv = ascii_data[chr].xadvance;
		stbtt_GetPackedQuad(ascii_data, atlas_width, atlas_height, chr, &offx, &offy, &quad, 0);
		fprintf(data_file, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f,\n", xadv, offy, quad.x0, quad.x1, quad.y0, quad.y1, quad.s0, quad.s1, quad.t0, quad.t1);
	}
	/* rus char data */
	for (int chr = 0; chr < 64; chr++) {
		stbtt_aligned_quad quad;
		float offx = 0.0f, offy = 0.0f;
		float xadv = russian_data[chr].xadvance;
		stbtt_GetPackedQuad(russian_data, atlas_width, atlas_height, chr, &offx, &offy, &quad, 0);
		fprintf(data_file, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f,\n", xadv, offy, quad.x0, quad.x1, quad.y0, quad.y1, quad.s0, quad.s1, quad.t0, quad.t1);
	}
	/* YO char data */
	{
		stbtt_aligned_quad quad;
		float offx = 0.0f, offy = 0.0f;
		float xadv = yo_data[0].xadvance;
		stbtt_GetPackedQuad(yo_data, atlas_width, atlas_height, 0, &offx, &offy, &quad, 0);
		fprintf(data_file, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f,\n", xadv, offy, quad.x0, quad.x1, quad.y0, quad.y1, quad.s0, quad.s1, quad.t0, quad.t1);
	}
	/* yo char data */
	{
		stbtt_aligned_quad quad;
		float offx = 0.0f, offy = 0.0f;
		float xadv = yo_small_data[0].xadvance;
		stbtt_GetPackedQuad(yo_small_data, atlas_width, atlas_height, 0, &offx, &offy, &quad, 0);
		fprintf(data_file, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f,\n", xadv, offy, quad.x0, quad.x1, quad.y0, quad.y1, quad.s0, quad.s1, quad.t0, quad.t1);
	}

	fprintf(data_file, "};\n");
	printf("data saved!\n");
	fclose(data_file);
	
	return 0;
}