#ifndef UTILS_H_
#define UTILS_H_

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define UNUSED(x) (void)(x)
#define UNIMPLEMENTED(msg) \
    do { \
        fprintf(stderr, "%s:%d: UNIMPLEMENTED: %s\n", __FILE__, __LINE__, msg); \
    } while(0)
#define ARRAY_LEN(xs) (sizeof(xs) / sizeof(xs[0]))

typedef struct {
    size_t width;
    size_t height;
    uint32_t *pixels;
} Image32;

#define IMAGE32_GET(img, x, y) &(img).pixels[(y)*(img).width + (x)]

int save_output_image(Image32 *image, const char* file_name)
{
    return stbi_write_png(
                 file_name,
                 image->width,
                 image->height,
                 sizeof(uint32_t),
                 image->pixels,
                 image->width * sizeof(uint32_t));
}

#endif // UTILS_H_
