#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define UNUSED(x) (void)(x)
#define UNIMPLEMENTED(msg) \
    do { \
        fprintf(stderr, "%s:%d: UNIMPLEMENTED: %s\n", __FILE__, __LINE__, msg); \
    } while(0)
#define ARRAY_LEN(xs) = (sizeof(xs) / sizeof(xs[0]))

#define CIFAR_SIZE 32
#define CIFAR_COMP 3
static_assert(CIFAR_SIZE * CIFAR_SIZE * CIFAR_COMP == 3072);

typedef struct {
    size_t width;
    size_t height;
    uint32_t *pixels;
} Image32;

#define IMAGE32_GET(img, x, y) (img).pixels[(y)*(img).width + (x)]

/**
 * markov: {
 *   state: {pixel: count}
 * }
 */
#define STATE_SIZE 1
typedef struct {
    uint32_t vert[STATE_SIZE];
    uint32_t horz[STATE_SIZE];
} State;

typedef struct {
    uint32_t key;   // pixel
    uint32_t value; // count TODO: make proba instead of count
} Next;

typedef struct {
    State key;
    Next  *value;
} Markov;

void feed_pixel_to_stats(Next **stats, uint32_t pixel)
{
    ptrdiff_t index = hmgeti(*stats, pixel);

    if (index < 0)
        hmput(*stats, pixel, 1);
    else
        (*stats)[index].value += 1;
}

void get_image_stats(Image32 *image, Next **stats)
{
    for (size_t y = 0; y < image->height; ++y)
        for (size_t x = 0; x < image->width; ++x)
            feed_pixel_to_stats(stats, IMAGE32_GET(*image, x, y));
}

State make_state(Image32 *image, size_t x, size_t y)
{
    State state = {0};

    for (size_t i = 1; i <= STATE_SIZE; ++i) {
        state.vert[i - 1] = IMAGE32_GET(*image, x, y - i);
        state.horz[i - 1] = IMAGE32_GET(*image, x - i, y);
    }

    return state;
}

void feed_image_to_markov(Markov **chain, Image32 *image)
{
    for (size_t y = 0; y < image->height; ++y) {
        for (size_t x = 0; x < image->width; ++x) {
            // state leading to the pixel[y, x]
            State state = make_state(image, x, y);
            uint32_t pixel = IMAGE32_GET(*image, x, y);
            ptrdiff_t index = hmgeti(*chain, state);

            if (index < 0) {
                Next *next = NULL;
                feed_pixel_to_stats(&next, pixel);
                hmput(*chain, state, next);
            }
            else {
                feed_pixel_to_stats(&((*chain)[index].value), pixel);
            }
        }
    }
}

void read_next_cifar_image(FILE *stream, uint8_t *label, Image32 *output)
{
    // schema: <1 x label><1024 x r><1024 x g><1024 x b>
    fread(label, 1, 1, stream);

    static uint8_t pixels[CIFAR_SIZE * CIFAR_SIZE * CIFAR_COMP];
    fread(pixels, sizeof(pixels), 1, stream);

    for (size_t y = 0; y < CIFAR_SIZE; ++y) {
        for (size_t x = 0; x < CIFAR_SIZE; ++x) {
            size_t index = y * CIFAR_SIZE + x;
            uint8_t r = pixels[CIFAR_SIZE * CIFAR_SIZE * 0 + index];
            uint8_t g = pixels[CIFAR_SIZE * CIFAR_SIZE * 1 + index];
            uint8_t b = pixels[CIFAR_SIZE * CIFAR_SIZE * 2 + index];

            output->pixels[index] = r | (g << 8) | (b << 16) | 0xFF000000;
        }
    }
}

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

const char* training_data[] = {
    "./data/cifar-10-batches-bin/data_batch_2.bin",
};

int main()
{
    int ret = 0;
    const char *input_file_name = training_data[0];

    FILE *stream = fopen(input_file_name, "rb");
    if (stream == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        ret = -1;
        goto err_file;
    }

    uint8_t label;
    Image32 image = {0};
    image.width = CIFAR_SIZE;
    image.height = CIFAR_SIZE;
    image.pixels = malloc(sizeof(uint32_t) * image.width * image.height);
    if (image.pixels == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        ret = -1;
        goto err_mem;
    }

    Markov *chain = NULL;

    for (size_t i = 1; i <= 10000; ++i) {
        read_next_cifar_image(stream, &label, &image);
        feed_image_to_markov(&chain, &image);
        printf("[%3zu] Fed image to Markov (len: %zu)\n", i, hmlenu(chain));
    }

/* #ifdef DEBUG */
/*     Next *stats = NULL; */
/*     get_image_stats(&image, &stats); */
/*     uint32_t total = 0; */
/*     for (size_t i = 0; i < hmlenu(stats); total += stats[i++].value); */
/*     assert(total == CIFAR_SIZE * CIFAR_SIZE); */
/*     printf("All good!\n"); */
/* #endif */

    free(image.pixels);
err_mem:
    fclose(stream);
err_file:
    return ret;
}
