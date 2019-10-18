TARGETS := $(patsubst %.R,%.tex,$(shell ls | grep R))

.PHONY: all clean

all: $(TARGETS)

%.tex: %.R
	Rscript $<

clean:
	rm *.tex
