##
#  Environment variables to create the Checkplots
#  in an AIDA XML file with JAIDA/AIDAJNI
#
#  Set JAIDA_HOME, AIDAJNI_HOME, and JDK_HOME
#  according to your environment
#
#  F. Gaede 08/04/2004
##

# -- JAIDA/AIDAJNI setup -- modify as needed on your system

export	JDK_HOME=/opt/products/java/1.4.2
export	JAIDA_HOME=/afs/desy.de/user/g/gaede/public/JAIDA-3.2.0
export	AIDAJNI_HOME=/afs/desy.de/user/g/gaede/public/AIDAJNI-3.2.0
source	${AIDAJNI_HOME}/bin/Linux-g++2/aidajni-setup.sh

#export	JDK_HOME=/opt/products/java/1.5.0
#export	JAIDA_HOME=/opt/products/JAIDA/3.2.3
#export	AIDAJNI_HOME=/opt/products/AIDAJNI/3.2.3
#source	${AIDAJNI_HOME}/bin/Linux-g++/aidajni-setup.sh

source	${JAIDA_HOME}/bin/aida-setup.sh

export	G4ANALYSIS_USE=1

# On some systems there might be a problem with JAIDA
# and OpenGL that causes a runtime error in the Java VM.
# In that case you can't use OpenGL with AIDA.
# -> Uncomment the following lines and relink:

#unset	G4VIS_BUILD_OPENGLX_DRIVER
#unset	G4VIS_USE_OPENGLX
