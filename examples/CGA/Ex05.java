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
//// $Id: Ex05.java,v 1.8 2006/05/23 15:15:09 musat Exp $
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
//// This example uses the LCIO (see 
//// http://www-it.desy.de/physics/projects/simsoft/lcio)
//// and was built according to the LCTools example in java
////-------------------------------------------------------


import java.util.*;
import java.lang.*;
import hep.lcio.event.*;
import hep.lcio.implementation.io.LCFactory;
import hep.lcio.io.*;
import java.io.IOException;
import hep.lcio.data.*;


public class Ex05 {
	static CGARunManager run;

	public static void main(String args[]) throws IOException {

 		run = new CGARunManager();

		if (args.length == 0)
        		 help();

		LCReader lcReader = LCFactory.getInstance().createLCReader();
		lcReader.open(args[0]);
		for (;;) {
			LCRunHeader runHdr = lcReader.readNextRunHeader();
			if (runHdr == null)
				break;
			System.out.println("  Run : " + runHdr.getRunNumber() + " - " + runHdr.getDetectorName() + ":  " + runHdr.getDescription());
			run.init(
				runHdr.getParameters().
					getStringVal("MOKKA_SteeringFile"),
				runHdr.getDetectorName(), "", "", "", ""
				);
		}

		lcReader.close();
		lcReader.open(args[0]);

		double start[]={-68, 169, 0};
		double end[]={-200*Math.sin(3.1418/8), 200*Math.cos(3.1418/8),
			0};//z = -283.5 to touch four EnvLogs
		double direction[]={end[0]-start[0], end[1]-start[1],
			end[2]-start[2]};
		run.beamOn(start, end, direction, "geantino", 20, 1);

		int nEvents = 0;
		for (;;) {
			LCEvent evt = lcReader.readNextEvent();
			if (evt == null)
				break;
// call dumpEvt to extract event information
			dumpEvt(evt);			

			nEvents++;
		}
		
		System.out.println("  " + nEvents + " events read from file : " + args[0]);
		lcReader.close();
	}

	private static void help() {
		System.out.println(
		"java -cp $LCIO/src/java:$LCIO/lib/lcio.jar:" + 
		"$LCIO/tools/sio.jar:.:$G4WORKDIR/tmp/$G4SYSTEM/CGAJava " + 
		Ex05.class.getName() + " <input-file>");
		System.exit(1);
	}


   public static void dumpEvt(LCEvent evt)
   {
// you have to build the same detector model that the one
// used by Mokka to generate the LCIO file

      // the event:
      System.out.println("event  : " + evt.getEventNumber() + " - run " + evt.getRunNumber() + " detector : " + evt.getDetectorName() + " - collections  : ");

      String[] strVec = evt.getCollectionNames();

      // loop over collections:
      for (int j = 0; j < strVec.length; j++)
      {
         String name = strVec[j];

         System.out.print("     " + name + " " + evt.getCollection(name).getTypeName() + " : ");

         LCCollection col = evt.getCollection(name);

// cellIndex only deals with SimCalorimeterHit collections:
         if (evt.getCollection(name).getTypeName().equals(LCIO.SIMCALORIMETERHIT))
         {
// first use the collection flag to set the sensitive detector
            run.setSD(col.getFlag());
            int nPrint = col.getNumberOfElements();
            System.out.print(nPrint + " hits: ");

            if (nPrint == 0)
               System.out.println(nPrint);
	    System.out.println();
            for (int i = 0; i < nPrint; i++)
            {
               SimCalorimeterHit hit = (SimCalorimeterHit) col.getElementAt(i);

               float[] x = hit.getPosition();

	       int id00 = hit.getCellID0() ;
	       int id01 = hit.getCellID1() ;

// now use the CellID0 to get the cell center coordinates
		CellIdUtility theUtility = 
			new CellIdUtility(x[0], x[1], x[2]);
		if((x[0] != 0.0) && (x[1] != 0.0) && (x[2] != 0.0))
			run.getCellId(theUtility);	
//		int id10 = theUtility.cellID0();
//		int id11 = theUtility.cellID1();
//		if(id00 != id10) {
//			System.out.println("id00: " + id00 + " - id10: " +
//				id10);
//			System.exit(1);
//		}
//		if(id01 != id11) {
//			System.out.println("id01: " + id01 + " - id11: " +
//				id11);
//			System.exit(1);
//		}
	       Vector cgaResult = run.cellIndex(id00, id01);

               System.out.print("    hit - pos: (" + x[0] + ", " + x[1] + ", " + x[2]);

	       System.out.print(") - pos from CGA: (" 
		+ ((Double)(cgaResult.elementAt(0))).floatValue() + ", " 
		+ ((Double)(cgaResult.elementAt(1))).floatValue() + ", " 
		+ ((Double)(cgaResult.elementAt(2))).floatValue() + "); "
		+ "GRZone: " + 
		( (Integer)(cgaResult.elementAt(3))).intValue() );
               System.out.println();
            }
         }
      }
   }
}
