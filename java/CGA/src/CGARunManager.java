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
//// $Id: CGARunManager.java,v 1.9 2006/05/23 11:42:08 musat Exp $
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


import java.util.*;

public class CGARunManager {
	public CGARunManager() {
	}
	public native void init(String steer, String model, String setup, 
				String host, String user, String passwd);
	public native void beamOn(double []start, double []end, 
		double []direction, String particle, float energy, 
		int nbPart);
	public native String whereAmI(double []position);
	public native void getSteps(Vector cont);
	public native void getVolumeData(String name, Vector cont);
	public native void setSD(int flag);
	public native Vector cellIndex(int cellID0, int cellID1);
	public native void getCellId(CellIdUtility anUtility);
	public native void setTBConfigAngle(float angle);
	public native double getBdl(double []start, double []end);
	public native double getEdl(double []start, double []end);
	public native Vector getB(double []position);
	public native Vector getE(double []position);
	public native Vector getLocalPosition(double []position);
	public native boolean isCalorimeter(double []position);
	public native boolean isTracker(double []position);
	public native Vector getListOfLogicalVolumes(double []position);
	public native Vector getListOfPhysicalVolumes(double []position);
	public native String getRegionName(double []position);
	public native String getMaterialName(double pos[]);
	public native double getDensity(double []pos);
	public native double getTemperature(double []pos);
	public native double getPressure(double []pos);
	public native double getRadLen(double []pos);
	public native double getIntLen(double []pos);

       	static {
               System.loadLibrary("CGAJava");
        }
}

