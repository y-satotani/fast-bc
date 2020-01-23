.PHONY: all clean

pdf := $(patsubst %.svg,%.pdf,$(shell ls | egrep "\.svg$$"))
png := $(patsubst %.svg,%.png,$(shell ls | egrep "\.svg$$"))

all: $(pdf) $(png)

%.pdf: %.svg
	inkscape -z -o $@ --export-latex $<

%.png: %.svg
	inkscape -z -o $@ -d 150 $<

clean:
	rm *.pdf *.pdf_tex *.png
