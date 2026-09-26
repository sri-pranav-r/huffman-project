/*
 * main.c
 * ------
 * Command-line entry point.
 *
 *   ./huff compress   input.txt  output.huf
 *   ./huff decompress output.huf restored.txt
 */

#include <stdio.h>
#include <string.h>
#include "huffman.h"

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Usage:\n");
        printf("  %s compress   <input file>  <output .huf file>\n", argv[0]);
        printf("  %s decompress <input .huf>  <output file>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "compress") == 0)
        return compressFile(argv[2], argv[3]);

    if (strcmp(argv[1], "decompress") == 0)
        return decompressFile(argv[2], argv[3]);

    printf("Unknown command: %s (use compress or decompress)\n", argv[1]);
    return 1;
}
