CC := gcc
IDIR := /usr/include/igraph ../
LDIR := /usr/lib/x86_64-linux-gnu/ ../bcdec
LIBS := -lm -ligraph -lbcdec
CFLAGS := -g -O3 -march=native -Wall -Wl,-R../bcdec \
          $(addprefix -I,$(IDIR)) $(addprefix -L,$(LDIR))

EXE := $(patsubst %.c,%.out,$(shell ls | egrep ".\c$$"))

.PHONY: all clean

all: $(EXE)

%.out: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f *.out
