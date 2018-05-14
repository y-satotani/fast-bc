SRCS=$(wildcard *.Rmd)
PDFS=$(SRCS:.Rmd=.pdf)
MDS=$(SRCS:.Rmd=.md)

.PHONY: all clean

all: $(PDFS) $(MDS)

%.pdf: %.Rmd
	R --vanilla -e 'rmarkdown::render("$<", output_format = "pdf_document")'

%.md: %.Rmd
	R --vanilla -e 'rmarkdown::render("$<", output_format = "github_document")'

clean:
	rm -rf *.pdf *.md *.html *_files
