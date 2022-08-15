#ifndef MARKOV_H_
#define MARKOV_H_

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "./utils.h"
#include "./config.h"

/**
 * markov :: {state: {pixel: count}}
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

State make_state(Image32 *image, size_t x, size_t y)
{
    State state = {0};

    for (size_t i = 1; i <= STATE_SIZE; ++i) {
        if (y >= i)
            state.vert[i - 1] = *IMAGE32_GET(*image, x, y - i);

        if (x >= i)
            state.horz[i - 1] = *IMAGE32_GET(*image, x - i, y);
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

    assert(0); // unreachable
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
                /* size_t r_state = rand() % num_states; */
                /* *IMAGE32_GET(*output, x, y) = get_random_next_pixel((*chain)[r_state].value); */

                *IMAGE32_GET(*output, x, y) = 0xFF000000;
            }
            else {
                *IMAGE32_GET(*output, x, y) = get_random_next_pixel((*chain)[index].value);
            }
        }
    }
}

#endif // MARKOV_H_
