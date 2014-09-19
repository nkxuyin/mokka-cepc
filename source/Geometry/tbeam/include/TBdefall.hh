/* For some odd reason, the classes are not being instantiated from .cc.
   
   As an ugly hack, this file is included in Proto00.cc to instantiate 
   everything for TB.

   This should eventually be changed to INSTANTIATE in each .cc file
   just like other SubD drivers.
*/

#ifndef TBdefall_h
#define TBdefall_h 1

#include "CGADefs.h"

#include "TBhcal00.hh"
INSTANTIATE (TBhcal00)

#include "TBecal00.hh"
INSTANTIATE (TBecal00)

#include "TBcatcher00.hh"
INSTANTIATE (TBcatcher00)

#endif
