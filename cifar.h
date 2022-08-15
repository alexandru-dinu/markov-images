#ifndef CIFAR_H_
#define CIFAR_H_

#include "./utils.h"

#define CIFAR_SIZE 32
#define CIFAR_COMP 3
static_assert(CIFAR_SIZE * CIFAR_SIZE * CIFAR_COMP == 3072);

#define CIFAR_BATCH_LENGTH 10000

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

#endif // CIFAR_H_
