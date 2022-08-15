#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "./config.h"

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

/**
 * markov: {
 *   state: {pixel: count}
 * }
 */
typedef struct {
    uint32_t vert[STATE_SIZE];
    uint32_t horz[STATE_SIZE];
} State;

typedef struct {
    uint32_t key;   // pixel
    uint32_t value; // count
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
            feed_pixel_to_stats(stats, *IMAGE32_GET(*image, x, y));
}

uint32_t round_pixel(uint32_t pixel, uint8_t scale)
{
    // ABGR (each 1 byte)
    uint8_t b = ((pixel >> 8 * 2) & 0xFF) / scale;
    uint8_t g = ((pixel >> 8 * 1) & 0xFF) / scale;
    uint8_t r = ((pixel >> 8 * 0) & 0xFF) / scale;

    return r | (g << 8) | (b << 16) | 0xFF000000;
}

State make_state(Image32 *image, size_t x, size_t y)
{
    State state = {0};

    for (size_t i = 1; i <= STATE_SIZE; ++i) {
        if (y >= i)
            state.vert[i - 1] = round_pixel(*IMAGE32_GET(*image, x, y - i), 1);

        if (x >= i)
            state.horz[i - 1] = round_pixel(*IMAGE32_GET(*image, x - i, y), 1);
    }

    return state;
}

void feed_image_to_markov(Markov **chain, Image32 *image)
{
    for (size_t y = 0; y < image->height; ++y) {
        for (size_t x = 0; x < image->width; ++x) {
            // state leading to the pixel[y, x]
            State state = make_state(image, x, y);
            uint32_t pixel = *IMAGE32_GET(*image, x, y);
            ptrdiff_t index = hmgeti(*chain, state);

            if (index < 0) {
                // add new { state: {pixel: 1} }
                Next *next = NULL;
                hmput(next, pixel, 1);
                hmput(*chain, state, next);
            }
            else {
                feed_pixel_to_stats(&((*chain)[index].value), pixel);
            }
        }
    }
}

uint32_t get_random_next_pixel(Next *stats)
{
    size_t num_stats = hmlenu(stats);

    size_t total = 0;
    for (size_t i = 0; i < num_stats; total += stats[i++].value);

    size_t r = 1 + rand() % total;
    size_t s = 0;

    for (size_t i = 0; i < num_stats; ++i) {
        s += stats[i].value;
        if (s >= r)
            return stats[i].key;
    }

    assert(0);
}

void generate_image_from_markov(Markov **chain, Image32 *output)
{
    size_t num_states = hmlenu(*chain);
    assert(num_states > 0);

    for (size_t y = 0; y < output->height; ++y) {
        for (size_t x = 0; x < output->width; ++x) {
            State context = make_state(output, x, y);
            ptrdiff_t index = hmgeti(*chain, context);

            if (index < 0) {
                /* *IMAGE32_GET(*output, x, y) = 0xFF0000FF; */

                size_t r_state = rand() % num_states;
                *IMAGE32_GET(*output, x, y) = get_random_next_pixel((*chain)[r_state].value);
            }
            else {
                *IMAGE32_GET(*output, x, y) = get_random_next_pixel((*chain)[index].value);
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


int main()
{
    srand(time(0));
    int ret = 0;

    Image32 image = {0};
    image.width = CIFAR_SIZE;
    image.height = CIFAR_SIZE;
    image.pixels = malloc(sizeof(uint32_t) * image.width * image.height);
    if (image.pixels == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        ret = -1;
        goto err_mem;
    }

    // train a Markov chain and generate a single image
    {
        Markov *chain = NULL;
        uint8_t label;
        size_t seen = 0;

        for (size_t i = 0; i < ARRAY_LEN(cifar_training_data); ++i) {
            FILE *stream = fopen(cifar_training_data[i], "rb");
            if (stream == NULL) {
                fprintf(stderr, "%s\n", strerror(errno));
                ret = -1;
                goto err_file;
            }

            for (size_t j = 0; j < CIFAR_BATCH_LENGTH; ++j) {
                read_next_cifar_image(stream, &label, &image);

                /* if (CIFAR_LABEL == -1 || label == CIFAR_LABEL) { */
                {
                    seen += 1;
                    feed_image_to_markov(&chain, &image);
                    printf("[%zu] Fed image (label: %u) to Markov (len: %zu)\n",
                            seen, label, hmlenu(chain));
                }
            }

            fclose(stream);
        }

        for (size_t i = 0; i < NUM_OUTPUT_IMAGES; ++i) {
            char output_file_name[64] = {0};
            sprintf(output_file_name, "./data/output/%zu.png", i);

            Image32 output = {0};
            output.width = 32;
            output.height = 32;
            output.pixels = malloc(sizeof(uint32_t) * output.width * output.height);

            generate_image_from_markov(&chain, &output);

            save_output_image(&output, output_file_name);
            printf("Generated image %zu\n", i);

            free(output.pixels);
        }
    }

#ifdef DEBUG
    Next *stats = NULL;
    get_image_stats(&image, &stats);
    uint32_t total = 0;
    for (size_t i = 0; i < hmlenu(stats); total += stats[i++].value);
    assert(total == CIFAR_SIZE * CIFAR_SIZE);
    printf("All good!\n");
#endif

err_file:
    free(image.pixels);
err_mem:
    return ret;
}
