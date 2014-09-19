# $Id: GNUmakefile,v 1.33 2009/03/24 11:01:40 engels Exp $

name		:= Mokka
G4TARGET	:= $(name)
G4EXLIB		:= true

ARCH = $(shell uname -s)

ifneq ($(ARCH),Darwin)
EXTRALIBS	+= -Wl,-whole-archive
endif
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/Tesla -lTesla
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/Calice -lCalice
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/SiD -lSiD
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/LDC -lLDC
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/SiLCModel -lSiLCModel
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/SiLCG4 -lSiLCG4
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/tbeam -ltbeam
EXTRALIBS       += -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/EUTelescope -lEUTelescope
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/PluginManager -lPluginManager
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/TrackingOnlyPlugin -lTrackingOnlyPlugin
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/Checkplots -lCheckplots
EXTRALIBS       += -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/MarkerPlugin -lMarkerPlugin
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/workshop -lworkshop
EXTRALIBS       += -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/Pi0Tracker -lPi0Tracker
EXTRALIBS       += -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/ShowerDecomposition -lShowerDecomposition
ifneq ($(ARCH),Darwin)
EXTRALIBS	+= -Wl,-no-whole-archive
endif
EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/CGA -lCGA

ifdef GEAR
  EXTRALIBS	+= -L$(G4WORKDIR)/tmp/$(G4SYSTEM)/MokkaGear -lMokkaGear
  CPPFLAGS 	+= -I../Geometry/MokkaGear/include
endif

CPPFLAGS	+= -I../Geometry/CGA/include
CPPFLAGS	+= -I../Plugin/PluginManager/include
CPPFLAGS	+= -I../LCPhys/include

# Include libraries which are only needed by geometry drivers
ifdef G4LIB_USE_GRANULAR
  EXTRALIBS	+= -lG4geomdivision -lG4geomBoolean
endif

# There are four ways to determine the location of the MySQL libraries:
# 1. Use MYSQL_LIBDIR, if defined
# 2. Use MYSQL_PATH, if defined (with or without trailing ".../mysql")
# 3. Use the "mysql_config" tool, if known to the shell
# 4. Use "/usr/lib/mysql" as a default

ifdef MYSQL_LIBDIR
  EXTRALIBS	+= -L$(MYSQL_LIBDIR) -lmysqlclient -lz
else
ifdef MYSQL_PATH
  EXTRALIBS	+= -L$(MYSQL_PATH)/{lib64,lib}/mysql -L$(MYSQL_PATH)/{lib64,lib} -lmysqlclient -lz
else
ifneq "$(shell /bin/sh -c type\ mysql_config 2> /dev/null)" ""
  EXTRALIBS	+= $(shell mysql_config --libs)
else
  EXTRALIBS	+= -L/usr/{lib64,lib}/mysql -lmysqlclient -lz
endif
endif
endif

ifdef LCIO
  EXTRALIBS	+= -L$(LCIO)/lib -llcio -lsio
  ifdef DCAP
    EXTRALIBS += -L$(DCAP)/lib -ldcap
  endif
endif

ifdef GEAR
  EXTRALIBS	+= -L$(GEAR)/lib -lgearxml -lgear
endif

ifdef G4ANALYSIS_USE
  EXTRALIBS	+= $(shell aida-config --lib)
endif

.PHONY: all
all: lib bin

include GNUmakefile.common
include $(G4INSTALL)/config/binmake.gmk
