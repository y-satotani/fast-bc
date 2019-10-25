.PHONY: all clean

pdf := $(patsubst %.svg,%.pdf,$(shell ls | egrep "\.svg$$"))
eps := $(patsubst %.svg,%.eps,$(shell ls | egrep "\.svg$$"))

all: $(pdf) $(eps)

%.pdf: %.svg
	inkscape -z -f $< -A $@ --export-latex

%.eps: %.svg
	inkscape -z -f $< -E $@ -d 300

clean:
	rm *.pdf *.pdf_tex *.eps
