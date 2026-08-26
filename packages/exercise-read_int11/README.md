# Read int11

> **Multimedia data processing**
>
> Data: *2026-03-11*

Write a command line program in C ++ with this syntax:

```bash
read_int11 <filein.bin> <fileout.txt>
```

The first parameter is the name of a binary file that contains 11-bit numbers in 2's complement, with the bits sorted from most significant to least significant. The program must create a new file, with the name passed as the second parameter, with the same numbers saved in decimal text format separated by a new line. Ignore any excess bits in the last byte.
