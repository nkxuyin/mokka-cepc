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
//// $Id: CellIdUtility.java,v 1.2 2006/02/06 17:25:55 musat Exp $
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


public class CellIdUtility {
	public CellIdUtility(double X, double Y, double Z) {
		x = X; y = Y; z = Z; xdir = 0; ydir = 0; zdir = 1.0;
	}

	public CellIdUtility(double X, double Y, double Z,
		double xDir, double yDir, double zDir) {
		
		x = X; y = Y; z = Z; xdir = xDir; ydir = yDir;
		zdir = zDir;
	}
	
	private double x, y, z, xdir, ydir, zdir;
	private int cellID0, cellID1, flag;
	
	public double x() { return x; }
	public double y() { return y; }
	public double z() { return z; }
	public double xDir() { return xdir; }
	public double yDir() { return ydir; }
	public double zDir() { return zdir; }
	public int cellID0() { return cellID0;}
	public int cellID1() { return cellID1;}
	public int flag() { return flag;}
	public void setCellIDs(int cellid0, int cellid1) { 
		cellID0 = cellid0;
		cellID1 = cellid1;
	}
	public void setFlag(int aFlag) { flag = aFlag;}
}

