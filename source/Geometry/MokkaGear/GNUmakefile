
ifdef GEAR

name := MokkaGear
G4TARGET := $(name)
G4EXLIB := true

.PHONY: all
all: lib 

CPPFLAGS += -I$(GEAR)/src/cpp/include
CPPFLAGS += -I../../Kernel/include
CPPFLAGS += -I../../Geometry/CGA/include


include ../../Kernel/GNUmakefile.common
include $(G4INSTALL)/config/binmake.gmk


else

# nothing to do here if we don't build with GEAR ...

.PHONY: all
all: lib 

lib:	

endif
