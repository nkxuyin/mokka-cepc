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
//// $Id: Step.java,v 1.2 2006/04/12 12:55:48 musat Exp $
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

public class Step {
	public Step() {};
	public Step(String volumeName, String materialName, double dist, 
		double xx, double yy, double zz, double x0, double InterLen) {
		
		volName = volumeName;
		matName = materialName;
		distance = dist; nbX0 = x0; nInterLen = InterLen;
		X = xx; Y = yy; Z = zz;	
	}
	
	private String volName, matName;
	private double distance, nbX0, nInterLen;
	private double X, Y, Z;
	
	public String VolName() { return volName;}
	public String MatName() { return matName;}
	public double Distance() { return distance; }
	public double X0() { return nbX0; }
	public double InterLen() { return nInterLen; }
	public double X() { return X; }
	public double Y() { return Y; }
	public double Z() { return Z; }
}

