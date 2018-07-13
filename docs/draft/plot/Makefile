TARGETS := $(patsubst %.R,%.pdf,$(shell ls | grep R))

.PHONY: all clean

all: $(TARGETS)

%.pdf: %.R
	Rscript $<
clean:
	rm *.pdf
