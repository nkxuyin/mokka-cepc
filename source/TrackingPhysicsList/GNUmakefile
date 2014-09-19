name := TrackingPhysicsList
G4TARGET := $(name)
G4EXLIB := true

.PHONY: all
all: lib 

CPPFLAGS += -I../Kernel/include -I../Geometry/CGA/include

ifdef G4ANALYSIS_USE
  CPPFLAGS += $(shell aida-config --include)
endif

include ../Kernel/GNUmakefile.common
include $(G4INSTALL)/config/binmake.gmk
