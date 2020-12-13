xkbcat: xkbcat.c
	$(CC) -O3 --std=c99 -pedantic -Wall xkbcat.c -o xkbcat -lX11 -lXi
clean:
	rm --force xkbcat
