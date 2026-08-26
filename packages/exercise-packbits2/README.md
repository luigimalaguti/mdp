# Packbits 2

> **Multimedia data processing**
>
> Data: *2026-03-31*

As a further extension of the previous exercise, create another version of the program that, if there is a copy of bytes followed by a 2-byte run, does not generate the run, but incorporates it into the copy, allowing for further copying. For example, consider the sequence:

```
xyaabcd
```

Compressed with the basic algorithm, it would become:

```
+---+---+---+---+---+---+---+---+---+---+
| 1 | x | y |255| a | 2 | b | c | d |128|
+---+---+---+---+---+---+---+---+---+---+
```

With the considered variant, instead:

```
+---+---+---+---+---+---+---+---+---+
| 6 | x | y | a | a | b | c | d |128|
+---+---+---+---+---+---+---+---+---+
```

Leading to a 1 byte saving. Obviously, if the 2-byte run had been followed by another run, there would have been no savings.

**General Important Notes**

In general, solutions should avoid loading the entire file into memory or keep the entire sequence compressed or decompressed in memory. Everything should be done by reducing as much as possible the amount of data kept in RAM.

For the first solution it's acceptable to leave everything in memory, but try later to find a solution that does not require it. Try your algorithms with BIG files.
