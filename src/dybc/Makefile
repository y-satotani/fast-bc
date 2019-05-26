CC := gcc
IDIR := /usr/include/igraph
LDIR := /usr/lib/x86_64-linux-gnu/
LIBS := -lm -ligraph
CFLAGS := -fpic -g -O3 -march=native -Wall \
          $(addprefix -I,$(IDIR)) $(addprefix -L,$(LDIR))

OBJ := $(patsubst %.c,%.o,$(shell ls | egrep ".\c$$"))

.PHONY: all clean

all: libdybc.so

libdybc.so: $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS)

clean:
	rm -f *.o *.so
