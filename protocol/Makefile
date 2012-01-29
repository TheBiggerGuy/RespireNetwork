DOT = dot

GV_FILES := $(wildcard *.gv)

all: $(patsubst %.gv,%.png,$(GV_FILES))

clean:
	@rm -f *.png

%.png: %.gv
	@echo "Making PNG for $*.gv"
	@$(DOT) -Tpng -o $@ $<

%.pdf: %.gv
	@echo "Making PNG for $*.gv"
	@$(DOT) -Tpdf -o $@ $<

.PHONY: clean all
