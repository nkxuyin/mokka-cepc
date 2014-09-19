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
## $Id: GNUmakefile,v 1.22 2009/03/24 11:01:40 engels Exp $
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

# Please edit the following four lines
# LIBSTART is -lchill for gcc 2.95.2 
# and is -lfrtbegin for gcc 3.2
LIBSTART = -lfrtbegin
#on a 64 bit machine it can be:
#LIBSTART = -L/usr/lib/gcc/x86_64-redhat-linux/4.1.1/ -lgfortranbegin -lgfortran


LIBPACKNAME := libCGAPack.so
TARGETDIR := $(G4WORKDIR)/lib/$(G4SYSTEM)
LIBPACK := $(TARGETDIR)/$(LIBPACKNAME)

TGTS := $(LIBPACK) Ex01 Ex02 Ex03.class 
ifdef LCIO
TGTS += Ex04 Ex05.class 
endif
TGTS += Ex06
ifdef LCIO
TGTS += Ex07
endif

EXLIBS := -lstdc++ $(LIBSTART) -lm

ifdef GEAR 
GEARFLAGS := -DMOKKA_GEAR -I ../../source/Geometry/MokkaGear/include \
		-I$(GEAR)/include
endif

ifdef LCIO
CPPFLAGS      += -DLCIO_MODE -I$(LCIO)/include
endif

CGAFLAGS := -I ../../source/Geometry/CGA/include

.PHONY: all clean_local
all: $(TGTS)

$(LIBPACK): 
	@$(MAKE) -C ../../source/Geometry/CGA -f GNUmakefile.libpack

Ex01:   $(LIBPACK) Ex01.o 
	@echo Linking $@ ...
	@$(CXX) -o $@ $^ $(EXLIBS)

Ex01.o: Ex01.c
	@echo Compiling $< ...
	@$(CXX) $(CPPFLAGS) $(CGAFLAGS) -c $<

Ex02:	$(LIBPACK) Ex02.o 
	@echo Linking $@ ...
	@g77 -o $@ $^ $(EXLIBS)

Ex02.o: Ex02.f
	@echo Compiling $< ...
	@g77 -c $<

Ex03.class: Ex03.java
	@echo Compiling $< ...
	@javac -classpath .:$(TARGETDIR)/CGAJava.jar Ex03.java

ifdef LCIO
Ex04:   $(LIBPACK) Ex04.o 
	@echo Linking $@ ...
	@$(CXX) -o $@ $^ $(EXLIBS)

Ex04.o: Ex04.cc
	@echo Compiling $< ...
	@$(CXX) $(CPPFLAGS) $(CGAFLAGS) -c $<

Ex05.class: Ex05.java
	@echo Compiling $< ...
	@javac -classpath .:$(TARGETDIR)/CGAJava.jar:$(LCIO)/lib/lcio.jar:$(LCIO)/tools/sio.jar Ex05.java

Ex07:   $(LIBPACK) Ex07.o 
	@echo Linking $@ ...
	@source  $(TARGETDIR)/env.sh ; $(CXX) -o $@ $^ $(EXLIBS)

Ex07.o: Ex07.cc
	@echo Compiling $< ...
	@$(CXX) $(CPPFLAGS) $(GEARFLAGS) -c $<

else
Ex04: 
	@echo "You have to install LCIO and compile Mokka with it"
	
Ex04.o: 
	@echo "You have to install LCIO and compile Mokka with it"
	
Ex05.class: 
	@echo "You have to install LCIO and compile Mokka with it"
	

Ex07:
	@echo "You have to install LCIO and compile Mokka with it"

Ex07.o:
	@echo "You have to install LCIO and compile Mokka with it"


endif

Ex06:   $(LIBPACK) Ex06.o 
	@echo Linking $@ ...
	@g77 -o $@ $^ $(EXLIBS)

Ex06.o: Ex06.f
	@echo Compiling $< ...
	@g77 -c $<

run-ex03: $(TARGETDIR)/libCGAJava.so Ex03.class
	@java -cp .:$(TARGETDIR)/CGAJava.jar:$(LCIO)/lib/lcio.jar:$(LCIO)/tools/freehep-sio-2.1.jar:$(LCIO)/tools/freehep-xdr-2.0.4.jar:. Ex03 

ifdef LCIO
run-ex05: $(TARGETDIR)/libCGAJava.so Ex05.class
	@java -cp .:$(TARGETDIR)/CGAJava.jar:$(LCIO)/lib/lcio.jar:$(LCIO)/tools/freehep-sio-2.1.jar:$(LCIO)/tools/freehep-xdr-2.0.4.jar:. Ex05 ./recjob.slcio
else
run-ex05:
	@echo "You have to install LCIO and compile Mokka with it"
endif

.PHONY: clean

clean_local:
	rm -f *.o $(TGTS) $(TARGETDIR)/env.sh


