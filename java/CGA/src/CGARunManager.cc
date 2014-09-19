//
////*******************************************************
////*                                                     *
////*                      Mokka                          * 
////*   - the detailed geant4 simulation for Tesla -      *
////*                                                     *
////* For more information about Mokka, please, go to the *
////*                                                     *
////*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
////*                                                     *
////*    Mokka home page.                                 *
////*                                                     *
////*******************************************************
////
//// $Id: CGARunManager.cc,v 1.10 2006/05/23 11:42:08 musat Exp $
//// $Name: mokka-07-00 $
////
//// History
//// first Java implementation for the 
//// Mokka Common Geometry Access (CGA) by 
//// Gabriel Musat (musat@poly.in2p3.fr), March 2003
////
//// see CGA documentation at 
//// http://polype.in2p3.fr/geant4/tesla/www/mokka/
////        software/doc/CGADoc/CGAIndex.html
////-------------------------------------------------------

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include "CGARunManager.h"
#include "CGADefs.h"
#include "Control.hh"

using namespace std;

JNIEXPORT void JNICALL Java_CGARunManager_init
  (JNIEnv *env , jobject, jstring steer, jstring model, jstring setup, jstring host, jstring user, jstring passwd) {

	char *Steer = (char *)(env->GetStringUTFChars(steer, 0));
	char *Model = (char *)(env->GetStringUTFChars(model, 0));
	char *Setup = (char *)(env->GetStringUTFChars(setup, 0));
	char *Host = (char *)(env->GetStringUTFChars(host, 0));
	char *User = (char *)(env->GetStringUTFChars(user, 0));
	char *Passwd = (char *)(env->GetStringUTFChars(passwd, 0));

	CGAInit(Steer, Model, Setup, Host, User, Passwd);

	env->ReleaseStringUTFChars(steer, Steer);
	env->ReleaseStringUTFChars(model, Model);
	env->ReleaseStringUTFChars(setup, Setup);
	env->ReleaseStringUTFChars(host, Host);
	env->ReleaseStringUTFChars(user, User);
	env->ReleaseStringUTFChars(passwd, Passwd);
}

JNIEXPORT void JNICALL Java_CGARunManager_beamOn
  (JNIEnv *env , jobject, jdoubleArray start, jdoubleArray end, jdoubleArray direction, jstring particle, jfloat energy, jint nbPart) {
	
	  char *Particle = (char *)(env->GetStringUTFChars(particle, 0));

	  double * Start = (double *)(env->GetDoubleArrayElements(start, 0));
	  double * End = (double *)(env->GetDoubleArrayElements(end, 0));
	  double * Direction = (double *)(env->GetDoubleArrayElements(direction, 0));
	  CGABeamOn(Start, End, Direction, Particle, energy, nbPart);
	  env->ReleaseStringUTFChars(particle, Particle);
	  env->ReleaseDoubleArrayElements(start, Start, (int)env->GetArrayLength(start));
	  env->ReleaseDoubleArrayElements(end, End, (int)env->GetArrayLength(end));
	  env->ReleaseDoubleArrayElements(direction, Direction, (int)env->GetArrayLength(direction));
}

JNIEXPORT jstring JNICALL Java_CGARunManager_whereAmI
  (JNIEnv * env, jobject, jdoubleArray position) {

	double * Position = (double *)(env->GetDoubleArrayElements(position, 0));
	string volName;
	volName = CGAWhereAmIForJava(Position);
	env->ReleaseDoubleArrayElements(position, Position, (int)env->GetArrayLength(position));
	return env->NewStringUTF(volName.c_str());
}

