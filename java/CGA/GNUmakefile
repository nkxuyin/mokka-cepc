##
##*******************************************************
##*                                                     *
##*                      Mokka                          * 
##*   - the detailed geant4 simulation for Tesla -      *
##*                                                     *
##* For more information about Mokka, please, go to the *
##*                                                     *
##*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
##*                                                     *
##*    Mokka home page.                                 *
##*                                                     *
##*******************************************************
##
## $Id: GNUmakefile,v 1.20 2009/03/24 11:01:40 engels Exp $
## $Name: mokka-07-00 $
##
## History
## first implementation for the 
## Mokka Common Geometry Access (CGA) by 
## Gabriel Musat (musat@poly.in2p3.fr), March 2003
##
## see CGA documentation at 
## http://polype.in2p3.fr/geant4/tesla/www/mokka/
##        software/doc/CGADoc/CGAIndex.html
##-------------------------------------------------------

# Please edit the following lines
# and set JAVAPATH 
# Also, don't forget to set the java CLASSPATH
JAVAPATH=/opt/exp_soft/ilc/LOCAL_LLR/auxsw/sl4/java/jdk1.6.0_10

name := CGAJava

EXLIBS := -lstdc++ -lm 

CPPFLAGS := \
	-I$(JAVAPATH)/include \
	-I$(JAVAPATH)/include/linux \
	-I ../../source/Geometry/CGA/include \
	-I ../../source/Kernel/include

include ../../source/Kernel/GNUmakefile.common

.PHONY: all clean_local

all: $(LIBPACK) $(TARGETDIR)/libCGAJava.so $(TARGETDIR)/CGAJava.jar
include $(G4INSTALL)/config/binmake.gmk

G4LIBDIR := $(G4TMP)/$(G4SYSTEM)/$(name)

$(LIBPACK):
	@$(MAKE) -C ../../source/Geometry/CGA -f GNUmakefile.libpack


$(TARGETDIR)/libCGAJava.so: $(G4LIBDIR)/CGARunManager.o
	@echo Building libCGAJava.so ...
	$(shell [ ! -d $(TARGETDIR) ] && mkdir -p $(TARGETDIR) )
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) \
	-shared -o $@ $^ $(LDFLAGS) \
	$(LIBPACK)

$(G4LIBDIR)/CGARunManager.o: src/CGARunManager.cc
	$(shell [ ! -d $(G4LIBDIR) ] && mkdir -p $(G4LIBDIR) )
	@echo Compiling $< ...
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(APPFLAGS) \
	-c $< -o $@

$(TARGETDIR)/CGAJava.jar: $(TARGETDIR)/CGARunManager.class $(TARGETDIR)/Step.class $(TARGETDIR)/CellIdUtility.class
	@echo Building CGAJava.jar ...
	$(shell [ ! -d $(TARGETDIR) ] && mkdir -p $(TARGETDIR) )
	@cd $(TARGETDIR); jar -cf $@ CGARunManager.class Step.class CellIdUtility.class

$(TARGETDIR)/CGARunManager.class: src/CGARunManager.java
	$(shell [ ! -d $(TARGETDIR) ] && mkdir -p $(TARGETDIR) )
	@echo Compiling $< ...
	@javac -classpath src $< -d $(TARGETDIR)

$(TARGETDIR)/Step.class: src/Step.java
	$(shell [ ! -d $(TARGETDIR) ] && mkdir -p $(TARGETDIR) )
	@echo Compiling $< ...
	@javac $< -d $(TARGETDIR)


$(TARGETDIR)/CellIdUtility.class: src/CellIdUtility.java
	$(shell [ ! -d $(TARGETDIR) ] && mkdir -p $(TARGETDIR) )
	@echo Compiling $< ...
	@javac $< -d $(TARGETDIR)


clean_local:
	@rm -f $(G4LIBDIR)/*.o $(G4LIBDIR)/*.a $(TARGETDIR)/*.class \
		$(LIBPACK) $(TARGETDIR)/libCGAJava.so \
		$(TARGETDIR)/CGAJava.jar $(TARGETDIR)/env.sh


