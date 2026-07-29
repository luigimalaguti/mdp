# Frequencies

> **Multimedia data processing**
>
> Data: *2026-03-11*

Write a command-line C++ program that accepts the following syntax:

```bash
frequencies <input_file> <output_file>
```

The program takes a binary file as input and for each byte (interpreted as an unsigned 8-bit integer) it counts its occurrences. The output is a text file consisting of one line for each different byte found in the input file with the following format:

```
<byte><tab><occurrences><new line>
```

The byte is represented with its two-digit hexadecimal value, occurrences in base ten. The rows are sorted by byte value, from the smallest to the largest.
