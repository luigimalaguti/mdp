# Sort int

Write a command line program in C language with this syntax:

```bash
sort_int <filein.txt> <fileout.txt>
```

The first parameter is the name of a text file that contains text formatted base 10 numbers representable as 32 bits signed integers, separated by whitespace. The program must create a new file, with the name passed as the second parameter, with the same numbers sorted from the smallest to the largest, each followed by <new line>.

For example, the file containing:

```
120↵
35 -98↵
100000↵
87↵
236 85
```

would output the file

```
-98↵
35↵
85↵
87↵
120↵
236↵
100000↵
```

If the input file contains incorrect data, the program limits itself to the data it has managed to read so far and ignores the rest. For example the file containing:

```
120↵
35 -98x↵
87↵
236 85
```

would output the file

```
-98↵
35↵
120↵
```

because after reading -98, the x would block the input.

If the output file exists, it will be overwritten. The program returns 0 if the command line parameters are correct and if it can read the input file and write the output file, otherwise it returns 1.

Avoid arbitrary limitations on the number of elements in the input file. You can assume that the amount of RAM available is sufficient to accommodate the entire file.

Make sure it works with the file containing:

```
-2000000000 2000000000 -2100000000 2100000000
```
