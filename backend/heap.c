/*
 * heap.c
 * ------
 * A simple array-based Min Heap (priority queue) of Node pointers.
 * The node with the SMALLEST frequency is always at index 0.
 *
 * For a node at index i:
 *   parent      = (i - 1) / 2
 *   left child  = 2 * i + 1
 *   right child = 2 * i + 2
 */

#include <stdlib.h>
#include "huffman.h"

/* Allocate an empty heap that can hold 'capacity' nodes. */
struct MinHeap *createHeap(int capacity)
{
    struct MinHeap *heap = (struct MinHeap *)malloc(sizeof(struct MinHeap));
    heap->size = 0;
    heap->capacity = capacity;
    heap->array = (struct Node **)malloc(capacity * sizeof(struct Node *));
    return heap;
}

/* Swap two node pointers in the heap array. */
static void swapNodes(struct Node **a, struct Node **b)
{
    struct Node *temp = *a;
    *a = *b;
    *b = temp;
}

/* Move the node at index i UP until its parent is smaller. Used after insert. */
static void siftUp(struct MinHeap *heap, int i)
{
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap->array[i]->freq < heap->array[parent]->freq) {
            swapNodes(&heap->array[i], &heap->array[parent]);
            i = parent;
        } else {
            break;
        }
    }
}

/* Move the node at index i DOWN until both children are larger. Used after extract. */
static void siftDown(struct MinHeap *heap, int i)
{
    while (1) {
        int smallest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < heap->size && heap->array[left]->freq < heap->array[smallest]->freq)
            smallest = left;
        if (right < heap->size && heap->array[right]->freq < heap->array[smallest]->freq)
            smallest = right;

        if (smallest == i)
            break;                      /* heap property satisfied */

        swapNodes(&heap->array[i], &heap->array[smallest]);
        i = smallest;
    }
}

/* Insert a node: put it at the end, then sift it up. */
void insertHeap(struct MinHeap *heap, struct Node *node)
{
    if (heap->size == heap->capacity)
        return;                         /* heap is full (never happens in this project) */

    heap->array[heap->size] = node;
    heap->size++;
    siftUp(heap, heap->size - 1);
}

/* Remove and return the minimum node: take the root, move the last node
 * to the root, then sift it down. */
struct Node *extractMin(struct MinHeap *heap)
{
    if (heap->size == 0)
        return NULL;

    struct Node *min = heap->array[0];
    heap->size--;
    heap->array[0] = heap->array[heap->size];
    siftDown(heap, 0);
    return min;
}

/* Free the heap itself (NOT the tree nodes, they belong to the tree). */
void freeHeap(struct MinHeap *heap)
{
    free(heap->array);
    free(heap);
}
