#include "CGAInitProcessor.h"
#include <iostream>

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>
#include "CGADefs.h"


using namespace lcio ;
using namespace marlin ;


CGAProcessor aCGAProcessor ;


CGAProcessor::CGAProcessor() : Processor("CGAProcessor") {
  
  // modify processor description
  _description = "CGAProcessor supplies access to the Mokka geometry module via the CGA interface" ;
  

  // register steering parameters: name, description, class-variable, default value

/*
  registerProcessorParameter( "CollectionName" , 
			      "Name of the MCParticle collection"  ,
			      _colName ,
			      std::string("MCParticle") ) ;
*/
}


void CGAProcessor::init() { 

  // usually a good idea to
  printParameters() ;

  _nRun = 0 ;
  _nEvt = 0 ;
  
}

void CGAProcessor::processRunHeader( LCRunHeader* run) { 

  CGAInit((run->getDetectorName()).c_str(), "", "", "", "");
  _nRun++ ;
} 

void CGAProcessor::processEvent( LCEvent * evt ) { 

    
  // this gets called for every event 
  // usually the working horse ...

}



void CGAProcessor::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void CGAProcessor::end(){ 
  
//   std::cout << "MyProcessor::end()  " << name() 
// 	    << " processed " << _nEvt << " events in " << _nRun << " runs "
// 	    << std::endl ;

}

