xkbcat: xkbcat.c
	clang -O3 --std=c99 -pedantic -Wall -lX11 -lXi -o xkbcat xkbcat.c
clean:
	rm --force $(BINARY_NAME)
