TEXSRC   = ../../specifications/design\ specifications/DNC_CSR/DNC_CSR.tex
TEXPARSE = ./parse_texcsr.pl
MCODEPARSE = ./parse_mcode.pl
UCODEDIR = ../../behave/scc/microcode
TARGETS  = numachip-defines.h numachip-autodefs.h \
	numachip-mseq-ucode.h numachip-mseq-table.h

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *~


numachip-autodefs.h: $(TEXSRC) $(TEXPARSE)
	$(TEXPARSE) $(TEXSRC) > $@

numachip-defines.h: numachip-define-generate.h numachip-overrides.def numachip-autodefs.h
	cpp -P $< > $@

numachip-mseq-ucode.h: $(UCODEDIR)/mseq.code $(MCODEPARSE)
	$(MCODEPARSE) "u32 numachip_mseq_ucode[] = " < $< > $@

numachip-mseq-table.h: $(UCODEDIR)/mseq.table $(MCODEPARSE)
	$(MCODEPARSE) "u16 numachip_mseq_table[] = " < $< > $@

