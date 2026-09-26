/*
 * huffman.c
 * ---------
 * Everything to do with the Huffman tree:
 *   1. count byte frequencies in a file
 *   2. build the tree using the min heap
 *   3. traverse the tree to generate codes
 *   4. compress / decompress files using the codes
 *
 * Compressed file layout (.huf):
 *   [3 bytes] the letters "HUF" so we can tell it is our file
 *   [int]  number of distinct symbols (n)
 *   n x ( [unsigned char] symbol, [int] frequency )
 *   [int]  total number of bytes in the original file
 *   [...]  the Huffman codes of every byte, packed 8 bits per byte
 *
 * The decompressor reads the frequencies back, rebuilds the SAME tree
 * with the SAME code, and walks the tree bit by bit to recover the bytes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

/* Allocate a new tree node. */
struct Node *createNode(unsigned char ch, int freq, struct Node *left, struct Node *right)
{
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    node->ch = ch;
    node->freq = freq;
    node->left = left;
    node->right = right;
    return node;
}

/* A node is a leaf if it has no children. Leaves hold real symbols. */
static int isLeaf(struct Node *node)
{
    return node->left == NULL && node->right == NULL;
}

/* Print a symbol in a readable way, always 4 characters wide:
 * 'a'  for printable bytes, 0x0A for everything else. */
static void printSymbol(unsigned char ch)
{
    if (ch >= 32 && ch <= 126)
        printf("'%c' ", ch);
    else
        printf("0x%02X", ch);
}

/* Step 1: read the whole file once and count how often each byte appears. */
void countFrequencies(FILE *in, int freq[NUM_SYMBOLS])
{
    int c;
    for (int i = 0; i < NUM_SYMBOLS; i++)
        freq[i] = 0;

    while ((c = fgetc(in)) != EOF)
        freq[c]++;                      /* c is 0..255, used directly as the index */
}

/* Step 2: build the Huffman tree.
 *   - put every symbol with freq > 0 into the min heap as a leaf
 *   - repeatedly take the two smallest, join them under a new parent,
 *     and put the parent back
 *   - the last node left in the heap is the root
 * Returns NULL for an empty file. */
struct Node *buildHuffmanTree(int freq[NUM_SYMBOLS])
{
    struct MinHeap *heap = createHeap(NUM_SYMBOLS);

    for (int i = 0; i < NUM_SYMBOLS; i++) {
        if (freq[i] > 0)
            insertHeap(heap, createNode((unsigned char)i, freq[i], NULL, NULL));
    }

    while (heap->size > 1) {
        struct Node *left = extractMin(heap);
        struct Node *right = extractMin(heap);
        struct Node *parent = createNode(0, left->freq + right->freq, left, right);
        insertHeap(heap, parent);
    }

    struct Node *root = extractMin(heap);   /* NULL if the heap was empty */
    freeHeap(heap);
    return root;
}

/* Step 3: depth-first traversal of the tree to generate codes.
 *   going left  appends '0' to the path
 *   going right appends '1' to the path
 * When we reach a leaf, the path so far is that symbol's code.
 * Special case: a tree with only ONE symbol is just a root leaf, so we
 * give it the code "0" (otherwise its code would be empty). */
void generateCodes(struct Node *node, char codes[NUM_SYMBOLS][MAX_CODE_LEN],
                   char path[], int depth)
{
    if (node == NULL)
        return;

    if (isLeaf(node)) {
        if (depth == 0) {
            path[0] = '0';
            depth = 1;
        }
        path[depth] = '\0';
        strcpy(codes[node->ch], path);
        return;
    }

    path[depth] = '0';
    generateCodes(node->left, codes, path, depth + 1);

    path[depth] = '1';
    generateCodes(node->right, codes, path, depth + 1);
}

/* Print the code table so we can see what the algorithm produced. */
void printCodeTable(int freq[NUM_SYMBOLS], char codes[NUM_SYMBOLS][MAX_CODE_LEN])
{
    printf("=== Code table ===\n");
    printf("Symbol Freq   Code\n");
    for (int i = 0; i < NUM_SYMBOLS; i++) {
        if (freq[i] == 0)
            continue;
        printSymbol((unsigned char)i);
        printf("   %-6d %s\n", freq[i], codes[i]);
    }
}

/* Print the tree with a pre-order traversal (node first, then left, then right).
 * Each level is indented a little more, so the shape of the tree is visible.
 * Internal nodes are shown as [sum], leaves as symbol(freq). */
void printTree(struct Node *node, int depth)
{
    if (node == NULL)
        return;

    for (int i = 0; i < depth; i++)
        printf("    ");

    if (isLeaf(node)) {
        printSymbol(node->ch);
        printf("(%d)\n", node->freq);
    } else {
        printf("[%d]\n", node->freq);
    }

    printTree(node->left, depth + 1);
    printTree(node->right, depth + 1);
}

