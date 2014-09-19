#!/bin/ksh
rm -f zqq_newformat.slcio
EVTNUMBER=`awk 'NF==1 {print}' zqq_newformat.HEPEvt | wc -l`
export EVTNUMBER
echo The zqq_newformat.HEPEvt file has $EVTNUMBER events!
$G4WORKDIR/bin/$G4SYSTEM/Mokka test1.str <<!
#/vis/open OGLIX 1000
#/vis/viewer/viewpointVector 1 0 0
#/vis/drawVolume
#/vis/viewer/zoomTo 4
#/event/Draw 1
#/step/draw 1
/generator/generator zqq_newformat.HEPEvt
/run/beamOn $EVTNUMBER
exit
!
rm -f zqq_newformat.txt zqq_newformat.txt_ref
$LCIO/bin/dumpevent zqq_newformat.slcio 0 0 > zqq_newformat.txt
$LCIO/bin/dumpevent zqq_newformat.slcio_ref  0 0 > zqq_newformat.txt_ref
diff --brief zqq_newformat.txt zqq_newformat.txt_ref
