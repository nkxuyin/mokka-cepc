/*
      *******************************************************
      *                                                     *
      *                      Mokka                          * 
      *   - the detailed geant4 simulation for Tesla -      *
      *                                                     *
      * For more information about Mokka, please, go to the *
      *                                                     *
      *  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
      *                                                     *
      *    Mokka home page.                                 *
      *                                                     *
      *******************************************************
      
       $Id: Ex01.c,v 1.10 2006/05/23 11:42:08 musat Exp $
       $Name: mokka-07-00 $
      
       History
       first implementation for the 
       Mokka Common Geometry Access (CGA) by 
       Gabriel Musat (musat@poly.in2p3.fr), March 2003
      
       see CGA documentation at 
       http://polype.in2p3.fr/geant4/tesla/www/mokka/
              software/doc/CGADoc/CGAIndex.html
      -------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CGADefs.h"
#include <string>
#include <vector>

#define MAXSTRLEN 30
#define MAXLAYERS 1000


int main(void) {
	double initial[3], final[3], direction[3], distance[MAXLAYERS];
	double **preSteps, nbX0[MAXLAYERS], nInterLen[MAXLAYERS];
	float energy=20.0;
	char particle[]="geantino";
	char enveloppe[]="EnvLog";
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
	initial[0] = -68;
	initial[1] = 169;
	initial[2] = 0;
	final[0] = -200*sin(3.1418/8);
	final[1] = 200*cos(3.1418/8);
	final[2] = 0;
	direction[0] = final[0] - initial[0];
	direction[1] = final[1] - initial[1];
	direction[2] = final[2] - initial[2];
	CGAInit("", "D09M1", "", "", "", "");
	CGABeamOn(initial, final, direction, particle, energy, nbPart);

	CGAGetSteps(volName, matName, distance, preSteps, nbX0, nInterLen,
		&nSteps, &OKFlag, MAXSTRLEN, MAXSTRLEN);
	if(OKFlag == -1) {
		fprintf(stderr, "ERROR GETSTEPS\n");
		exit(1);
	}
	for(i=0; i<nSteps; i++) {
		fprintf(stdout, "%s %s %f %f %f %f %f %f\n", volName[i], 
		matName[i], distance[i], preSteps[i][0], preSteps[i][1], 
		preSteps[i][2], nbX0[i], nInterLen[i]);
	}

	CGAGetVolumeData(enveloppe, distance, preSteps, nbX0, nInterLen, 
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

	initial[0]=initial[1]=initial[2]=0;
	final[0]=1000; final[1]=1000; final[2]=1000;
	double IEdl = CGAGetEdl(initial, final);
	fprintf(stdout, "IEdl = %f\n", IEdl);

	double IBdl = CGAGetBdl(initial, final);
	fprintf(stdout, "IBdl = %f\n", IBdl);

	std::vector<double> B = CGAGetB(initial);
	fprintf(stdout, "B in origin = (%f %f %f)\n", B[0], B[1], B[2]);

	std::vector<double> E = CGAGetE(initial);
	fprintf(stdout, "E in origin = (%f %f %f)\n", E[0], E[1], E[2]);

	final[0] = 0; final[1] = 1730; final[2] = 0;
	fprintf(stdout, "Material: %s, Density: %f g/cm3, Pressure: %f bar, Temperature: %f, RadLen: %f, IntLen: %f\n",
		(CGAGetMaterialName(final)).c_str(), CGAGetDensity(final),
		CGAGetPressure(final), CGAGetTemperature(final),
		CGAGetRadLen(final), CGAGetIntLen(final));

	std::vector<std::string> listOfLVs = CGAGetListOfLogicalVolumes(final);
	
	for(unsigned int i=0; i < listOfLVs.size(); i++)
		fprintf(stdout, "%s\n", listOfLVs[i].data());

	std::vector<std::string> listOfPVs = CGAGetListOfPhysicalVolumes(final);
	
	for(unsigned int i=0; i < listOfPVs.size(); i++)
		fprintf(stdout, "%s\n", listOfPVs[i].data());

	fprintf(stdout, "%s\n", (CGAGetRegionName(final)).data());

        std::vector<double> localPos(3); 
	localPos[0]=localPos[1]=localPos[2]=0.0;

	localPos = CGAGetLocalPosition(final);

	fprintf(stdout, "LocalPosition: (%f,%f,%f)\n", localPos[0],
		localPos[1], localPos[2]);
	
	initial[0]=initial[1]=initial[2]=0;
	bool isTracker = CGAIsTracker(initial);
	if(isTracker)
		fprintf(stdout, "position: (%f,%f,%f) is in tracker region\n",
			initial[0], initial[1], initial[2]);

	final[0] = 0; final[1] = 1730; final[2] = 0;
	bool isCalorimeter = CGAIsCalorimeter(final);
	if(isCalorimeter)
           fprintf(stdout, "position: (%f,%f,%f) is in calorimeter region\n",
			final[0], final[1], final[2]);

        return 0;
}

