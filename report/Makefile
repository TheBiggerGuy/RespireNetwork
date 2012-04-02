
all: report

report:
	mkdir --parents out
	cp references.bib out/references.bib
	latex -output-directory=out -output-format=pdf main.tex
	cd out;bibtex main.aux
	latex -output-directory=out -output-format=pdf main.tex

clean:
	rm -rf out