JNIEXPORT void JNICALL Java_CGARunManager_getSteps
  (JNIEnv * env, jobject, jobject cont) {

	jclass cls = env->GetObjectClass(cont);
	jmethodID mid = env->GetMethodID(cls, "addElement", 
			  "(Ljava/lang/Object;)V");
	if (mid == 0) {
		cout << "Can't find method addElement" << endl;
		exit(1);
	}
	env->ExceptionClear();

	jclass theStep = env->FindClass("Step");
	if (theStep == 0) {
		cout << "Can't find Step class" << endl;
		exit(1);
	}
	jmethodID midStep = env->GetMethodID(theStep, "<init>",
		"(Ljava/lang/String;Ljava/lang/String;DDDDDD)V");
	if (midStep == 0) {
		cout << "Can't find constructor of class Step" << endl;
		exit(1);
	}

	vector<string> volNames;
	vector<string> matNames;
	vector<double> distance, x, y, z, nbX0, nInterLen;

	CGAGetStepsForJava(volNames, matNames, distance, x, y, z, nbX0, nInterLen);

	for(unsigned int iStep = 0; iStep < distance.size(); iStep++) {

		env->CallVoidMethod(cont, mid, env->NewObject(theStep, midStep, 
				env->NewStringUTF(volNames[iStep].c_str()),
				env->NewStringUTF(matNames[iStep].c_str()),
				distance[iStep], x[iStep], y[iStep], z[iStep], 
				nbX0[iStep], nInterLen[iStep]));

		if(env->ExceptionOccurred()) {
			cout << "error occured copying CGA data back" <<endl;
			env->ExceptionDescribe();
			env->ExceptionClear();
		}
	}
  }
  
JNIEXPORT void JNICALL Java_CGARunManager_getVolumeData
  (JNIEnv *env, jobject, jstring volName, jobject cont) {

	jclass cls = env->GetObjectClass(cont);
	jmethodID mid = env->GetMethodID(cls, "addElement", 
			  "(Ljava/lang/Object;)V");
	if (mid == 0) {
		cout << "Can't find method addElement" << endl;
		exit(1);
	}
	env->ExceptionClear();

	jclass theStep = env->FindClass("Step");
	if (theStep == 0) {
		cout << "Can't find Step class" << endl;
		exit(1);
	}
	jmethodID midStep = env->GetMethodID(theStep, "<init>",
		"(Ljava/lang/String;Ljava/lang/String;DDDDDD)V");
	if (midStep == 0) {
		cout << "Can't find constructor of class Step" << endl;
		exit(1);
	}

	char *  name = (char *)(env->GetStringUTFChars(volName, 0));
	vector<double> x, y, z;
	vector<double *> distance, nbX0, nInterLen;

	CGAGetVolumeDataForJava(name, distance, x, y, z, nbX0, nInterLen);

	for(unsigned int iStep = 0; iStep < distance.size(); iStep++) {

		env->CallVoidMethod(cont, mid, env->NewObject(theStep, midStep, 
				volName, volName, *(distance[iStep]), 
				x[iStep], y[iStep], z[iStep], 
				*(nbX0[iStep]), *(nInterLen[iStep])));

		if(env->ExceptionOccurred()) {
			cout << "error occured copying CGA data back" <<endl;
			env->ExceptionDescribe();
			env->ExceptionClear();
		}
	}
	env->ReleaseStringUTFChars(volName, name);
  }
  
JNIEXPORT void JNICALL Java_CGARunManager_setSD
  (JNIEnv *, jobject, jint flag) {

	  CGASetSD((int)flag);
}


