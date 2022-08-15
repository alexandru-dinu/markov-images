#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "./markov.h"
#include "./utils.h"
#include "./cifar.h"
#include "./config.h"
static_assert(NUM_TRAINING_IMAGES <= CIFAR_BATCH_LENGTH * ARRAY_LEN(cifar_training_data));

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
            return NULL;
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

int main()
{
    srand(time(0));

    // train a Markov chain and generate a single image
    Markov *chain = train_markov_cifar(NUM_TRAINING_IMAGES);
    {
        for (size_t i = 0; i < NUM_OUTPUT_IMAGES; ++i) {
            char output_file_name[64] = {0};
            sprintf(output_file_name, OUTPUT_FILE_FMT, i);

            Image32 output = {
                .width  = CIFAR_SIZE,
                .height = CIFAR_SIZE,
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
