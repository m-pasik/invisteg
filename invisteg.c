#include <png.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int write_data(char *input_name, char *data_name, char *output_name)
{
    int exit_code = EXIT_FAILURE;

    FILE *input_ptr = fopen(input_name, "rb");
    if (!input_ptr) {
        fprintf(stderr, "ERROR: Could not input image: %s.\n", output_name);
        goto close_input;
    }

    FILE *data_ptr = fopen(data_name, "rb");
    if (!data_ptr) {
        fprintf(stderr, "ERROR: Could not open data file: %s.\n", output_name);
        goto close_data;
    }

    FILE *output_ptr = fopen(output_name, "wb");
    if (!output_ptr) {
        fprintf(stderr, "ERROR: Could not open output image: %s.\n", output_name);
        goto close_all;
    }

    png_structp png_read = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_structp png_write = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_read || !png_write) {
        fprintf(stderr, "ERROR: Allocation failure.\n");
        if (png_read)
            goto destroy_read;
        goto close_all;
    }

    png_infop read_info = png_create_info_struct(png_read);
    png_infop write_info = png_create_info_struct(png_write);
    if (!read_info || !write_info) {
        fprintf(stderr, "ERROR: Allocation failure.\n");
        goto destroy_all;
    }

    if (setjmp(png_jmpbuf(png_read)) || setjmp(png_jmpbuf(png_write))) {
        fprintf(stderr, "ERROR: init_io failure.\n");
        goto destroy_all;
    }

    png_init_io(png_read, input_ptr);
    png_read_info(png_read, read_info);

    png_uint_32 width = png_get_image_width(png_read, read_info);
    png_uint_32 height = png_get_image_height(png_read, read_info);
    png_byte bit_depth = png_get_bit_depth(png_read, read_info);
    png_byte color_type = png_get_color_type(png_read, read_info);

    if (!PNG_COLOR_TYPE_RGBA && !PNG_COLOR_TYPE_GRAY_ALPHA) {
        fprintf(stderr, "ERROR: Invalid color type, supported types: RGBA, Gray + Alpha.\n");
        goto destroy_all;
    }

    png_init_io(png_write, output_ptr);
    png_set_IHDR(png_write, write_info, width, height, bit_depth, color_type,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_write, write_info);

    png_bytep row = (png_bytep)malloc(height * png_get_rowbytes(png_read, read_info));

    if (!row) {
        fprintf(stderr, "ERROR: Allocation failure.\n");
        goto destroy_all;
    }

    size_t bytes_read = 0;
    size_t y = 0;
    bool reading = true;

    if (color_type == PNG_COLOR_TYPE_RGBA) {
        if (bit_depth == 8)
            goto rgba8;
        if (bit_depth == 16)
            goto rgba16;
    } else {
        if (bit_depth == 8)
            goto graya8;
        if (bit_depth == 16)
            goto graya16;
    }
    goto write_end;

rgba8:
    for (y = 0; y < height; y++) {
        png_read_row(png_read, row, NULL);
        size_t x;
        for (x = 0; x < width * 4; x += 4) {
            if (row[x + 3] == 0) {
                char buff[3] = { 0 };
                if (reading) {
                    bytes_read = fread(buff, 1, 3, data_ptr);
                    if (ferror(data_ptr)) {
                        fprintf(stderr, "ERROR: Error reading data from file: %s\n", data_name);
                        goto write_end;
                    }
                }
                if (!bytes_read)
                    reading = false;
                memcpy((char *)(row + x), buff, 3);
            }
        }
        png_write_row(png_write, row);
    }
    goto clean;

rgba16:
    for (y = 0; y < height; y++) {
        png_read_row(png_read, row, NULL);
        for (size_t x = 0; x < width * 8; x += 8) {
            if (*(uint16_t *)(row + x + 6) == 0) {
                char buff[6] = { 0 };
                if (reading) {
                    bytes_read = fread(buff, 1, 6, data_ptr);
                    if (ferror(data_ptr)) {
                        fprintf(stderr, "ERROR: Error reading data from file: %s\n", data_name);
                        goto write_end;
                    }
                }
                if (!bytes_read)
                    reading = false;
                memcpy((char *)(row + x), buff, 6);
            }
        }
        png_write_row(png_write, row);
    }
    goto clean;

graya8:
    for (y = 0; y < height; y++) {
        png_read_row(png_read, row, NULL);
        size_t x;
        for (x = 0; x < width * 2; x += 2) {
            if (row[x + 1] == 0) {
                char buff[1] = { 0 };
                if (reading) {
                    bytes_read = fread(buff, 1, 1, data_ptr);
                    if (ferror(data_ptr)) {
                        fprintf(stderr, "ERROR: Error reading data from file: %s\n", data_name);
                        goto write_end;
                    }
                }
                if (!bytes_read)
                    reading = false;
                memcpy((char *)(row + x), buff, 1);
            }
        }
        png_write_row(png_write, row);
    }
    goto clean;

graya16:
    for (y = 0; y < height; y++) {
        png_read_row(png_read, row, NULL);
        for (size_t x = 0; x < width * 4; x += 4) {
            if (*(uint16_t *)(row + x + 2) == 0) {
                char buff[2] = { 0 };
                if (reading) {
                    bytes_read = fread(buff, 1, 2, data_ptr);
                    if (ferror(data_ptr)) {
                        fprintf(stderr, "ERROR: Error reading data from file: %s\n", data_name);
                        goto write_end;
                    }
                }
                if (!bytes_read)
                    reading = false;
                memcpy((char *)(row + x), buff, 2);
            }
        }
        png_write_row(png_write, row);
    }
    goto clean;

