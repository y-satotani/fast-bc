CC := gcc
IDIR := /usr/local/include/igraph ../
LDIR := /usr/local/lib ../dybc
LIBS := -lm -ligraph -ldybc
CFLAGS := -g -O3 -march=native -Wall -Wl,-Rdybc \
          $(addprefix -I,$(IDIR)) $(addprefix -L,$(LDIR))

EXE := $(patsubst %.c,%.out,$(shell ls | egrep ".\c$$"))

.PHONY: all clean

all: $(EXE)

debug: CFLAGS += -DDEBUG -g
debug: $(EXE)

%.out: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f *.out
