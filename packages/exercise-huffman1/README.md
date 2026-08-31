# Huffman 1

> **Multimedia data processing**
>
> Data: *2026-03-31*

Write a command line program in C++ with this syntax:

```bash
huffman1 [c|d] <input file> <output file>
```

When the "c" option is specified, the program opens the specified file (the file must be treated as a binary file, that is, it can contain any value from 0 to 255 in each byte), calculates the frequencies and generates the corresponding Huffman codes. Then it produces a file with the following format:

| Field        | Size                                                                 | Description                                                               |
| :----------- | :------------------------------------------------------------------- | :------------------------------------------------------------------------ |
| MagicNumber  | 8 byte                                                               | "HUFFMAN1"                                                                |
| TableEntries | 8-bit unsigned integer                                               | Number of items in the following Huffman table (**0 means 256 symbols**). |
| HuffmanTable | TableEntries **triplets** (sym = 8 bit, len = 5 bit, code = len bit) | Triplet table with *symbol*, *length* and *Huffman code*: the length and the Huffman code is specified for each symbol. The code is written with as many bits as indicated in the triplet len field. |
| NumSymbols   | 32 bit unsigned integer stored in **big endian**                     | Number of symbols encoded in the file.                                    |
| Data         | NumSymbols Huffman codes                                             | Values encoded with Huffman codes, according to the previous table.       |

When the "d" option is specified, the program decompresses the contents of the input file (check that it's stored in the previous format) and saves it in the output file.