clean:
    exit_code = EXIT_SUCCESS;
    long read = ftell(data_ptr);
    fseek(data_ptr, 0, SEEK_END);
    long size = ftell(data_ptr);
    printf("%.2f%% of the data has been written.\n", (float)100 * (float)read / (float)size);

write_end:
    png_write_end(png_write, NULL);
    free(row);
destroy_all:
    png_destroy_write_struct(&png_write, &write_info);
destroy_read:
    png_destroy_read_struct(&png_read, &read_info, NULL);
close_all:
    fclose(output_ptr);
close_data:
    fclose(data_ptr);
close_input:
    fclose(input_ptr);

    return exit_code;
}

int read_data(char *input_name, char *output_name)
{
    int exit_code = EXIT_FAILURE;

    FILE *input_ptr = fopen(input_name, "rb");
    if (!input_ptr) {
        fprintf(stderr, "ERROR: Could not input image: %s.\n", output_name);
        goto close_input;
    }

    FILE *output_ptr = fopen(output_name, "wb");
    if (!output_ptr) {
        fprintf(stderr, "ERROR: Could not open output file: %s.\n", output_name);
        goto close_all;
    }

    png_structp png_read = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_read) {
        fprintf(stderr, "ERROR: Allocation failure.\n");
        goto destroy_read;
    }

    png_infop read_info = png_create_info_struct(png_read);
    if (!read_info) {
        fprintf(stderr, "ERROR: Allocation failure.\n");
        goto destroy_read;
    }

    if (setjmp(png_jmpbuf(png_read))) {
        fprintf(stderr, "ERROR: init_io failure.\n");
        goto destroy_read;
    }

    png_init_io(png_read, input_ptr);
    png_read_info(png_read, read_info);

    png_uint_32 width = png_get_image_width(png_read, read_info);
    png_uint_32 height = png_get_image_height(png_read, read_info);
    png_byte bit_depth = png_get_bit_depth(png_read, read_info);
    png_byte color_type = png_get_color_type(png_read, read_info);

    if (!PNG_COLOR_TYPE_RGBA && !PNG_COLOR_TYPE_GRAY_ALPHA) {
        fprintf(stderr, "ERROR: Invalid color type, supported types: RGBA, Gray + Alpha.\n");
        goto destroy_read;
    }

    png_bytep row = (png_bytep)malloc(height * png_get_rowbytes(png_read, read_info));

    if (!row) {
        fprintf(stderr, "ERROR: Allocation failure.\n");
        goto destroy_read;
    }

    if (color_type == PNG_COLOR_TYPE_RGBA) {
        if (bit_depth == 8)
            goto rgba8;
        if (bit_depth == 16)
            goto rgba16;
    } else {
        if (bit_depth == 8)
            goto graya8;
        if (bit_depth == 16)
            goto graya16;
    }
    goto free_row;

rgba8:
    for (size_t y = 0; y < height; y++) {
        png_read_row(png_read, row, NULL);
        size_t x;
        for (x = 0; x < width * 4; x += 4) {
            if (row[x + 3] == 0) {
                if (fwrite(row + x, 1, 3, output_ptr) != 3) {
                    fprintf(stderr, "ERROR: Failed writing to file %s\n", output_name);
                    goto free_row;
                }
            }
        }
    }
    goto clean;

rgba16:
    for (size_t y = 0; y < height; y++) {
        png_read_row(png_read, row, NULL);
        size_t x;
        for (x = 0; x < width * 8; x += 8) {
            if (*(uint16_t *)(row + x + 6) == 0) {
                if (fwrite(row + x, 1, 6, output_ptr) != 6) {
                    fprintf(stderr, "ERROR: Failed writing to file %s\n", output_name);
                    goto free_row;
                }
            }
        }
    }
    goto clean;

graya8:
    for (size_t y = 0; y < height; y++) {
        png_read_row(png_read, row, NULL);
        size_t x;
        for (x = 0; x < width * 2; x += 2) {
            if (row[x + 1] == 0) {
                if (fwrite(row + x, 1, 1, output_ptr) != 1) {
                    fprintf(stderr, "ERROR: Failed writing to file %s\n", output_name);
                    goto free_row;
                }
            }
        }
    }
    goto clean;

graya16:
    for (size_t y = 0; y < height; y++) {
        png_read_row(png_read, row, NULL);
        size_t x;
        for (x = 0; x < width * 4; x += 4) {
            if (*(uint16_t *)(row + x + 2) == 0) {
                if (fwrite(row + x, 1, 2, output_ptr) != 2) {
                    fprintf(stderr, "ERROR: Failed writing to file %s\n", output_name);
                    goto free_row;
                }
            }
        }
    }
    goto clean;

clean:
    exit_code = EXIT_SUCCESS;
free_row:
    free(row);
destroy_read:
    png_destroy_read_struct(&png_read, &read_info, NULL);
close_all:
    fclose(output_ptr);
close_input:
    fclose(input_ptr);

    return exit_code;
}

int main(int argc, char **argv)
{
    if (argc == 4)
        return write_data(argv[1], argv[2], argv[3]);
    else if (argc == 3)
        return read_data(argv[1], argv[2]);

    printf("Usage: %s [input_image] [input_data/output_data] [output_image]\n\n"
           "Instructions:\n"
           "\tIf 3 files are provided the program will write input_data to\n"
           "\ttransparent pixels in input_image and save it to output_image.\n\n"
           "\tIf 2 files are provided the program will read data from transparent\n"
           "\tpixels in the input_image and write it to output_data.\n",
        argv[0]);

    return EXIT_FAILURE;
}
