# Huffman 2

> **Multimedia data processing**
>
> Data: *2026-03-31*

Variation on the previous one:

```bash
huffman2 [c|d] <input file> <output file>
```

With this format:

| Field        | Size                                                                 | Description                                                                                |
| :----------- | :------------------------------------------------------------------- | :----------------------------------------------------------------------------------------- |
| MagicNumber  | 8 byte                                                               | "HUFFMAN2"                                                                                 |
| TableEntries | 8-bit unsigned integer                                               | Number of items in the following Huffman table (**0 means 256 symbols**).                  |
| HuffmanTable | TableEntries **couples** (sym = 8 bit, len = 5 bit)                  | Table of *symbol*, *length* couples. This table **must** be sorted on len and then on sym. |
| NumSymbols   | 32 bit unsigned integer stored in **big endian**                     | Number of symbols encoded in the file.                                                     |
| Data         | NumSymbols Huffman codes                                             | Values encoded with Huffman codes, according to the previous table.                        |

In this case the Huffman codes are the canonical codes with the most likely one starting from 0.
