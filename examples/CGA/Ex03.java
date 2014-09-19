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
//// $Id: Ex03.java,v 1.7 2006/05/23 11:42:08 musat Exp $
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

public class Ex03 {
	public static void main(String args[]) {
		CGARunManager run = new CGARunManager();
		run.init("", "D09M1", "", "", "", "");

		double start[]={-68, 169, 0};
		double end[]={-200*Math.sin(3.1418/8), 200*Math.cos(3.1418/8),
				0};//z = -283.5 to touch four EnvLogs
		double direction[]={end[0]-start[0], end[1]-start[1], 
				end[2]-start[2]};
		
		run.beamOn(start, end, direction, "geantino", 20, 1);

		System.out.println("=================================");

		Vector vec = new Vector();
		run.getSteps(vec);	
		for(int i = 0; i < vec.size(); i++)
			System.out.println(i + " " + 
			((Step)vec.elementAt(i)).VolName() 
			+ " " + ((Step)vec.elementAt(i)).MatName()+ " " + 
			((Step)vec.elementAt(i)).Distance() + " " + 
			((Step)vec.elementAt(i)).X() + " " + 
			((Step)vec.elementAt(i)).Y() + " " +
			((Step)vec.elementAt(i)).Z() + " " +
			((Step)vec.elementAt(i)).X0()+ " " +
			((Step)vec.elementAt(i)).InterLen());

		System.out.println("=================================");

		Vector vec2 = new Vector();
		run.getVolumeData("EnvLog", vec2);	
		for(int i = 0; i < vec2.size(); i++)
			System.out.println(i + " " + 
			((Step)vec2.elementAt(i)).VolName() 
			+ " " + ((Step)vec2.elementAt(i)).Distance() + " " + 
			((Step)vec2.elementAt(i)).X() + " " + 
			((Step)vec2.elementAt(i)).Y() + " " +
			((Step)vec2.elementAt(i)).Z() + " " +
			((Step)vec2.elementAt(i)).X0()+ " " +
                        ((Step)vec.elementAt(i)).InterLen());

		System.out.println("=================================");

		System.out.println(run.whereAmI(end));

		double p0[]={0.0, 0.0, 0.0};
		double p1[]={0.0, 0.0, 1000.0};

		System.out.println("Bdl=" + run.getBdl(p0, p1));
		System.out.println("Edl=" + run.getEdl(p0, p1));

		Vector B = run.getB(p0);
		System.out.println("B:("+B.elementAt(0)+","+B.elementAt(1)+
			","+B.elementAt(2)+")");

		Vector E = run.getE(p0);
		System.out.println("E:("+E.elementAt(0)+","+E.elementAt(1)+
			","+E.elementAt(2)+")");

		double p2[] = {0.0, 1730.0, 0.0};
		Vector pos = run.getLocalPosition(p2);
		System.out.println("LocalPosition:("+pos.elementAt(0)+","+
			pos.elementAt(1)+","+pos.elementAt(2)+")");

		boolean res = run.isCalorimeter(p2);
		if(res)
			System.out.println("Position:("+p2[0]+","+
			p2[1]+","+p2[2]+") is in calorimeter region");

		double p3[] = {0.0, 1000.0, 0.0};
		res = run.isTracker(p3);
		if(res)
			System.out.println("Position:("+p3[0]+","+
			p3[1]+","+p3[2]+") is in tracker region");

		Vector list;
		list = run.getListOfLogicalVolumes(p2);
		System.out.println("LogicalVolumes:");
		for(int i=0; i< list.size(); i++)
			System.out.print(list.elementAt(i)+" ");
		System.out.println("===");

		list = run.getListOfPhysicalVolumes(p2);
		System.out.println("PhysicalVolumes:");
		for(int i=0; i< list.size(); i++)
			System.out.print(list.elementAt(i)+" ");
		System.out.println("===");

		System.out.println("Regionname: " + run.getRegionName(p2));
        	System.out.println("MaterialName: " + run.getMaterialName(p2));
		System.out.println("Density: " + run.getDensity(p2));
        	System.out.println("Temperature: " + run.getTemperature(p2));
        	System.out.println("Pressure: " + run.getPressure(p2));
        	System.out.println("RadLen: " + run.getRadLen(p2));
        	System.out.println("InteractionLength: " + run.getIntLen(p2));
	}
}
