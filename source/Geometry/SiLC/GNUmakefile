.PHONY: all doc clean clean_libs

all:
	@$(MAKE) -C Model
	@$(MAKE) -C Mokka

doc:
	@doxygen config.doxygen

clean:
	@$(MAKE) -C Model clean
	@$(MAKE) -C Mokka clean

clean_libs:
	@$(MAKE) -C Model clean_libs
	@$(MAKE) -C Mokka clean_libs
