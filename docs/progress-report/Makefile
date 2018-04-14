SRCS=$(wildcard *.Rmd)
PDFS=$(SRCS:.Rmd=.pdf)

all: $(PDFS)

%.pdf: %.Rmd
	R --vanilla -e 'rmarkdown::render("$<", output_format = "pdf_document")'
