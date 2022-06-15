export PROJNAME := ISO9660
export RESULT := ISO9660

.PHONY: all

all: $(RESULT)
	@

%: force
	@$(MAKE) -f ../helper/Makefile $@ --no-print-directory
force: ;
