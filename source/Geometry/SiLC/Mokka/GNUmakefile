name := SiLCG4
G4TARGET := $(name)
G4EXLIB := true

.PHONY: all
all: lib 

CPPFLAGS += -I../Model/include -I../../../Kernel/include -I../../CGA/include -I../../Tesla/include

ifdef GEAR
CPPFLAGS += -I../../MokkaGear/include
endif 

include ../../../Kernel/GNUmakefile.common
include $(G4INSTALL)/config/binmake.gmk

