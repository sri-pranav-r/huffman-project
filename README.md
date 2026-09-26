# Huffman File Compressor

A DSA mini-project. The compression engine is written in C and uses only
syllabus concepts: dynamic memory allocation, a binary tree, tree traversals,
a binary min heap used as a priority queue, and a frequency table (basic
hashing by direct addressing). A small web page lets you drop a file in and
get the compressed (or restored) file back.

## Folder layout

```
backend/    the C program (this is the project)
  huffman.h   structs + function declarations
  heap.c      min heap: createHeap, insertHeap, extractMin, freeHeap
  huffman.c   frequency count, tree build, code generation, tree printout, compress, decompress
  main.c      command line: ./huff compress|decompress <in> <out>
  Makefile    build with: make
frontend/   thin web UI (not part of the DSA content)
  server.py   Python standard-library web server that runs ./huff
  index.html  the drop-in-file page
samples/    files to test with
```

The compiled program `backend/huff` is not included (mail services and
upload sites block executables). Run `make` in `backend/`, or just start the
web server, and it is built in a second. To try a file that does not
compress well, make a random one with:

```
head -c 5000 /dev/urandom > samples/random.bin
```

## Run it

Build and use from the terminal:

```
cd backend
make
./huff compress   ../samples/sample.txt sample.huf
./huff decompress sample.huf restored.txt
cmp ../samples/sample.txt restored.txt      # no output means identical
```

Compress prints a summary (sizes and number of distinct symbols), the code
table, and the Huffman tree as an indented pre-order traversal. The web page
shows the table and the tree in their own tabs.

Or with the web page:

```
python3 frontend/server.py
```
then open http://localhost:8000 in a browser. The server compiles the C
program automatically the first time if `backend/huff` does not exist.

## How it works

1. **Count frequencies.** Read the file byte by byte and do `freq[c]++` on an
   `int freq[256]` array. The byte value is used directly as the array index,
   which is the simplest form of hashing.
2. **Build the tree with a min heap.** Every symbol with a non-zero count
   becomes a leaf node and is inserted into a min heap ordered by frequency.
   While the heap has more than one node: extract the two smallest, create a
   parent whose frequency is their sum, insert the parent. The last node is
   the root of the Huffman tree.
3. **Generate codes by traversal.** Depth-first traversal from the root.
   Going left appends `0`, going right appends `1`. When a leaf is reached the
   path so far is that symbol's code. Frequent symbols end up near the root
   and get short codes.
4. **Write the compressed file.** A small header (the letters `HUF`, which
   symbols exist and their counts, plus the original byte count), then every byte of the input
   replaced by its code. Bits are packed 8 to a byte using shift and OR.
5. **Decompress.** Check the `HUF` marker (so a random file gives a clean
   error instead of garbage), read the header, rebuild the same tree with the same
   function, then walk the tree one bit at a time: `0` go left, `1` go right,
   on a leaf output the symbol and jump back to the root.
6. **Free memory.** The tree is freed with a post-order traversal so children
   are freed before their parent.
