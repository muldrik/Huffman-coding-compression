# Huffman codes file compression

The program implements compression using Huffman-coding algorithm

**Command line arguments, separated by spaces:**

* `-c:` compress
* `-u:` uncompress
* `-f, --file <path>`: input file name
* `-o, --output <path>`: output file name
The program prints compression statistics: input data size, output data size and memory used to store encoding information in bytes.

 Example
```
$ ./huffman -c -f myfile.txt -o result.bin
15678
6172
482
```
Input file size (initial data): 15678 bytes, output data size (excluding encoding info): 6172 bytes, encoding info size: 482 bytes. Compressed file size: 6172 + 482 = 6654 bytes.

```
$ ./huffman -u -f result.bin -o myfile_new.txt
6172
15678
482
```
Output file size (uncompressed data): 15678 bytes, compressed data (excluding encoding info): 6172 bytes, encoding info size: 482 bytes. Compressed file size: 6172 + 482 = 6654 bytes.

Makefile:

 * `make` builds executable hw_02 to obj/ directory (created on build in doesn't exist)

 * `make test` builds executable hw_02_test to obj/ directory

 * `make clean` cleans the obj/ directory
