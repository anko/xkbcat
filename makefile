xkbcat: xkbcat.c
	clang -O3 --std=gnu11 -pedantic -Wall -lX11 -o xkbcat xkbcat.c
clean:
	rm xkbcat
.PHONY: xkbcat


