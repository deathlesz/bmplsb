#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

int main(const int argc, const char *argv[]) {
    if (argc != 4) {
        printf("[!] usage: %s read <IMAGE> <LENGTH>\n"
               "           %s write <IMAGE> <FILE>\n",
               argv[0], argv[0]);
        return 1;
    }

    if (!strcmp(argv[1], "write")) {
        return write(argv);
    } else if (!strcmp(argv[1], "read")) {
        return read(argv);
    }
}

int write(const char *argv[]) {
    FILE *file = fopen(argv[2], "r");
    if (file == NULL) {
        printf("[!] failed to open '%s'\n", argv[2]);
        return 1;
    }

    bmp_header bmp_header;
    dib_header dib_header;
    if (fread(&bmp_header, sizeof(bmp_header), 1, file) != 1) {
        puts("[!] failed to read bmp header");
        return 1;
    }
    if (fread(&dib_header, sizeof(dib_header), 1, file) != 1) {
        puts("[!] failed to read dib header");
        return 1;
    }

    if (dib_header.bits_per_pixel != 24) {
        puts("[!] this program doesn't work with non-24bpp files :(");
        return 1;
    }

    if (fseek(file, bmp_header.offset, SEEK_SET)) {
        printf("[!] failed to seek to position %d\n", bmp_header.offset);
        return 1;
    }

    unsigned long input_length = strlen(argv[2]);

    char *filename = malloc(input_length + 9);
    strcpy(filename, "modified-");
    strcat(filename, argv[2]);

    FILE *out = fopen(filename, "w");
    if (out == NULL) {
        printf("[!] failed to open '%s'\n", filename);
        return 1;
    }

    if (fwrite(&bmp_header, sizeof(bmp_header), 1, out) != 1) {
        puts("[!] failed to write BMP header");
        return 1;
    }
    if (fwrite(&dib_header, sizeof(dib_header), 1, out) != 1) {
        puts("[!] failed to write DIB header");
        return 1;
    }

    long pos;
    if ((pos = ftell(out)) == -1) {
        puts("[!] failed to calculate padding for output file");
        return 1;
    }
    unsigned long const data_padding = bmp_header.offset - pos;

    char const zero = 0;
    if (fwrite(&zero, sizeof(char), bmp_header.offset - pos, out) !=
        data_padding) {
        puts("[!] failed to add padding to output file");
        return 1;
    }

    FILE *data_file = fopen(argv[3], "r");
    if (data_file == NULL) {
        printf("[!] failed to open '%s'", argv[3]);
    }

    long data_length;
    if (fseek(data_file, 0, SEEK_END) ||
        (data_length = ftell(data_file)) == -1 ||
        fseek(data_file, 0, SEEK_SET)) {
        puts("[!] failed to get size of data");
        return 1;
    }

    if (data_length > dib_header.width * dib_header.height) {
        puts("[!] data is too large for this image");
        return 1;
    }

    char *data = malloc(data_length);
    if (fread(data, 1, data_length, data_file) != (unsigned long)data_length) {
        puts("[!] failed to read data file");
        return 1;
    }

    char padding = dib_header.width % 4;

    unsigned int counter = 0;
    pixel pixel;
    for (unsigned int i = 0; i < dib_header.height; i++) {
        for (unsigned int j = 0; j < dib_header.width; j++) {
            if (fread(&pixel, sizeof(pixel), 1, file) != 1) {
                printf("[!] failed to read pixel #%d\n", i + j);
                return 1;
            }

            if (counter < data_length) {
                char byte = data[i + j];

                // we clear last 2/3 bits and then replace them with bits from
                // the message
                pixel.r &= 0b11111100;
                pixel.r |= ((byte & 0b11000000) >> 6);

                pixel.g &= 0b11111000;
                pixel.g |= ((byte & 0b00111000) >> 3);

                pixel.b &= 0b11111000;
                pixel.b |= ((byte & 0b00000111));

                counter++;
            }

            if (fwrite(&pixel, sizeof(pixel), 1, out) != 1) {
                printf("[!] failed to write pixel #%d", i + j);
                return 1;
            }
        }
        if (fseek(file, padding, SEEK_CUR)) {
            printf("[!] failed to account for padding in original file");
            return 1;
        }
        if (fwrite(&zero, sizeof(char), padding, out) !=
            (unsigned long)padding) {
            printf("[!] failed to add padding");
            return 1;
        }
    }

    if (fflush(out) == EOF) {
        printf("[!] failed to flush output file; it may be corrupted");
    }

    free(data);

    if (fclose(file) == EOF) {
        printf("[!] failed to close '%s'\n", argv[2]);
    }
    if (fclose(out) == EOF) {
        printf("[!] failed to close 'out.bmp'");
    }

    return 0;
}

int read(const char *argv[]) {
    FILE *file = fopen(argv[2], "r");
    if (file == NULL) {
        printf("[!] failed to open '%s'\n", argv[2]);
        return 1;
    }

    bmp_header bmp_header;
    dib_header dib_header;
    if (fread(&bmp_header, sizeof(bmp_header), 1, file) != 1) {
        puts("[!] failed to read bmp header");
        return 1;
    }
    if (fread(&dib_header, sizeof(dib_header), 1, file) != 1) {
        puts("[!] failed to read dib header");
        return 1;
    }
    if (dib_header.bits_per_pixel != 24) {
        puts("[!] this program doesn't work with non-24bpp files :(");
        return 1;
    }

    if (fseek(file, bmp_header.offset, SEEK_SET)) {
        printf("[!] failed to seek to position %d\n", bmp_header.offset);
        return 1;
    }

    unsigned int const length = atoi(argv[3]);
    char padding = dib_header.width % 4;

    unsigned int counter = 0;
    pixel pixel;
    for (unsigned int i = 0; i < dib_header.height && counter < length; i++) {
        for (unsigned int j = 0; j < dib_header.width && counter < length;
             j++) {
            if (fread(&pixel, sizeof(pixel), 1, file) != 1) {
                printf("[!] failed to read pixel #%d\n", i + j);
                return 1;
            }

            char result = 0;
            result |= (pixel.r & 0b00000011) << 6;
            result |= (pixel.g & 0b00000111) << 3;
            result |= (pixel.b & 0b00000111);

            printf("%c", result);
            counter++;
        }

        if (fseek(file, padding, SEEK_CUR)) {
            printf("[!] failed to account for padding");
            return 1;
        }
    }

    if (fclose(file) == EOF) {
        printf("[!] failed to close '%s'\n", argv[2]);
    }

    return 0;
}
