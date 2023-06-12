CC := gcc
LDLIBS := -lXi -lX11
LDFLAGS :=
CFLAGS := -O3 --std=c99 -Wall -pedantic -Wno-parentheses -fomit-frame-pointer
DEBUG := $(CFLAGS) -O0 -ggdb -fvar-tracking-assignments -fvar-tracking -fno-builtin
BIN := xkbcat


all:
	make $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%: %.o
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -f *~ *.o $(BIN)

debug:
	CFLAGS="$(DEBUG)" make

help:
	echo "make <all|clean|test|help>"


.SILENT: all clean test help
