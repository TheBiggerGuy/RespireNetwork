
all: report

report: images
	mkdir --parents out
	cp references.bib out/references.bib
	latex -output-directory=out -output-format=pdf main.tex
	cd out;bibtex main.aux
	latex -output-directory=out -output-format=pdf main.tex
	latex -output-directory=out -output-format=pdf main.tex

images: images/respire_family_history.png images/modules_dep.png

images/respire_family_history.png:
	dot -Tpng -o images/respire_family_history.png respire_family_history.dot

images/modules_dep.png:
	dot -Tpng -o images/modules_dep.png modules.dot

clean:
	rm -rf out
