# Quite OK Image

> **Multimedia data processing**
>
> Data: *2026-09-04*

The Quite OK Image Format is a novel image format proposed on 24 November 2021 by an independent programmer. It losslessly compresses RGB and RGBA images to a similar size of PNG, while offering (with careful implementation) a 20x-50x speedup in compression and 3x-4x speedup in decompression. The specification is fully contained in the one page file `qoi-specification.pdf`.

## Your task

Write a command line program with the following syntax:

```bash
qoi_decomp <input file> <output file>
```

The program must open a .qoi file encoded with the QOI format, decompress and save it into PAM format. Use always a RGBA pixel format, that is 4 bytes per pixel. The code for PAM format output is already provided in the example file `qoi_decomp.cpp`. Only if you are interested, the PAM format is documented in the file `PAM format specification.html`.
