CC := gcc
IDIR := /usr/local/include/igraph
LDIR := /usr/local/lib/
LIBS := -lm -ligraph
CFLAGS := -fpic -g -O3 -march=native -Wall \
          $(addprefix -I,$(IDIR)) $(addprefix -L,$(LDIR))

OBJ := $(patsubst %.c,%.o,$(shell ls | egrep ".\c$$"))

.PHONY: all clean

all: libdybc.so

debug: CFLAGS += -DDEBUG -g
debug: libdybc.so

libdybc.so: $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS)

clean:
	rm -f *.o *.so
