#ifndef CONFIG_H_
#define CONFIG_H_

#define STATE_SIZE 4

#define CIFAR_SIZE 32
#define CIFAR_COMP 3
static_assert(CIFAR_SIZE * CIFAR_SIZE * CIFAR_COMP == 3072);

/* #define NUM_TRAINING_IMAGES 100000 */
#define CIFAR_LABEL 3

#define CIFAR_BATCH_LENGTH 10000
const char* cifar_training_data[] = {
    /* "./data/cifar-10-batches-bin/data_batch_1.bin", */
    /* "./data/cifar-10-batches-bin/data_batch_2.bin", */
    "./data/cifar-10-batches-bin/data_batch_3.bin",
    /* "./data/cifar-10-batches-bin/data_batch_4.bin", */
    "./data/cifar-10-batches-bin/data_batch_5.bin",
};

#define NUM_OUTPUT_IMAGES 10

#endif // CONFIG_H_
