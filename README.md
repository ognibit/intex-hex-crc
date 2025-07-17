# intex-hex-crc

A small program to check if a Intel Hex file has all the checksums correct.

The program just check the CRC at the end of every record without verifying the
structure correctness.
It could be used as a quick check of the file in case of network transfer
without using an additional checksum.
It is designed to be integrated into scripts.

It has been tested only on Linux, but it should work on other platforms too.

## Usage

The program takes as input the hex filename and it exits with 1 in case of
errors, 0 in case of success.
If the file does not contains records or they are corrupted, an error message
in printed on the standard error.
No message will be print in case of success.

```
hexcrc file.hex
```

