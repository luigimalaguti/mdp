# MOBI

> **Multimedia data processing**
>
> Data: *2025-06-13*

The MOBI format is used by several eBook readers, including the famous Amazon Kindle. The format is an extension of the Palm Database Format (PDB) which consists of a header of fixed size, a variable number of information on the records (Record Info Entry), followed by the same number of records. **All numeric values are encoded in Big Endian**.

The PDB header for the MOBI format has the following structure:

| Offset | Bytes | Content            | Comments                           |
| :----- | :---- | :----------------- | :--------------------------------- |
| 0      | 32    | name               | Database name (0 terminated)       |
| 32     | 2     | attributes         | (not important)                    |
| 34     | 2     | version            | file version                       |
| 36     | 4     | creation date      | Number of seconds since 01/01/1904 |
| 40     | 4     | modification date  | Number of seconds since 01/01/1904 |
| 44     | 4     | last backup date   | Number of seconds since 01/01/1904 |
| 48     | 4     | modificationNumber | (not important)                    |
| 52     | 4     | appInfoID          | (not important)                    |
| 56     | 4     | sortInfoID         | (not important)                    |
| 60     | 4     | type               | "BOOK"                             |
| 64     | 4     | creator            | "MOBI"                             |
| 68     | 4     | uniqueIDseed       | (not important)                    |
| 72     | 4     | nextRecordListID   | (not important)                    |
| 76     | 2     | numberOfRecords    | Number of records                  |

This is then followed by *numberOfRecords* Record Info Entries which have the following structure:

| Bytes | Content          | Comments                      |
| :---- | :--------------- | :---------------------------- |
| 4     | recordDataOffset | Offset in current record file |
| 1     | recordAttributes | (not important)               |
| 3     | uniqueID         | Unique record identifier      |

The first record (usually with uniqueID = 0) contains information about the eBook and in particular begins with a PalmDOC Header which has the following structure:

| Offset | Bytes | Content        | Comments                                                                     |
| :----- | :---- | :------------- | :--------------------------------------------------------------------------- |
| 0      | 2     | Compression    | 1 == no compression, 2 = PalmDOC compression, 17480 = HUFF/CDIC compression  |
| 2      | 2     | Unused         | (not important)                                                              |
| 4      | 4     | TextLength     | Length of the entire uncompressed text.                                      |
| 8      | 2     | RecordCount    | Number of PDB records used for text.                                         |
| 10     | 2     | RecordSize     | Maximum size of encoded text in a record (always 4096)                       |
| 12     | 2     | EncryptionType | 0 == no encryption, 1 = Old Mobipocket Encryption, 2 = Mobipocket Encryption |
| 14     | 2     | Unknown        | (not important)                                                              |

In this record there is further data that we will not consider.

After this record, subsequent *RecordCount* records contain text compressed with a variant of LZ77. In particular, for each byte of the compressed record its value is checked and various actions are performed:

- **00**: Stops decoding of the current record
- **01-08**: The next 1-8 bytes are copied to output
- **09-7F**: The byte is output as is.
- **80-BF**: LZ77 style decoding is applied as follows. The current byte (80-BF) **and the next** are treated as a single 16-bit sequence and decoded as:

    ```
    first byte           |          second byte
    | 1 | 0 | x | x | x | x | x | x | x | x | x | x | x | x | x | x |
    | fixed |                 distance                  |  length-3 |
    ```

1. The two most significant bits of the first `80-BF` byte are always `10`.
2. The other 6 together with the 5 most significant of the next byte (11 bits in total) form an integer from 0 to 2047, with the value 0 never occurring.
3. The last 3 of the next byte provide a value from 0 to 7 which must be increased by 3 and therefore a length from 3 to 10.

You must then move back *distance* (1-2047) bytes in the data decompressed so far and copy length (3-10) bytes in output starting from that position.

- **C0-FF**: An encoding is applied for space+character pairs: a space is output followed by the current byte (C0-FF) to which the most significant bit is reset. `C0` gives `20 40`, `E5` gives `20 65`, `FF` gives `20 7F`.

## Your task

Write a command line program that can extract uncompressed text from files in this format. The program must support the following syntax:

```bash
MOBIdecode <input filename> <output filename>
```

The two arguments are respectively the name of the file to decode and the one to produce as output. All arguments are mandatory.

Follow the following intermediate steps in preparing the solution (make sure that when moving to a subsequent step the previous one is not destroyed!):

1. Analyze the command line, open the input file, create the output file, write the Byte Order Mark (BOM) for UTF-8 i.e. the 3 bytes `EF BB BF` into it, read the PDB header and print as output:

    ```
    PDB name: <database name>↵
    Creation date (s): <creation date>↵
    Type: <type>↵
    Creator: <creator>↵
    Records: <numberOfRecords>↵
    ↵
    ```
2. Read all *Record Info Entries* and print as output (after the previous prints):

    ```
    0 - offset: <recordDataOffset of record 0> - id: <uniqueID of record 0>↵
    1 - offset: <recordDataOffset of record 1> - id: <uniqueID of record 1>↵
    ...
    ↵
    ```
3. Move to the offset of the first record, read the *PalmDOC* Header and print as output (after the previous prints):

    ```
    Compression: <Compression>↵
    TextLength: <TextLength>↵
    RecordCount: <RecordCount>↵
    RecordSize: <RecordSize>↵
    EncryptionType: <EncryptionType>↵
    ↵
    ```
4. Move to the offset of the next record, decode it with the algorithm described previously until obtaining 4096 bytes of output. At that point the first record is finished and any subsequent bytes must be ignored. Save the decoding of the first record to the output file (after the previously written BOM). The output file is HTML with some particular tags, but if opened with any browser it should appear correctly without strange symbols.
5. Conclude the exercise by repeating the decoding on the following *RecordCount* records, considering that a record is concluded when 4096 bytes have been obtained by decoding that record. For the last record, stop decoding when the output size is *TextLength* bytes. After these records, there are others that contain indexes, bookmarks and images, but they must be ignored.

**Beware of Big Endian - Only submit code that compiles - Do not destroy what has been done until a certain step to move on to the next (copy paste of the entire solution)**.
