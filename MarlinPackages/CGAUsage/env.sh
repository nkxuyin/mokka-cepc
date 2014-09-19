#######################################################
#
# Example environment script to build example MyMarlin
#  
#  F. Gaede, DESY
#######################################################


# modify as needed:
export MARLIN=/home/musat/Marlin
export GEAR=/home/musat/gear
export CLHEP_BASE_DIR=/usr/local/CLHEP/2.0.2.2
export G4INSTALL=/usr/local/geant4/geant4.8.0.p01
export G4SYSTEM=Linux-g++
export MOKKA=/home/musat/Mokka
export MOKKALIBS=$HOME/$G4INSTALL/tmp/$G4SYSTEM
export MYSQLLIBPATH=/usr/lib/mysql
export LCIO=/usr/local/LCIO/01.07

# use the same env.sh that has been used to build the Marlin library

. $MARLIN/env.sh 
