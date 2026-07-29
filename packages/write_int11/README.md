# Write int11

> **Multimedia data processing**
>
> Data: *2026-03-11*

Write a command line program in C++ with this syntax:

```bash
write_int11 <filein.txt> <fileout.bin>
```

The first parameter is the name of a text file that contains base 10 integers from -1000 to 1000 separated by whitespace. The program must create a new file, with the name passed as the second parameter, with the same numbers saved as 11-bit binary in 2's complement. The bits are inserted in the file from the most significant to the least significant. The last byte of the file, if incomplete, is filled with bits equal to 0. Since the incomplete byte will have at most 7 padding bits, there's no risk of interpreting padding as another value.
