# Build the huff program: make
# Remove build output:    make clean

CC = cc
CFLAGS = -Wall -Wextra -std=c99

huff: main.c heap.c huffman.c huffman.h
	$(CC) $(CFLAGS) -o huff main.c heap.c huffman.c

clean:
	rm -f huff
