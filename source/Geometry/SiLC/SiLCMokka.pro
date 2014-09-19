# -------------------------------------------------
# QtCreator Project File. Silc.
# -------------------------------------------------
QT -= core \
    gui
TARGET = SiLC
TEMPLATE = lib
DEFINES += SILC_LIBRARY
INCLUDEPATH += Model/include/ \
    Mokka/include \
    $(GEAR)/include \
    $(CLHEP_INCLUDE_DIR) \
    $(G4INCLUDE) \
    $(MOKKA)/source/Geometry/CGA/include \
    $(MOKKA)/source/Geometry/MokkaGear/include \
    $(MOKKA)/source/Kernel/include
SOURCES += Mokka/src/XY_G4Endcap.cc \
    Mokka/src/XUV_G4Endcap.cc \
    Mokka/src/SiLCSD.cc \
    Mokka/src/G4Module.cc \
    Mokka/src/G4Globals.cc \
    Mokka/src/G4EndcapArray.cc \
    Mokka/src/G4Endcap.cc \
    Mokka/src/G4BarrelSingleLayer.cc \
    Mokka/src/G4BarrelDoubleLayer.cc \
    Mokka/src/G4BarrelArray.cc \
    Mokka/src/G4Barrel.cc \
    Mokka/src/G4SiliconSubDetector.cc \
    Mokka/src/SiSubDetectorDriver.cc
HEADERS += Mokka/include/XY_G4Endcap.hh \
    Mokka/include/XUV_G4Endcap.hh \
    Mokka/include/SiLCSD.hh \
    Mokka/include/G4Module.hh \
    Mokka/include/G4Globals.hh \
    Mokka/include/G4EndcapArray.hh \
    Mokka/include/G4Endcap.hh \
    Mokka/include/G4BarrelSingleLayer.hh \
    Mokka/include/G4BarrelDoubleLayer.hh \
    Mokka/include/G4BarrelArray.hh \
    Mokka/include/G4Barrel.hh \
    Mokka/include/MokkaStore.hh \
    Mokka/include/G4SiliconSubDetector.hh \
    Mokka/include/SiSubDetectorDriver.hh
