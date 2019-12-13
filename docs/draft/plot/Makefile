pdf := $(patsubst %.R,%.pdf,$(shell ls | grep R))
eps := $(patsubst %.R,%.eps,$(shell ls | grep R))

.PHONY: all clean

all: $(pdf) $(eps)

%.pdf: %.R
	Rscript $<
%.eps: %.pdf
	inkscape -z -o $@ -d 300 $<
clean:
	rm *.pdf *.eps
