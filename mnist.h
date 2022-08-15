#ifndef MNIST_H_
#define MNIST_H_

#include "./utils.h"

#define MNIST_SIZE 28
#define MNIST_COMP 1
static_assert(MNIST_SIZE * MNIST_SIZE * MNIST_COMP == 784);

#define MNIST_BATCH_LENGTH 60000

const char* mnist_training_data[] = {
    "./data/mnist-raw/train-images-idx3-ubyte",
};

void read_next_mnist_image(FILE *stream, Image32 *output)
{
    // [offset] [type]          [value]          [description]
    // 0000     32 bit integer  0x00000803(2051) magic number
    // 0004     32 bit integer  60000            number of images
    // 0008     32 bit integer  28               number of rows
    // 0012     32 bit integer  28               number of columns
    // 0016     unsigned byte   ??               pixel
    // 0017     unsigned byte   ??               pixel
    // ........
    // xxxx     unsigned byte   ??               pixel
    // Pixels are organized row-wise. Pixel values are 0 to 255. 0 means background (white), 255 means foreground (black).

    // stream cursor will always be at the beginning of an image
    assert((ftell(stream) - 16) % (MNIST_SIZE * MNIST_SIZE) == 0);

    static uint8_t pixels[MNIST_SIZE * MNIST_SIZE * MNIST_COMP];
    fread(pixels, sizeof(pixels), 1, stream);

    for (size_t y = 0; y < MNIST_SIZE; ++y) {
        for (size_t x = 0; x < MNIST_SIZE; ++x) {
            size_t index = y * MNIST_SIZE + x;
            output->pixels[index] = (pixels[index] << 8) | 0xFF000000;
        }
    }
}

#endif // MNIST_H_
