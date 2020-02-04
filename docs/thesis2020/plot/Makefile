pdf := $(patsubst %.R,%.pdf,$(shell ls | grep R))
png := $(patsubst %.R,%.png,$(shell ls | grep R))

.PHONY: all clean

all: $(pdf) $(png)

%.pdf: %.R
	Rscript $<
%.png: %.pdf
	inkscape -z -o $@ -d 300 $<
clean:
	rm *.pdf *.png
