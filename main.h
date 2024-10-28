#pragma once

#include <stdint.h>

int write(const char *argv[]);
int read(const char *argv[]);

typedef struct __attribute__((__packed__)) {
    unsigned char marker1;
    unsigned char marker2;
    uint32_t file_length;
    int16_t unused1;
    int16_t unused2;
    uint32_t offset;
} bmp_header;

typedef struct __attribute__((__packed__)) {
    uint32_t header_length;
    uint32_t width;
    uint32_t height;
    uint16_t color_planes;
    uint16_t bits_per_pixel;
    int32_t compression_method;
    uint32_t image_size;
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    uint32_t colors;
    uint32_t colors_used;
} dib_header;

typedef struct __attribute__((__packed__)) {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixel; // only for 24 bpp, but i don't really care
