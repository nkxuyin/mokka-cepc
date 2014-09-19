#! /bin/bash

rm -f two_electrons.slcio

 Mokka -M CaliceEcal03 -l two_electrons << !
/generator/generator particleGun
/gun/position 0 0 -25
/gun/direction 0 0 1
/gun/energy 50 GeV
/gun/particle e-
/run/beamOn 1
/run/beamOn 1
exit
!

diff two_electrons.slcio two_electrons.slcio_ref
