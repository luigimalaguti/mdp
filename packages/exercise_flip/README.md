# Flip

> **Multimedia data processing**
>
> Data: *2026-04-23*

Write a command line program in C++ with this syntax:

```bash
flip <input file> <output file>
```

The program opens the specified input image in PAM format (which should be a graylevel image) and creates an "upside down" version, that is, the first line at the top becomes the last at the bottom of the new image, the second becomes the penultimate and so on. Save the image in PAM format with the specified filename. Verify that the image is viewable in XnView.

The program returns 0 if everything is fine, 1 if something went wrong.
