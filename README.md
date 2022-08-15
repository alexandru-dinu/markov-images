# Simple image generation using Markov chains

Inspired by a [stream](https://www.twitch.tv/videos/1526029909) from [@rexim](https://github.com/rexim).

## Usage
First, make data dirs, e.g.:
```console
$ ls ./data
cifar-10-batches-bin/  output/
```
Next, adjust [`config.h`](./config.h) accordingly, then:
```
make && ./main
```
You can view the images, e.g. using [`feh`](https://linux.die.net/man/1/feh):
```
feh data/output
```

## Datasets
- [CIFAR-10](https://www.cs.toronto.edu/~kriz/cifar.html)

## Dependencies
`stb_image_write.h` and `stb_ds.h` from https://github.com/nothings/stb.
