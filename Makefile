DOT = dot

#all: *.gv
#	@echo "Building all"

clean:
	rm -f *.png

%.gv: $*.png

%.png:
	$(DOT) -Tpng -o $*.png $*.gv;

.PHONY: clean all %.gv
.SILENT: # get it to be less verbose
