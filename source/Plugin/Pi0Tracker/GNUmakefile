name := Pi0Tracker
G4TARGET := $(name)
G4EXLIB := true

.PHONY: all
all: lib 

CPPFLAGS += -I../PluginManager/include
CPPFLAGS += -I../../Kernel/include
CPPFLAGS += -I../../Geometry/CGA/include/

include ../../Kernel/GNUmakefile.common
include $(G4INSTALL)/config/binmake.gmk
