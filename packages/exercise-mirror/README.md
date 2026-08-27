# Mirror

> **Multimedia data processing**
>
> Data: *2026-04-27*

Write a command line program in C++ with this syntax:

```bash
mirror <input file> <output file>
```

The program opens the specified input image in PAM format (which should be a color image in RGB) and creates a "mirrored" version, that is, the first column on the left becomes the last on the right of the new image, the second becomes the penultimate and so on. Save the image in PAM format with the specified filename. Verify that the image is viewable in XnView.

The program returns 0 if everything is fine, 1 if something went wrong.
