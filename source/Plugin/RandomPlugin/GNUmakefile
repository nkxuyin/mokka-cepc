name := RandomPlugin
G4TARGET := $(name)
G4EXLIB := true

.PHONY: all
all: lib 

CPPFLAGS += -I../PluginManager/include

include ../../Kernel/GNUmakefile.common
include $(G4INSTALL)/config/binmake.gmk