JNIEXPORT void JNICALL Java_CGARunManager_getCellId
  (JNIEnv *env, jobject, jobject theUtility)
{
	jclass theCellIdUtility = env->GetObjectClass(theUtility);
	if (theCellIdUtility == 0) {
		cout << "Can't find CellIdUtility class " << endl;
		exit(1);
	}
	jmethodID midDouble = env->GetMethodID(theCellIdUtility, "x",
		"()D");
	if (midDouble == 0) {
		cout << "Can't find method x() of CellIdUtility" << endl;
		exit(1);
	}
	double X = (double) env->CallDoubleMethod(theUtility, midDouble);
	if(env->ExceptionOccurred()) {
		cout << "error occured retrieving X" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	midDouble = env->GetMethodID(theCellIdUtility, "y",
		"()D");
	if (midDouble == 0) {
		cout << "Can't find method y() of CellIdUtility" << endl;
		exit(1);
	}
	double Y = (double) env->CallDoubleMethod(theUtility, midDouble);
	if(env->ExceptionOccurred()) {
		cout << "error occured retrieving Y" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	midDouble = env->GetMethodID(theCellIdUtility, "z",
		"()D");
	if (midDouble == 0) {
		cout << "Can't find method z() of CellIdUtility" << endl;
		exit(1);
	}
	double Z = (double) env->CallDoubleMethod(theUtility, midDouble);
	if(env->ExceptionOccurred()) {
		cout << "error occured retrieving Z" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	midDouble = env->GetMethodID(theCellIdUtility, "xDir",
		"()D");
	if (midDouble == 0) {
		cout << "Can't find method xDir() of CellIdUtility" << endl;
		exit(1);
	}
	double xDir = (double) env->CallDoubleMethod(theUtility, midDouble);
	if(env->ExceptionOccurred()) {
		cout << "error occured retrieving xDir" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	midDouble = env->GetMethodID(theCellIdUtility, "yDir",
		"()D");
	if (midDouble == 0) {
		cout << "Can't find method yDir() of CellIdUtility" << endl;
		exit(1);
	}
	double yDir = (double) env->CallDoubleMethod(theUtility, midDouble);
	if(env->ExceptionOccurred()) {
		cout << "error occured retrieving yDir" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	midDouble = env->GetMethodID(theCellIdUtility, "zDir",
		"()D");
	if (midDouble == 0) {
		cout << "Can't find method zDir() of CellIdUtility" << endl;
		exit(1);
	}
	double zDir = (double) env->CallDoubleMethod(theUtility, midDouble);
	if(env->ExceptionOccurred()) {
		cout << "error occured retrieving zDir" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	int flagFromCGA;
	cell_ids cellIds = CGAGetCellId(X, Y, Z, flagFromCGA, xDir, yDir, zDir);
	
	jmethodID midVoid = env->GetMethodID(theCellIdUtility, "setCellIDs",
		"(II)V");
	if (midVoid == 0) {
		cout << "Can't find method setCellIDs() of CellIdUtility" << endl;
		exit(1);
	}
	env->CallVoidMethod(theUtility, midVoid, cellIds.id0, cellIds.id1);
	if(env->ExceptionOccurred()) {
		cout << "error occured while setting CellId" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	midVoid = env->GetMethodID(theCellIdUtility, "setFlag",
		"(I)V");
	if (midVoid == 0) {
		cout << "Can't find method setFlag() of CellIdUtility" << endl;
		exit(1);
	}
	env->CallVoidMethod(theUtility, midVoid, flagFromCGA);
	if(env->ExceptionOccurred()) {
		cout << "error occured while setting flag" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

JNIEXPORT jobject JNICALL Java_CGARunManager_cellIndex
  (JNIEnv *env, jobject, jint cellID0, jint cellID1) {

	jclass theVector = env->FindClass("java/util/Vector");
	if (theVector == 0) {
		cout << "Can't find Vector class" << endl;
		exit(1);
	}
	jmethodID midVector = env->GetMethodID(theVector, "<init>",
		"(I)V");
	if (midVector == 0) {
		cout << "Can't find constructor of class Vector" << endl;
		exit(1);
	}
	jobject result = env->NewObject(theVector, midVector, 3);
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Vector class" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jclass cls = env->GetObjectClass(result);
	jmethodID mid = env->GetMethodID(cls, "addElement", 
			  "(Ljava/lang/Object;)V");
	if (mid == 0) {
		cout << "Can't find method addElement" << endl;
		exit(1);
	}
	
	cell_ids indices;
	indices.id0 = (int)cellID0;
	indices.id1 = (int)cellID1;
	cell_info returnOfCGA = CGACellIndex(indices);

	jclass theDouble = env->FindClass("java/lang/Double");
	if (theDouble == 0) {
		cout << "Can't find Double class" << endl;
		exit(1);
	}
	jmethodID midDouble = env->GetMethodID(theDouble, "<init>",
		"(D)V");
	if (midDouble == 0) {
		cout << "Can't find constructor of class Double" << endl;
		exit(1);
	}
	jobject dblX = env->NewObject(theDouble, midDouble,
		returnOfCGA.X);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblX);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
		
	jobject dblY = env->NewObject(theDouble, midDouble,
		returnOfCGA.Y);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblY);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
		
	jobject dblZ = env->NewObject(theDouble, midDouble,
		returnOfCGA.Z);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblZ);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jclass theInt = env->FindClass("java/lang/Integer");
	if (theInt == 0) {
		cout << "Can't find Integer class" << endl;
		exit(1);
	}
	jmethodID midInt = env->GetMethodID(theInt, "<init>",
		"(I)V");
	if (midInt == 0) {
		cout << "Can't find constructor of class Integer" << endl;
		exit(1);
	}

	jobject intGRZone = env->NewObject(theInt, midInt,
		returnOfCGA.GRZone);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Integer class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, intGRZone);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	return result;
}

JNIEXPORT void JNICALL Java_CGARunManager_setTBConfigAngle
  (JNIEnv *, jobject, jfloat angle) {
	CGASetTBConfigAngle((float)angle);
}

/*
 * Class:     CGARunManager
 * Method:    getBdl
 * Signature: ([D[D)D
 */
JNIEXPORT jdouble JNICALL Java_CGARunManager_getBdl
  (JNIEnv *env, jobject, jdoubleArray start, jdoubleArray end) {

        double * Start = (double *)(env->GetDoubleArrayElements(start, 0));
        double * End = (double *)(env->GetDoubleArrayElements(end, 0));
        
	double Bdl = CGAGetBdl(Start, End);

        env->ReleaseDoubleArrayElements(start, Start, 
		(int)env->GetArrayLength(start));
        env->ReleaseDoubleArrayElements(end, End, 
		(int)env->GetArrayLength(end));

	return Bdl;
}
                                                                                
/*
 * Class:     CGARunManager
 * Method:    getEdl
 * Signature: ([D[D)D
 */
JNIEXPORT jdouble JNICALL Java_CGARunManager_getEdl
  (JNIEnv *env, jobject, jdoubleArray start, jdoubleArray end) {

        double * Start = (double *)(env->GetDoubleArrayElements(start, 0));
        double * End = (double *)(env->GetDoubleArrayElements(end, 0));
        
	double Edl = CGAGetEdl(Start, End);

        env->ReleaseDoubleArrayElements(start, Start, 
		(int)env->GetArrayLength(start));
        env->ReleaseDoubleArrayElements(end, End, 
		(int)env->GetArrayLength(end));

	return Edl;
}
                                                                                
/*
 * Class:     CGARunManager
 * Method:    CGAGetB
 * Signature: ([D)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_CGARunManager_getB
  (JNIEnv *env, jobject, jdoubleArray position) {
                                                                                
        double *Position=(double *)(env->GetDoubleArrayElements(position, 0));
        
	std::vector<double> B = CGAGetB(Position);

	jclass theVector = env->FindClass("java/util/Vector");
	if (theVector == 0) {
		cout << "Can't find Vector class" << endl;
		exit(1);
	}
	jmethodID midVector = env->GetMethodID(theVector, "<init>",
		"(I)V");
	if (midVector == 0) {
		cout << "Can't find constructor of class Vector" << endl;
		exit(1);
	}
	jobject result = env->NewObject(theVector, midVector, 3);
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Vector class" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jclass cls = env->GetObjectClass(result);
	jmethodID mid = env->GetMethodID(cls, "addElement", 
			  "(Ljava/lang/Object;)V");
	if (mid == 0) {
		cout << "Can't find method addElement" << endl;
		exit(1);
	}
	
	jclass theDouble = env->FindClass("java/lang/Double");
	if (theDouble == 0) {
		cout << "Can't find Double class" << endl;
		exit(1);
	}
	jmethodID midDouble = env->GetMethodID(theDouble, "<init>",
		"(D)V");
	if (midDouble == 0) {
		cout << "Can't find constructor of class Double" << endl;
		exit(1);
	}

	jobject dblX = env->NewObject(theDouble, midDouble,
		B[0]);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblX);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jobject dblY = env->NewObject(theDouble, midDouble,
		B[1]);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblY);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jobject dblZ = env->NewObject(theDouble, midDouble,
		B[2]);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblZ);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

        env->ReleaseDoubleArrayElements(position, Position, 
		(int)env->GetArrayLength(position));

	return result;
}

/*
 * Class:     CGARunManager
 * Method:    CGAGetE
 * Signature: ([D)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_CGARunManager_getE
  (JNIEnv *env, jobject, jdoubleArray position) {
                                                                                
        double *Position=(double *)(env->GetDoubleArrayElements(position, 0));
        
	std::vector<double> E = CGAGetE(Position);

	jclass theVector = env->FindClass("java/util/Vector");
	if (theVector == 0) {
		cout << "Can't find Vector class" << endl;
		exit(1);
	}
	jmethodID midVector = env->GetMethodID(theVector, "<init>",
		"(I)V");
	if (midVector == 0) {
		cout << "Can't find constructor of class Vector" << endl;
		exit(1);
	}
	jobject result = env->NewObject(theVector, midVector, 3);
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Vector class" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jclass cls = env->GetObjectClass(result);
	jmethodID mid = env->GetMethodID(cls, "addElement", 
			  "(Ljava/lang/Object;)V");
	if (mid == 0) {
		cout << "Can't find method addElement" << endl;
		exit(1);
	}
	
	jclass theDouble = env->FindClass("java/lang/Double");
	if (theDouble == 0) {
		cout << "Can't find Double class" << endl;
		exit(1);
	}
	jmethodID midDouble = env->GetMethodID(theDouble, "<init>",
		"(D)V");
	if (midDouble == 0) {
		cout << "Can't find constructor of class Double" << endl;
		exit(1);
	}

	jobject dblX = env->NewObject(theDouble, midDouble,
		E[0]);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblX);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jobject dblY = env->NewObject(theDouble, midDouble,
		E[1]);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblY);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jobject dblZ = env->NewObject(theDouble, midDouble,
		E[2]);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblZ);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

        env->ReleaseDoubleArrayElements(position, Position, 
		(int)env->GetArrayLength(position));

	return result;
}

/*
 * Class:     CGARunManager
 * Method:    getLocalPosition
 * Signature: ([D)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_CGARunManager_getLocalPosition
  (JNIEnv *env, jobject, jdoubleArray position) {

        double *Position=(double *)(env->GetDoubleArrayElements(position, 0));
        
	std::vector<double> localPosition = CGAGetLocalPosition(Position);

	jclass theVector = env->FindClass("java/util/Vector");
	if (theVector == 0) {
		cout << "Can't find Vector class" << endl;
		exit(1);
	}
	jmethodID midVector = env->GetMethodID(theVector, "<init>",
		"(I)V");
	if (midVector == 0) {
		cout << "Can't find constructor of class Vector" << endl;
		exit(1);
	}
	jobject result = env->NewObject(theVector, midVector, 3);
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Vector class" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jclass cls = env->GetObjectClass(result);
	jmethodID mid = env->GetMethodID(cls, "addElement", 
			  "(Ljava/lang/Object;)V");
	if (mid == 0) {
		cout << "Can't find method addElement" << endl;
		exit(1);
	}
	
	jclass theDouble = env->FindClass("java/lang/Double");
	if (theDouble == 0) {
		cout << "Can't find Double class" << endl;
		exit(1);
	}
	jmethodID midDouble = env->GetMethodID(theDouble, "<init>",
		"(D)V");
	if (midDouble == 0) {
		cout << "Can't find constructor of class Double" << endl;
		exit(1);
	}

	jobject dblX = env->NewObject(theDouble, midDouble,
		localPosition[0]);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblX);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jobject dblY = env->NewObject(theDouble, midDouble,
		localPosition[1]);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblY);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jobject dblZ = env->NewObject(theDouble, midDouble,
		localPosition[2]);	
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Double class" <<
			endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->CallVoidMethod(result, mid, dblZ);
	if(env->ExceptionOccurred()) {
		cout << "error occured copying CGA data back" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

        env->ReleaseDoubleArrayElements(position, Position, 
		(int)env->GetArrayLength(position));

	return result;
}

/*
 * Class:     CGARunManager
 * Method:    isCalorimeter
 * Signature: ([D)Z
 */
JNIEXPORT jboolean JNICALL Java_CGARunManager_isCalorimeter
  (JNIEnv *env, jobject, jdoubleArray position) {

        double *Position=(double *)(env->GetDoubleArrayElements(position, 0));
        
	bool result = CGAIsCalorimeter(Position);

        env->ReleaseDoubleArrayElements(position, Position, 
		(int)env->GetArrayLength(position));

	return result;
}
                                                                                
/*
 * Class:     CGARunManager
 * Method:    isTracker
 * Signature: ([D)Z
 */
JNIEXPORT jboolean JNICALL Java_CGARunManager_isTracker
  (JNIEnv *env, jobject, jdoubleArray position) {

        double *Position=(double *)(env->GetDoubleArrayElements(position, 0));
        
	bool result = CGAIsTracker(Position);

        env->ReleaseDoubleArrayElements(position, Position, 
		(int)env->GetArrayLength(position));

	return result;
}
                                                                                
/*
 * Class:     CGARunManager
 * Method:    getListOfLogicalVolumes
 * Signature: ([D)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_CGARunManager_getListOfLogicalVolumes
  (JNIEnv *env, jobject, jdoubleArray position) {
                                                                                
        double *Position=(double *)(env->GetDoubleArrayElements(position, 0));
        
	std::vector<std::string> list = 
		CGAGetListOfLogicalVolumes(Position);

	jclass theVector = env->FindClass("java/util/Vector");
	if (theVector == 0) {
		cout << "Can't find Vector class" << endl;
		exit(1);
	}
	jmethodID midVector = env->GetMethodID(theVector, "<init>",
		"(I)V");
	if (midVector == 0) {
		cout << "Can't find constructor of class Vector" << endl;
		exit(1);
	}
	jobject result = env->NewObject(theVector, midVector, 1);
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Vector class" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jclass cls = env->GetObjectClass(result);
	jmethodID mid = env->GetMethodID(cls, "addElement", 
			  "(Ljava/lang/Object;)V");
	if (mid == 0) {
		cout << "Can't find method addElement" << endl;
		exit(1);
	}
	
	jclass theString = env->FindClass("java/lang/String");
	if (theString == 0) {
		cout << "Can't find String class" << endl;
		exit(1);
	}
	jmethodID midString = env->GetMethodID(theString, "<init>",
		"(Ljava/lang/String;)V");
	if (midString == 0) {
		cout << "Can't find constructor of class String" << endl;
		exit(1);
	}

	for(unsigned int iStep = 0; iStep < list.size(); iStep++) {

		std::string lvName = list[iStep];

		env->CallVoidMethod(result, mid, env->NewObject(theString, 
				midString, env->NewStringUTF(lvName.c_str())));

		if(env->ExceptionOccurred()) {
			cout << "error occured copying CGA data back" <<endl;
			env->ExceptionDescribe();
			env->ExceptionClear();
		}
	}

        env->ReleaseDoubleArrayElements(position, Position, 
		(int)env->GetArrayLength(position));

	
	return result;
}

/*
 * Class:     CGARunManager
 * Method:    getListOfPhysicalVolumes
 * Signature: ([D)Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_CGARunManager_getListOfPhysicalVolumes
  (JNIEnv *env, jobject, jdoubleArray position) {

        double *Position=(double *)(env->GetDoubleArrayElements(position, 0));
        
	std::vector<std::string> list = 
		CGAGetListOfPhysicalVolumes(Position);

	jclass theVector = env->FindClass("java/util/Vector");
	if (theVector == 0) {
		cout << "Can't find Vector class" << endl;
		exit(1);
	}
	jmethodID midVector = env->GetMethodID(theVector, "<init>",
		"(I)V");
	if (midVector == 0) {
		cout << "Can't find constructor of class Vector" << endl;
		exit(1);
	}
	jobject result = env->NewObject(theVector, midVector, 1);
	if(env->ExceptionOccurred()) {
		cout << "error occured instantiating Vector class" <<endl;
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	jclass cls = env->GetObjectClass(result);
	jmethodID mid = env->GetMethodID(cls, "addElement", 
			  "(Ljava/lang/Object;)V");
	if (mid == 0) {
		cout << "Can't find method addElement" << endl;
		exit(1);
	}
	
	jclass theString = env->FindClass("java/lang/String");
	if (theString == 0) {
		cout << "Can't find String class" << endl;
		exit(1);
	}
	jmethodID midString = env->GetMethodID(theString, "<init>",
		"(Ljava/lang/String;)V");
	if (midString == 0) {
		cout << "Can't find constructor of class String" << endl;
		exit(1);
	}

	for(unsigned int iStep = 0; iStep < list.size(); iStep++) {

		std::string lvName = list[iStep];

		env->CallVoidMethod(result, mid, env->NewObject(theString, 
				midString, env->NewStringUTF(lvName.c_str())));

		if(env->ExceptionOccurred()) {
			cout << "error occured copying CGA data back" <<endl;
			env->ExceptionDescribe();
			env->ExceptionClear();
		}
	}

        env->ReleaseDoubleArrayElements(position, Position, 
		(int)env->GetArrayLength(position));

	
	return result;
}

/*
 * Class:     CGARunManager
 * Method:    getRegionName
 * Signature: ([D)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_CGARunManager_getRegionName
  (JNIEnv *env, jobject, jdoubleArray position) {
                                                                                
	double * Position = (double *)(env->GetDoubleArrayElements(position, 0));
	std::string regName = CGAGetRegionName(Position);
	env->ReleaseDoubleArrayElements(position, Position, (int)env->GetArrayLength(position));
	return env->NewStringUTF(regName.c_str());
}

/*
 * Class:     CGARunManager
 * Method:    getMaterialName
 * Signature: ([D)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_CGARunManager_getMaterialName
  (JNIEnv *env, jobject, jdoubleArray position) {

	double * Position = (double *)(env->GetDoubleArrayElements(position, 0));
	std::string matName = CGAGetMaterialName(Position);
	env->ReleaseDoubleArrayElements(position, Position, (int)env->GetArrayLength(position));
	return env->NewStringUTF(matName.c_str());
}

/*
 * Class:     CGARunManager
 * Method:    getDensity
 * Signature: ([D)D
 */
JNIEXPORT jdouble JNICALL Java_CGARunManager_getDensity
  (JNIEnv *env, jobject, jdoubleArray position) {
                                                                                
	double *Position = (double *)(env->GetDoubleArrayElements(position, 0));
	double density = CGAGetDensity(Position);
	env->ReleaseDoubleArrayElements(position, Position, (int)env->GetArrayLength(position));

	return density;
}

/*
 * Class:     CGARunManager
 * Method:    getTemperature
 * Signature: ([D)D
 */
JNIEXPORT jdouble JNICALL Java_CGARunManager_getTemperature
  (JNIEnv *env, jobject, jdoubleArray position) {
                                                                                
	double *Position = (double *)(env->GetDoubleArrayElements(position, 0));
	double temperature = CGAGetTemperature(Position);
	env->ReleaseDoubleArrayElements(position, Position, (int)env->GetArrayLength(position));

	return temperature;
}
/*
 * Class:     CGARunManager
 * Method:    getPressure
 * Signature: ([D)D
 */
JNIEXPORT jdouble JNICALL Java_CGARunManager_getPressure
  (JNIEnv *env, jobject, jdoubleArray position) {

	double *Position = (double *)(env->GetDoubleArrayElements(position, 0));
	double pressure = CGAGetPressure(Position);
	env->ReleaseDoubleArrayElements(position, Position, (int)env->GetArrayLength(position));

	return pressure;
}

/*
 * Class:     CGARunManager
 * Method:    getRadLen
 * Signature: ([D)D
 */
JNIEXPORT jdouble JNICALL Java_CGARunManager_getRadLen
  (JNIEnv *env, jobject, jdoubleArray position) {

	double *Position = (double *)(env->GetDoubleArrayElements(position, 0));
	double radLen = CGAGetRadLen(Position);
	env->ReleaseDoubleArrayElements(position, Position, (int)env->GetArrayLength(position));

	return radLen;
}
/*
 * Class:     CGARunManager
 * Method:    getIntLen
 * Signature: ([D)D
 */
JNIEXPORT jdouble JNICALL Java_CGARunManager_getIntLen
  (JNIEnv *env, jobject, jdoubleArray position) {

	double *Position = (double *)(env->GetDoubleArrayElements(position, 0));
	double intLen = CGAGetIntLen(Position);
	env->ReleaseDoubleArrayElements(position, Position, (int)env->GetArrayLength(position));

	return intLen;
}
