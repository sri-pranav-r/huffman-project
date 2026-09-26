/*
 * huffman.h
 * ---------
 * Shared structures and function declarations for the Huffman
 * compression project.
 *
 * Data structures used (from the DSA syllabus):
 *   - Frequency table : a plain array of 256 ints (direct addressing)
 *   - Min Heap        : array-based priority queue of Node pointers
 *   - Binary Tree     : the Huffman tree built from the heap
 *   - Tree traversal  : used to generate codes and to free the tree
 *   - Dynamic memory  : malloc / free for every node and the heap
 */

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>

#define NUM_SYMBOLS 256      /* one slot for every possible byte value */
#define MAX_CODE_LEN 256     /* a Huffman code can never be longer than this */
#define MAGIC "HUF"          /* first 3 bytes of every .huf file, so we can recognise one */

/* One node of the Huffman binary tree. */
struct Node {
    unsigned char ch;        /* the byte this leaf represents (unused for internal nodes) */
    int freq;                /* how many times it occurs (sum of children for internal nodes) */
    struct Node *left;       /* left child  (edge labelled 0) */
    struct Node *right;      /* right child (edge labelled 1) */
};

/* A min heap that stores pointers to tree nodes, ordered by freq. */
struct MinHeap {
    int size;                /* how many nodes are currently in the heap */
    int capacity;            /* maximum number of nodes the array can hold */
    struct Node **array;     /* dynamically allocated array of node pointers */
};

/* ---- heap.c ---- */
struct MinHeap *createHeap(int capacity);
void insertHeap(struct MinHeap *heap, struct Node *node);
struct Node *extractMin(struct MinHeap *heap);
void freeHeap(struct MinHeap *heap);

/* ---- huffman.c ---- */
struct Node *createNode(unsigned char ch, int freq, struct Node *left, struct Node *right);
void countFrequencies(FILE *in, int freq[NUM_SYMBOLS]);
struct Node *buildHuffmanTree(int freq[NUM_SYMBOLS]);
void generateCodes(struct Node *node, char codes[NUM_SYMBOLS][MAX_CODE_LEN],
                   char path[], int depth);
void printCodeTable(int freq[NUM_SYMBOLS], char codes[NUM_SYMBOLS][MAX_CODE_LEN]);
void printTree(struct Node *node, int depth);
void freeTree(struct Node *node);
int compressFile(const char *inPath, const char *outPath);
int decompressFile(const char *inPath, const char *outPath);

#endif
