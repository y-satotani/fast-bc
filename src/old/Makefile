CC := gcc
IDIR := /usr/include/igraph
LDIR := /usr/lib/x86_64-linux-gnu/
LIBS := -lm -ligraph
CFLAGS := -g -Ofast -Wall $(addprefix -I,$(IDIR)) $(addprefix -L,$(LDIR))

OBJ := bc_algorithms.o distance_update.o

.PHONY: all clean

all: test_bc.out test_dec.out experiment.out

test_bc.out: test_bc.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

test_dec.out: test_dec.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

experiment.out: experiment.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS)

clean:
	rm -f *.o *.out
