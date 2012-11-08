TEXSRC     := ../../specifications/design\ specifications/DNC_CSR/DNC_CSR.tex
TEXPARSE   := ./parse_texcsr.pl
TARGETS    := numachip-defines.h numachip-autodefs.h

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *~

numachip-autodefs.h: $(TEXSRC) $(TEXPARSE)
	$(TEXPARSE) $(TEXSRC) > $@

numachip-defines.h: numachip-define-generate.h numachip-overrides.def numachip-autodefs.h
	cpp -P $< > $@

