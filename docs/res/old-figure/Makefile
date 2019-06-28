.PHONY: all clean

all: $(patsubst %.svg,%.pdf,$(shell ls | egrep "\.svg$$"))

%.pdf: %.svg
	inkscape -z -f $< -A $@ --export-latex

%.png: %.svg
	inkscape -z -f $< -e $@ -d 100

clean:
	rm *.pdf *.pdf_tex
