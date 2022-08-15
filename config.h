#ifndef CONFIG_H_
#define CONFIG_H_

#define STATE_SIZE          4
#define NUM_TRAINING_IMAGES 16000
#define NUM_OUTPUT_IMAGES   32
#define OUTPUT_FILE_FMT     "./data/output/%zu.png"
#define VERBOSE             1 // 0 or 1

const char* cifar_training_data[] = {
    "./data/cifar-10-batches-bin/data_batch_1.bin",
    "./data/cifar-10-batches-bin/data_batch_2.bin",
    "./data/cifar-10-batches-bin/data_batch_3.bin",
    "./data/cifar-10-batches-bin/data_batch_4.bin",
    "./data/cifar-10-batches-bin/data_batch_5.bin",
};

#endif // CONFIG_H_
