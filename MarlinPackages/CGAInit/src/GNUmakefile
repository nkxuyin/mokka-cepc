###########################################################
#
# Example GNUmakefile to build an Application with Marlin
#
# @author Frank Gaede
# @version $Id: GNUmakefile,v 1.8 2009/03/24 11:01:40 engels Exp $
#
###########################################################

# requires path to MARLIN and Marlin v00-09 or higher
# can be called from top level makefile


#----------------------------------------------------------------------------
#                user variables 
#----------------------------------------------------------------------------


# name of the program/library
PROG = CGAInit


#----  additional user include/libraries
#-- optionally can be specified in the to level GNUmakefile of $MARLIN

ARCH = $(shell uname -s)

EXTRALIBS += \
ifneq ($(ARCH),Darwin)
	-Wl,-whole-archive  \
endif
        $(MOKKALIBS)/Mokka/libMokka.a \
        $(MOKKALIBS)/Tesla/libTesla.a \
        $(MOKKALIBS)/Calice/libCalice.a \
        $(MOKKALIBS)/SiD/libSiD.a \
        $(MOKKALIBS)/LDC/libLDC.a \
        $(MOKKALIBS)/tbeam/libtbeam.a \
        $(MOKKALIBS)/PluginManager/libPluginManager.a \
        $(MOKKALIBS)/Checkplots/libCheckplots.a \
        $(MOKKALIBS)/workshop/libworkshop.a \
        $(MOKKALIBS)/LCPhys/libLCPhys.a \
ifneq ($(ARCH),Darwin)
        $(MOKKALIBS)/CGA/libCGA.a \
	-Wl,-no-whole-archive
else
        $(MOKKALIBS)/CGA/libCGA.a
endif

PLLIBSDIR = $(G4INSTALL)/lib/plists/$(G4SYSTEM)
G4LIBDIR = $(G4INSTALL)/lib/$(G4SYSTEM)

MOKKA_LISTS     := FTFC FTFP LBE
MOKKA_LISTS     += LHEP LHEP_BERT LHEP_BIC LHEP_LEAD LHEP_LEAD_HP LHEP_PRECO LHEP_PRECO_HP
                                                                                
MOKKA_LISTS     += QGSP QGSP_BERT QGSP_BIC QGSP_HP QGSC QGSC_LEAD QGSC_LEAD_HP
MOKKA_LISTS     += LHEP_BERT_HP LHEP_BIC_HP LHEP_EMV QGSP_BERT_HP QGSP_EMV
MOKKA_LISTS     += Packaging
                                                                                
EXTRALIBS       += $(foreach lib,$(MOKKA_LISTS),$(PLLIBSDIR)/lib$(lib).a)
EXTRALIBS       += -L$(G4LIBDIR) $(shell $(G4LIBDIR)/liblist -m $(G4LIBDIR) < $(G4LIBDIR)/libname.map)
EXTRALIBS       += $(CLHEP_BASE_DIR)/lib/libCLHEP.a

ifdef LCIO
  EXTRALIBS     += $(LCIO)/lib/liblcio.a $(LCIO)/sio/lib/libsio.a
endif

BASEDIR = ..
INSTALL = $(BASEDIR)/lib
BINDIR = $(BASEDIR)/bin

all:  lib $(BINDIR)/$(PROG)

EXTRALIBS     += $(MYSQLLIBPATH)/libmysqlclient.a

ifndef USERINCLUDES
 USERINCLUDES = $(CXXFLAGS) -I ../../../include -I $(MOKKA)/source/Geometry/CGA/include
endif

ifndef USERLIBS
 USERLIBS = $(EXTRALIBS) 
endif


#----------------------------------------------------------------------------
#        do not modify anything below in order to use top level makefile
#----------------------------------------------------------------------------

# the base directory of your application
# this is where you  would normaly have the lib, bin and source directories

ifdef MARLINDEBUG
 CPPFLAGS += -g -Wall -ansi
else
 CPPFLAGS += -O3 -Wall -ansi
endif


# check for private compiler definition
ifdef MY_CXX
  CXX = $(MY_CXX)
else
  CXX = g++
endif

AR = ar

#--------------------------------------------------------------------------------------
ifneq ($(ARCH),Darwin)
LIBS +=   -Wl,-whole-archive  -L $(INSTALL)  -l$(PROG) -Wl,-no-whole-archive
else
LIBS +=   -L $(INSTALL)  -l$(PROG)
endif
#--------------------------------------------------------------------------------------

INCLUDE +=  -I $(BASEDIR)/include
INCLUDE += $(USERINCLUDES)
CPPFLAGS += $(INCLUDE)
CPPFLAGS += `. $(MARLIN)/bin/marlin_includes.sh`

LIBS += $(USERLIBS)
LIBS += `. $(MARLIN)/bin/marlin_libs.sh`


$(BINDIR)/$(PROG): $(INSTALL)/lib$(PROG).a
	mkdir -p $(BINDIR)
	$(CXX) -o $(BINDIR)/$(PROG) $(CPPFLAGS) $(MARLIN)/src/Marlin.cc $(LIBS) 

objects = $(patsubst %.cc,%.o, $(wildcard *.cc)  )

$(INSTALL)/lib$(PROG).a: $(objects)
	mkdir -p $(INSTALL)
	$(AR) cr $(INSTALL)/lib$(PROG).a  $? 

lib: $(INSTALL)/lib$(PROG).a


clean:
	rm -f *.o $(INSTALL)/lib$(PROG).a $(BINDIR)/$(PROG)

doc:
	cd $(BASEDIR)/doc ; doxygen ;

