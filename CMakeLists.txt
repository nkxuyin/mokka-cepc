######################################################################
#
#  CMake build file for Mokka
#  --------------------------
#
# (requires geant4 9.5 or higher  - currently only available as beta) 
#
#  status: experimental 
# 
#  F.Gaede, DESY, 2011
######################################################################

PROJECT(Mokka)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4)

# project version
SET( ${PROJECT_NAME}_VERSION_MAJOR 8 )
SET( ${PROJECT_NAME}_VERSION_MINOR 3 )
SET( ${PROJECT_NAME}_VERSION_PATCH 0 )


FIND_PACKAGE( ILCSOFT_CMAKE_MODULES  REQUIRED )
#FIND_PACKAGE( ILCUTIL COMPONENTS ILCSOFT_CMAKE_MODULES streamlog ILCTEST REQUIRED )

# load default settings from ILCSOFT_CMAKE_MODULES
INCLUDE( ilcsoft_default_settings )


# require proper c++

ADD_DEFINITIONS( "-Wall -ansi -pedantic" )
ADD_DEFINITIONS( "-Wno-long-long" )


#---------------------------------------------------------------------------
# define a macro for adding all headers and cc files in a given subdirectory to a variable
macro(MOKKA_COLLATE_APPLICATION_SOURCES source_dest_var source_dir )

    file( GLOB_RECURSE  local_source  ${source_dir}/src/*.cc )

    LIST( APPEND  ${source_dest_var} ${local_source} )

    include_directories(${source_dir}/include)
endmacro()
#---------------------------------------------------------------------------


###########################################################################################################
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Kernel )

MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/TrackingPhysicsList )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/PluginManager )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/CGA	     )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/Calice	     )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/EUTelescope )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/LDC	     )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/MokkaGear   )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/SiD	     )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/SiLC/Model  )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/SiLC/Mokka  )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/Tesla	     )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/Tmag	     )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/tbeam	     )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Geometry/workshop    )

MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/Checkplots         )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/JDoePlugin	  )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/LogPlugin	  )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/MagPlugin	  )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/MarkerPlugin	  )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/MaterialPlugin	  )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/OverlapPlugin	  )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/PluginManager	  )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/RandomPlugin	  )
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/TrackingOnlyPlugin )

MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/Pi0Tracker ) 
MOKKA_COLLATE_APPLICATION_SOURCES( library_sources ${PROJECT_SOURCE_DIR}/source/Plugin/ShowerDecomposition )


#############################################################################################################################

# --- required external packages

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
option(WITH_GEANT4_TCSH_UI_ONLY "Build example with TCSH UI only" ON)
if(WITH_GEANT4_UIVIS)
  # for all possible options check file: Geant4Config.cmake
  if(WITH_GEANT4_TCSH_UI_ONLY)
    find_package(Geant4 REQUIRED gdml ui_tcsh vis_opengl_x11)
  else()
    find_package(Geant4 REQUIRED gdml ui_all vis_all)
  endif()
else()
  find_package(Geant4 REQUIRED gdml)
endif()

INCLUDE(${Geant4_USE_FILE})   # this also takes care of geant 4 definitions and include dirs

FIND_PACKAGE( LCIO REQUIRED )
FIND_PACKAGE( GEAR REQUIRED )

FIND_PACKAGE( MySQL REQUIRED )


#### some Mokka specific flags (should be made cmake parameters !?) ######
ADD_DEFINITIONS("-DMOKKA_GEAR" )
ADD_DEFINITIONS("-DLCIO_MODE" )
ADD_DEFINITIONS("-DG4LIB_USE_GDML")


INCLUDE_DIRECTORIES( ${LCIO_INCLUDE_DIRS} ${GEAR_INCLUDE_DIRS} ${MySQL_INCLUDE_DIRS} )
LINK_LIBRARIES(   ${Geant4_LIBRARIES} ${LCIO_LIBRARIES} ${GEAR_LIBRARIES} ${MySQL_LIBRARIES} )

#############################################################################################################################


#MESSAGE( STATUS " *************  library_sources :    ${library_sources} "  )


## create library
ADD_SHARED_LIBRARY( Mokka ${library_sources} )
INSTALL_SHARED_LIBRARY( Mokka DESTINATION lib )
TARGET_LINK_LIBRARIES( Mokka ${CMAKE_DL_LIBS} )




# create a binary:
# ----- Mokka executable ----------------------------------------------------
ADD_EXECUTABLE( bin_Mokka  ${PROJECT_SOURCE_DIR}/source/Kernel/Mokka.cc )

# avoid name conflict between Mokka library and Mokka application
SET_TARGET_PROPERTIES( bin_Mokka PROPERTIES OUTPUT_NAME Mokka )
TARGET_LINK_LIBRARIES( bin_Mokka Mokka )
INSTALL( TARGETS bin_Mokka DESTINATION bin )
# ----------------------------------------------------------------------------

