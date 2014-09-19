name := SiD
G4TARGET := $(name)
G4EXLIB := true

.PHONY: all
all: lib 

CPPFLAGS += -I../../Kernel/include -I../CGA/include -I../Tesla/include

include ../../Kernel/GNUmakefile.common
include $(G4INSTALL)/config/binmake.gmk
