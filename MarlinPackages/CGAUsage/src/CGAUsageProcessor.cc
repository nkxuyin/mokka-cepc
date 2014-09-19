#include "CGAUsageProcessor.h"
#include <iostream>

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>
#include "CGADefs.h"


using namespace lcio ;
using namespace marlin ;


CGAUsageProcessor aCGAUsageProcessor ;


CGAUsageProcessor::CGAUsageProcessor() : Processor("CGAUsageProcessor") {
  
  // modify processor description
  _description = "CGAUsageProcessor accesses the Mokka geometry module via the CGAProcessor" ;
  

  // register steering parameters: name, description, class-variable, default value

/*
  registerProcessorParameter( "CollectionName" , 
			      "Name of the MCParticle collection"  ,
			      _colName ,
			      std::string("MCParticle") ) ;
*/
}


void CGAUsageProcessor::init() { 

  // usually a good idea to
  printParameters() ;

  _nRun = 0 ;
  _nEvt = 0 ;
  
}

void CGAUsageProcessor::processEvent( LCEvent * evt ) { 

#define MAXSTRLEN 30
#define MAXLAYERS 1000


        double initial[3], final[3], direction[3], distance[MAXLAYERS];
        double **preSteps, nbX0[MAXLAYERS], nInterLen[MAXLAYERS];
        float energy=20.0;
        char particle[]="geantino";
        char finalVolName[MAXSTRLEN], **volName, **matName;
        int i, nbPart=1, nSteps=MAXLAYERS, OKFlag=1;
        volName=(char**)malloc(MAXLAYERS*sizeof(char*));
        matName=(char**)malloc(MAXLAYERS*sizeof(char*));
        preSteps=(double**)malloc(MAXLAYERS*sizeof(double*));
        for(i=0; i<MAXLAYERS; i++) {
                volName[i]=(char*)malloc(MAXSTRLEN*sizeof(char));
                matName[i]=(char*)malloc(MAXSTRLEN*sizeof(char));
                preSteps[i]=(double*)malloc(3*sizeof(double));
        }

        initial[0] = 0;
        initial[1] = 0;
        initial[2] = 0;
        final[0] = 0;
        final[1] = 6000;
        final[2] = 0;
        direction[0] = final[0] - initial[0];
        direction[1] = final[1] - initial[1];
        direction[2] = final[2] - initial[2];
        CGABeamOn(initial, final, direction, "geantino", 20, 1);

        CGAGetSteps(volName, matName, distance, preSteps, nbX0, nInterLen, 
		&nSteps, &OKFlag, MAXSTRLEN, MAXSTRLEN);
        if(OKFlag == -1) {
                fprintf(stderr, "ERROR GETSTEPS\n");
                exit(1);
        }

        for(i=0; i<nSteps; i++) {
                fprintf(stdout, "%s %s %f %f %f %f %f %f\n", volName[i], 
			matName[i], distance[i],
                        preSteps[i][0], preSteps[i][1], preSteps[i][2],
                        nbX0[i], nInterLen[i]);
        }

        CGAGetVolumeData("EnvLog", distance, preSteps, nbX0, nInterLen, 
		&nSteps, &OKFlag);
        if(OKFlag == -1) {
                fprintf(stderr, "ERROR GETVOLDATA\n");
                exit(1);
        }
        for(i=0; i<nSteps; i++) {
                fprintf(stdout, "EnvLog %f %f %f %f %f %f\n", distance[i],
                        preSteps[i][0], preSteps[i][1], preSteps[i][2],
                        nbX0[i], nInterLen[i]);
        }

        CGAWhereAmI(final, finalVolName, MAXSTRLEN);
        fprintf(stdout, "%s\n", finalVolName);


} 


    
void CGAUsageProcessor::processRunHeader( LCRunHeader* run) { 
}



void CGAUsageProcessor::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void CGAUsageProcessor::end(){ 
  
//   std::cout << "MyProcessor::end()  " << name() 
// 	    << " processed " << _nEvt << " events in " << _nRun << " runs "
// 	    << std::endl ;

}

