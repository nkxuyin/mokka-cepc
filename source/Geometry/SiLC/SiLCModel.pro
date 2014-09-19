# -------------------------------------------------
# QtCreator Project File. Silc.
# -------------------------------------------------
QT -= core \
    gui
TARGET = SiLC
TEMPLATE = lib
DEFINES += SILC_LIBRARY
INCLUDEPATH += Model/include/ \
    $(GEAR)/include
HEADERS += Model/include/XY_Endcap.hh \
    Model/include/XUV_Endcap.hh \
    Model/include/stl_extensions.hh \
    Model/include/Silc_Globals.hh \
    Model/include/Sensors.hh \
    Model/include/phys_primitives.hh \
    Model/include/Module.hh \
    Model/include/MaterialObject.hh \
    Model/include/GlobalNodeId.hh \
    Model/include/GearStore.hh \
    Model/include/Exception.hh \
    Model/include/EndcapSerializer.hh \
    Model/include/EndcapArray.hh \
    Model/include/Endcap.hh \
    Model/include/BarrelSingleLayer.hh \
    Model/include/BarrelSerializer.hh \
    Model/include/BarrelDoubleLayer.hh \
    Model/include/BarrelArray.hh \
    Model/include/Barrel.hh \
    Model/include/SiliconSubDetector.hh \
    Model/include/SubDetectorArray.hh
SOURCES += Model/src/XY_Endcap.cc \
    Model/src/XUV_Endcap.cc \
    Model/src/stl_extensions.cc \
    Model/src/Sensors.cc \
    Model/src/phys_primitives.cc \
    Model/src/Module.cc \
    Model/src/MaterialObject.cc \
    Model/src/Exception.cc \
    Model/src/EndcapSerializer.cc \
    Model/src/EndcapArray.cc \
    Model/src/Endcap.cc \
    Model/src/BarrelSingleLayer.cc \
    Model/src/BarrelSerializer.cc \
    Model/src/BarrelDoubleLayer.cc \
    Model/src/BarrelArray.cc \
    Model/src/Barrel.cc \
    Model/src/SiliconSubDetector.cc \
    Model/src/GlobalNodeId.cc
