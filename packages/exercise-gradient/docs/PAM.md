# PAM format

The PAM image format is a lowest common denominator 2 dimensional map format. The full specification is available at this [page](http://netpbm.sourceforge.net/doc/pam.html).

Each PAM image consists of a header followed immediately by the image pixels. Here is an example header:

```
P7
WIDTH 227
HEIGHT 149
DEPTH 3
MAXVAL 255
TUPLTYPE RGB
ENDHDR
```

The header begins with the ASCII characters "P7" followed by newline. This is the magic number.

The header continues with an arbitrary number of lines of ASCII text. Each line ends with and is delimited by a newline character (byte with value 10).

Each header line consists of zero or more whitespace-delimited tokens or begins with "#". If it begins with "#" it is a comment and the rest of this specification does not apply to it.

The type of header line is identified by its first token, which is 8 characters or less:

- WIDTH: number of columns
- HEIGHT: number of rows
- DEPTH: number of planes or channels
- MAXVAL: the maximum value which may be found in the image
- TUPLTYPE: the tuple type (GRAYSCALE or RGB)
- ENDHDR: the last line in the header

The raster consists of each row of the image, in order from top to bottom, consecutive with no delimiter of any kind between, before, or after, rows.

Each row consists of every tuple in the row, in order from left to right, consecutive with no delimiter of any kind between, before, or after, tuples.

Each tuple consists of every sample in the tuple, in order, consecutive with no delimiter of any kind between, before, or after, samples.

Each sample consists of an unsigned integer in pure binary format, with the most significant byte first (big endian). The number of bytes is the minimum number of bytes required to represent the maxval of the image.
