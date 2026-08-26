# Packbits 1

> **Multimedia data processing**
>
> Data: *2026-03-31*

Write a command line program in C++ with this syntax:

```bash
packbits [c|d] <input file> <output file>
```

When the "c" option is specified, the program opens the specified file (the file must be treated as a binary file, that is, it can contain any value from 0 to 255 in each byte), compresses it with the Packbits algorithm and saves it in the output file (since no header is defined, use ".packbits" extension). When the "d" option is specified, the program tries to decompress the file content.

If the input were:

```
aaaaabbbbbcdefghaaaaaaaaaaa
```

The output file would be (every box is a byte):

```
FC 61 FC 62 05 63 64 65 66 67 68 F6 61 80
```

**Implementation Notes**

Initially treat each repetition as a run. Remember that nor runs, nor copies can be longer than 128 bytes.

**General Important Notes**

In general, solutions should avoid loading the entire file into memory or keep the entire sequence compressed or decompressed in memory. Everything should be done by reducing as much as possible the amount of data kept in RAM.

For the first solution it's acceptable to leave everything in memory, but try later to find a solution that does not require it. Try your algorithms with BIG files.
