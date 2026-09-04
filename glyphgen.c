#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "lib/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

int parse_config(stbtt_pack_range **ranges, float font_size) {
	int n_ranges = 0;;
	int i = 0;
	
    FILE *file = fopen("charranges.cfg", "r");
    if (file == NULL) {
        perror("error opening config");
        return -1;
    }
	
	char buffer[64];
	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		n_ranges++;
	}
	fseek(file, 0, SEEK_SET);
	*ranges = (stbtt_pack_range*)malloc(sizeof(stbtt_pack_range) * n_ranges);

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
		int start_char;
		int char_len;
        sscanf(buffer, "%d %d", &start_char, &char_len);
		printf("%d\n", n_ranges);
		
		stbtt_packedchar *charmap_data = (stbtt_packedchar*)malloc(sizeof(stbtt_packedchar) * char_len);
		
		(*ranges)[i].font_size = font_size;
		(*ranges)[i].first_unicode_codepoint_in_range = start_char;
		(*ranges)[i].array_of_unicode_codepoints = NULL;
		(*ranges)[i].num_chars = char_len;
		(*ranges)[i].chardata_for_range = charmap_data;
		
		i++;
    }

    fclose(file);
	return n_ranges;
}

void gen_code(stbtt_pack_range *ranges, int n_ranges) {
	char *if_template = "\n"
    "if (ch >= %d && ch <= %d) {\n"
    "    int local_index = ch - %d;\n"
    "    return (%d + local_index);\n"
    "}\n";
	
	int atlas_offset = 0;
	int start_char = 0;
	int char_len = 0;
	
	FILE *file_ptr = fopen("code.c", "w");

    if (file_ptr == NULL) {
        fprintf(stderr, "error opening code.c\n");
    }

	for (int i = 0; i < n_ranges; i++) {
		start_char = ranges[i].first_unicode_codepoint_in_range;
		char_len = ranges[i].num_chars;
		
		fprintf(file_ptr, if_template, start_char, start_char + char_len, start_char, atlas_offset);
		
		atlas_offset += char_len;
	}
	
	fclose(file_ptr);
}

int main(int argc, char **argv) {
	if (argc != 7) { printf("Usage: glyphgen.exe atlas_width atlas_height size font.ttf (atlas.txt/atlas.png) (data.txt/data.fdt)\n"); return 0; }
	int type = 0;
	int format = 0;
	
	if (strstr(argv[5], ".png")) {
		type = 1;
	}
	else if (strstr(argv[5], ".txt")) {
		type = 0;
	}
	else {
		printf("Wrong atlas type, pick either .txt for binary or .png for image");
		return 1;
	}
	
	if (strstr(argv[6], ".fdt")) {
		format = 1;
	}
	else if (strstr(argv[6], ".txt")) {
		format = 0;
	}
	else {
		printf("Wrong data type, pick either .txt for array or .fdt for text");
		return 1;
	}
	
	int atlas_width = atoi(argv[1]);
	int atlas_height = atoi(argv[2]);
	
	float font_size = atof(argv[3]);
	
    int oversample_x = 2;
    int oversample_y = 2;
	
	stbtt_pack_range *ranges = NULL;
	int n_ranges = parse_config(&ranges, font_size);
	
	stbtt_pack_context context;
	unsigned char *atlas_data = (unsigned char*)malloc(atlas_width * atlas_height);
	
	FILE *font_file = fopen(argv[4], "rb");
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

    if (!stbtt_PackFontRanges(&context, font_bin, 0, ranges, n_ranges)) {
        printf("failed to pack font\n");
		return 1;
    }

	stbtt_PackEnd(&context);
	
	if (type == 1) {
		if (!stbi_write_png(argv[5], atlas_width, atlas_height, 1, atlas_data, atlas_width)) {
			printf("failed to write to png\n");
			return 1;
		}
		printf("png saved!\n");
	}
	else {
		FILE *atlas_file;
		atlas_file = fopen(argv[5], "w");
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
	data_file = fopen(argv[6], "w");
	if (data_file == NULL) {
        printf("error opening data file\n");
        return 1;
    }

	if (!format) {
		fprintf(data_file, "float font_data[%d] = {\n", (96 + 64 + 1 + 1) * 10);
	}
	
	for (int i = 0; i < n_ranges; i++)
		for (int chr = 0; chr < ranges[i].num_chars; chr++) {
			stbtt_aligned_quad quad;
			float offx = 0.0f, offy = 0.0f;
			float xadv = ranges[i].chardata_for_range[chr].xadvance;
			stbtt_GetPackedQuad(ranges[i].chardata_for_range, atlas_width, atlas_height, chr, &offx, &offy, &quad, 0);
			if (format) {
				fprintf(data_file, "%d %f %f %f %f %f %f %f %f %f %f\n", (ranges[i].first_unicode_codepoint_in_range + chr), xadv, offy, quad.x0, quad.x1, quad.y0, quad.y1, quad.s0, quad.s1, quad.t0, quad.t1);
			}
			else {
				fprintf(data_file, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f,\n", xadv, offy, quad.x0, quad.x1, quad.y0, quad.y1, quad.s0, quad.s1, quad.t0, quad.t1);
			}
		}

	if (!format) {
		fprintf(data_file, "};\n");
	}
	printf("data saved!\n");
	fclose(data_file);
	
	gen_code(ranges, n_ranges);
	printf("code generated!\n");
	
	return 0;
}