/* Free every node using a post-order traversal (children before parent). */
void freeTree(struct Node *node)
{
    if (node == NULL)
        return;
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

/* Step 4a: compress inPath into outPath. Returns 0 on success. */
int compressFile(const char *inPath, const char *outPath)
{
    FILE *in = fopen(inPath, "rb");
    if (in == NULL) {
        printf("Error: cannot open %s\n", inPath);
        return 1;
    }

    int freq[NUM_SYMBOLS];
    countFrequencies(in, freq);

    int total = 0;
    int distinct = 0;
    for (int i = 0; i < NUM_SYMBOLS; i++) {
        total += freq[i];
        if (freq[i] > 0)
            distinct++;
    }

    struct Node *root = buildHuffmanTree(freq);

    char codes[NUM_SYMBOLS][MAX_CODE_LEN];
    char path[MAX_CODE_LEN];
    generateCodes(root, codes, path, 0);

    FILE *out = fopen(outPath, "wb");
    if (out == NULL) {
        printf("Error: cannot create %s\n", outPath);
        fclose(in);
        freeTree(root);
        return 1;
    }

    /* --- write the header --- */
    fwrite(MAGIC, sizeof(char), 3, out);
    fwrite(&distinct, sizeof(int), 1, out);
    for (int i = 0; i < NUM_SYMBOLS; i++) {
        if (freq[i] > 0) {
            unsigned char symbol = (unsigned char)i;
            fwrite(&symbol, sizeof(unsigned char), 1, out);
            fwrite(&freq[i], sizeof(int), 1, out);
        }
    }
    fwrite(&total, sizeof(int), 1, out);

    /* --- write the encoded bits ---
     * We collect bits one at a time into 'buffer'. When 8 bits are ready
     * we write one byte and start again. */
    rewind(in);
    unsigned char buffer = 0;
    int bitCount = 0;
    int c;

    while ((c = fgetc(in)) != EOF) {
        char *code = codes[c];
        for (int k = 0; code[k] != '\0'; k++) {
            buffer = (buffer << 1) | (code[k] - '0');   /* shift left, add the new bit */
            bitCount++;
            if (bitCount == 8) {
                fputc(buffer, out);
                buffer = 0;
                bitCount = 0;
            }
        }
    }

    /* Flush the last partial byte, padding the right side with zeros. */
    if (bitCount > 0) {
        buffer = buffer << (8 - bitCount);
        fputc(buffer, out);
    }

    long compressedSize = ftell(out);
    fclose(in);
    fclose(out);

    printf("=== Summary ===\n");
    printf("Original size   : %d bytes\n", total);
    printf("Compressed size : %ld bytes\n", compressedSize);
    printf("Distinct symbols: %d\n", distinct);

    printCodeTable(freq, codes);
    printf("=== Huffman tree ===\n");
    printTree(root, 0);

    freeTree(root);
    return 0;
}

/* Step 4b: decompress inPath into outPath. Returns 0 on success. */
int decompressFile(const char *inPath, const char *outPath)
{
    FILE *in = fopen(inPath, "rb");
    if (in == NULL) {
        printf("Error: cannot open %s\n", inPath);
        return 1;
    }

    /* --- read the header and rebuild the frequency table --- */
    int freq[NUM_SYMBOLS];
    for (int i = 0; i < NUM_SYMBOLS; i++)
        freq[i] = 0;

    char magic[3];
    int distinct = 0;
    if (fread(magic, sizeof(char), 3, in) != 3 || strncmp(magic, MAGIC, 3) != 0
        || fread(&distinct, sizeof(int), 1, in) != 1) {
        printf("Error: %s is not a valid .huf file (was it made by this program?)\n", inPath);
        fclose(in);
        return 1;
    }
    for (int i = 0; i < distinct; i++) {
        unsigned char symbol;
        int count;
        fread(&symbol, sizeof(unsigned char), 1, in);
        fread(&count, sizeof(int), 1, in);
        freq[symbol] = count;
    }
    int total = 0;
    fread(&total, sizeof(int), 1, in);

    /* Same frequencies + same code = same tree as the compressor built. */
    struct Node *root = buildHuffmanTree(freq);

    FILE *out = fopen(outPath, "wb");
    if (out == NULL) {
        printf("Error: cannot create %s\n", outPath);
        fclose(in);
        freeTree(root);
        return 1;
    }

    /* --- walk the tree bit by bit ---
     * Start at the root. For every bit: 0 = go left, 1 = go right.
     * When we land on a leaf, output its symbol and jump back to the root. */
    struct Node *node = root;
    int written = 0;
    int c;

    while (written < total && (c = fgetc(in)) != EOF) {
        for (int bit = 7; bit >= 0 && written < total; bit--) {
            int bitValue = (c >> bit) & 1;        /* extract one bit, MSB first */

            if (isLeaf(root)) {
                fputc(root->ch, out);             /* single-symbol file: every bit is that symbol */
                written++;
                continue;
            }

            if (bitValue == 0)
                node = node->left;
            else
                node = node->right;

            if (isLeaf(node)) {
                fputc(node->ch, out);
                written++;
                node = root;
            }
        }
    }

    fclose(in);
    fclose(out);
    freeTree(root);

    printf("Decompressed size: %d bytes\n", written);
    return 0;
}
