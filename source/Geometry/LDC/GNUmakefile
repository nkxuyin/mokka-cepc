name := LDC
G4TARGET := $(name)
G4EXLIB := true

.PHONY: all
all: lib 

CPPFLAGS += -I../../Kernel/include -I../CGA/include  -I../Tesla/include

ifdef GEAR
CPPFLAGS += -I../MokkaGear/include
endif

#CPPFLAGS += -DONE_DISK=3 -DDEBUG_POSITIVE

include ../../Kernel/GNUmakefile.common
include $(G4INSTALL)/config/binmake.gmk
