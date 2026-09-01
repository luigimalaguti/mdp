# TIFF

> **Multimedia data processing**
>
> Data: *2026-05-26*

Write a command-line program that accepts the following options:

```bash
tiff2pam <input file .TIFF> <output file .PAM>
```

The program must read two parameters from the command line: the first is the name of the input file, the second the name of the output file.

The program must decode the header of images encoded according to the TIFF standard and decode images at gray levels (8 bits per pixel), saving the output in a PAM image. The TIFF standard provides countless variations and options, but for this exercise we will limit ourselves to considering only the subset of images for which the following considerations apply:

```
Little endian encoding (header starts with II)

ImageWidth = image width
ImageLength = image height
BitsPerSample = 8 (8 bits per pixel)
Compression = 1 (no compression)
PhotometricInterpretation = 1 (the value 0 corresponds to black)
RowsPerStrip = ImageLength (there is only one "strip" that contains the entire image)
StripByteCounts = ImageWidth*ImageLength (the strip is as large as the number of pixels in the image)
```

Read the standard carefully, especially the structure of the header and the IFD. The necessary information is in section 1 (Baseline TIFF).
