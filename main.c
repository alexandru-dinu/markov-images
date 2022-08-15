#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "./markov.h"
#include "./utils.h"
#include "./config.h"

#include "./mnist.h"
static_assert(NUM_TRAINING_IMAGES <= MNIST_BATCH_LENGTH);

/* #include "./cifar.h" */
/* static_assert(NUM_TRAINING_IMAGES <= CIFAR_BATCH_LENGTH * ARRAY_LEN(cifar_training_data)); */

#ifdef MNIST_H_
uint32_t be2le(uint32_t x)
{
    return ((x >> (8 * 3)) & 0xff)     |  // move byte 3 to byte 0
           ((x << (8 * 1)) & 0xff0000) |  // move byte 1 to byte 2
           ((x >> (8 * 1)) & 0xff00)   |  // move byte 2 to byte 1
           ((x << (8 * 3)) & 0xff000000); // move byte 0 to byte 3
}

Markov* train_markov_mnist(size_t num_training_images)
{
    Markov *chain = NULL;
    size_t seen = 0;

    Image32 image = {
        .width  = MNIST_SIZE,
        .height = MNIST_SIZE,
        .pixels = malloc(sizeof(uint32_t) * image.width * image.height),
    };
    assert(image.pixels != NULL);

    FILE *stream = fopen(mnist_training_data[0], "rb");
    if (stream == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        free(image.pixels);
        exit(1);
    }

    uint32_t header[4] = {0};
    fread(header, sizeof(uint32_t), 4, stream);
    assert(be2le(header[0]) == 2051); // magic number
    assert(be2le(header[1]) == MNIST_BATCH_LENGTH);
    assert(be2le(header[2]) == MNIST_SIZE);
    assert(be2le(header[3]) == MNIST_SIZE);

    for (size_t j = 0; j < MNIST_BATCH_LENGTH; ++j) {
        float r = 1.0f * rand() / RAND_MAX;
        if (r >= 0.05) continue;

        read_next_mnist_image(stream, &image);
        feed_image_to_markov(&chain, &image);
        seen += 1;

        if (VERBOSE)
            printf("[%zu] Fed image to Markov (states: %zu)\n", seen, hmlenu(chain));

        if (seen == num_training_images)
            break;
    }

    fclose(stream);
    free(image.pixels);
    return chain;
}
#endif // MNIST_H_

#ifdef CIFAR_H_
Markov* train_markov_cifar(size_t num_training_images)
{
    Markov *chain = NULL;

    uint8_t label;
    size_t seen = 0;

    Image32 image = {
        .width  = CIFAR_SIZE,
        .height = CIFAR_SIZE,
        .pixels = malloc(sizeof(uint32_t) * image.width * image.height),
    };
    assert(image.pixels != NULL);

    for (size_t i = 0; i < ARRAY_LEN(cifar_training_data); ++i) {
        FILE *stream = fopen(cifar_training_data[i], "rb");
        if (stream == NULL) {
            fprintf(stderr, "%s\n", strerror(errno));
            free(image.pixels);
            exit(1);
        }

        for (size_t j = 0; j < CIFAR_BATCH_LENGTH; ++j) {
            read_next_cifar_image(stream, &label, &image);
            feed_image_to_markov(&chain, &image);
            seen += 1;

            if (VERBOSE)
                printf("[%zu] Fed image (label: %u) to Markov (states: %zu)\n",
                        seen, label, hmlenu(chain));

            if (seen == num_training_images) {
                fclose(stream);
                free(image.pixels);
                return chain;
            }
        }

        fclose(stream);
    }

    assert(0); // unreachable
}
#endif // CIFAR_H_

int main()
{
    srand(time(0));

#ifdef MNIST_H_
    Markov *chain = train_markov_mnist(NUM_TRAINING_IMAGES);
    size_t output_size = MNIST_SIZE;
#endif
#ifdef CIFAR_H_
    Markov *chain = train_markov_cifar(NUM_TRAINING_IMAGES);
    size_t output_size = CIFAR_SIZE;
#endif

    {
        for (size_t i = 0; i < NUM_OUTPUT_IMAGES; ++i) {
            char output_file_name[64] = {0};
            sprintf(output_file_name, OUTPUT_FILE_FMT, i);

            Image32 output = {
                .width  = output_size,
                .height = output_size,
                .pixels = malloc(sizeof(uint32_t) * output.width * output.height),
            };
            assert(output.pixels != NULL);

            generate_image_from_markov(&chain, &output);
            save_output_image(&output, output_file_name);
            if (VERBOSE) printf("Generated image %zu\n", i);

            free(output.pixels);
        }
    }

    return 0;
}
