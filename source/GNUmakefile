SUBDIR := $(shell ls -d Geometry/* | sed /CVS/d)
SUBDIR += $(shell ls -d Plugin/*   | sed /CVS/d)
SUBDIR += Kernel

.PHONY: all clean clean_libs

all: 
	@for dir in $(SUBDIR); do $(MAKE) -C $$dir; done
ifdef MOKKA_PACK_LIBS
	@$(MAKE) -C ./Geometry/CGA -f GNUmakefile.libpack
endif

clean:
	@for dir in $(SUBDIR); do $(MAKE) -C $$dir clean; done

clean_libs:
	@for dir in $(SUBDIR); do $(MAKE) -C $$dir clean_libs; done
