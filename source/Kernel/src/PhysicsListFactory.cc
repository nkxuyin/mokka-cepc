// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: PhysicsListFactory.cc,v 1.10 2008/01/31 13:28:56 musat Exp $
// $Name: mokka-07-00 $

#include "PhysicsListFactory.hh"
#include "Control.hh"

//#include "PhysicsList.hh"
//#include "PhysicsListNeutrons.hh"
//#include "LCPhysicsList.hh"

//SJA:Add TrackingPhysicsList
#include "TrackingPhysicsList.hh"

#include "LHEP.hh"
//#include "LHEP_LEAD.hh"
//#include "LHEP_LEAD_HP.hh"

//#include "LHEP_PRECO.hh"
#include "QGSP.hh"
//#include "QGSP_HP.hh"
#include "QGSP_BIC.hh"
#include "QGSP_BERT.hh"

//#include "QGSC_LEAD.hh"
//#include "QGSC_LEAD_HP.hh"

// New Physics lists with Geant4 8.0
//#include "LHEP_BIC.hh"
//#include "LHEP_BIC_HP.hh"
#include "LHEP_EMV.hh"
#include "QGSP_BERT_HP.hh"
#include "QGSP_BIC_HP.hh"

#include "QGSP_BERT_TRV.hh"

#include "QBBC.hh"
#include "QGSP_BERT_EMV.hh"

#if G4_VERSION_GE( 910 )
#include "FTFP_BERT.hh"
#include "QGSC_BERT.hh"
#include "QGS_BIC.hh"
#include "FTF_BIC.hh"
#endif

#if G4_VERSION_GE( 930 )
/*New physics lists with GEANT4 9.3beta version*/
#include "QGSP_FTFP_BERT.hh"
#include "QGSC_CHIPS.hh"
#include "FTFP_BERT_TRV.hh"

/*New physics lists with GEANT4 9.3 version*/
#include "CHIPS.hh"
//fg: obsolete in geant4 9.6 #include "QGSP_INCL_ABLA.hh"
#include "FTFP_BERT_EMV.hh"
#include "FTFP_BERT_EMX.hh"
#include "QGSP_BERT_EMX.hh"
#include "QGSP_BIC_EMY.hh"
#include "QGSP_BERT_NOLEP.hh"
#endif

#include "QGSP_QEL.hh"
#include "LBE.hh"

#if G4_VERSION_GE( 940 )
#include "QGSP_BERT_CHIPS.hh"
#else
#include "FTFC.hh"
#include "FTFP.hh"
#include "LHEP_BERT.hh"
#include "LHEP_PRECO_HP.hh"
#include "QGSC.hh"
#include "LHEP_BERT_HP.hh"
#include "QGSP_EMV.hh"
#include "QGSC_EMV.hh"
#include "QGSC_EFLOW.hh"
#include "FTFP_EMV.hh"
#include "QGSP_BERT_NQE.hh"
#include "QGSP_EMV_NQE.hh"
#include "QGSP_EMX.hh"
#include "QGSP_NQE.hh"
#if G4_VERSION_GE( 910 )
#include "QGSP_DIF.hh"
#include "QGSP_BERT_DIF.hh"
#endif
#endif
#if G4_VERSION_GE(960)
#include "FTFP_BERT_HP.hh"
#endif

G4VUserPhysicsList *PhysicsListFactory::create(const std::string &name)
{

  if (name == "LHEP")                return new LHEP;
//  else if (name == "LHEP_LEAD")           return new LHEP_LEAD;
//  else if (name == "LHEP_LEAD_HP")        return new LHEP_LEAD_HP;
//  else if (name == "LHEP_PRECO")          return new LHEP_PRECO;
  else if (name == "QGSP")                return new QGSP;
//  else if (name == "QGSP_HP")             return new QGSP_HP;
  else if (name == "QGSP_BIC")            return new QGSP_BIC;
  else if (name == "QGSP_BERT")           return new QGSP_BERT;
//  else if (name == "QGSC_LEAD")           return new QGSC_LEAD;
//  else if (name == "QGSC_LEAD_HP")        return new QGSC_LEAD_HP;
// New Physics lists with Geant4 8.0
//  else if (name == "LHEP_BIC")            return new LHEP_BIC;
//  else if (name == "LHEP_BIC_HP")         return new LHEP_BIC_HP;
  else if (name == "LHEP_EMV")            return new LHEP_EMV;
  else if (name == "QGSP_BERT_HP")        return new QGSP_BERT_HP;
  else if (name == "QGSP_BIC_HP")	  return new QGSP_BIC_HP;
  else if (name == "QGSP_BERT_TRV")	  return new QGSP_BERT_TRV;
  else if (name == "QBBC")	  	  return new QBBC;
  else if (name == "QGSP_BERT_EMV")	  return new QGSP_BERT_EMV;
  else if (name == "QGSP_QEL")		  return new QGSP_QEL;
  else if (name == "LBE")                 return new LBE;

#if G4_VERSION_GE( 910 )
  else if (name == "FTFP_BERT")		  return new FTFP_BERT;
  else if (name == "QGSC_BERT")		  return new QGSC_BERT;
  else if (name == "QGS_BIC")		  return new QGS_BIC;
  else if (name == "FTF_BIC")		  return new FTF_BIC;
#endif

#if G4_VERSION_GE( 930 )
 /*New physics lists with GEANT4 9.3beta version*/
  else if (name == "QGSP_FTFP_BERT") return new QGSP_FTFP_BERT;
  else if (name == "QGSC_CHIPS")     return new QGSC_CHIPS;
  else if (name == "FTFP_BERT_TRV")  return new FTFP_BERT_TRV;

  /*New physics lists with GEANT4 9.3 version*/
  else if (name == "CHIPS") return new CHIPS;
  //else if (name == "QGSP_INCL_ABLA")   return new QGSP_INCL_ABLA;
  else if (name == "FTFP_BERT_EMV")   return new FTFP_BERT_EMV;
  else if (name == "FTFP_BERT_EMX")   return new FTFP_BERT_EMX;
  else if (name == "QGSP_BERT_EMX")   return new QGSP_BERT_EMX;
  else if (name == "QGSP_BIC_EMY")    return new QGSP_BIC_EMY;
  else if (name == "QGSP_BERT_NOLEP") return new QGSP_BERT_NOLEP;
#endif

  /* cr: new physics list with GEANT9.6.x*/
#if G4_VERSION_GE( 960 )
  else if (name == "FTFP_BERT_HP")  return new FTFP_BERT_HP;
#endif

  //SJA:Add TrackingPhysicsList
  else if (name == "TrackingPhysicsList") return new TrackingPhysicsList;
  
#if G4_VERSION_GE( 940 )
 else if (name == "QGSP_BERT_CHIPS") return new QGSP_BERT_CHIPS;
#else
//  else if (name == "LCPhys")          return new LCPhysicsList;
  else if (name == "FTFC")            return new FTFC;
  else if (name == "FTFP")            return new FTFP;
  else if (name == "LHEP_BERT")       return new LHEP_BERT;
  else if (name == "LHEP_PRECO_HP")   return new LHEP_PRECO_HP;
  else if (name == "QGSC")            return new QGSC;
  else if (name == "LHEP_BERT_HP")    return new LHEP_BERT_HP;
  else if (name == "QGSP_EMV")        return new QGSP_EMV;
  else if (name == "QGSC_EMV")        return new QGSC_EMV;
  else if (name == "QGSC_EFLOW")      return new QGSC_EFLOW;
  else if (name == "FTFP_EMV")        return new FTFP_EMV;
  else if (name == "QGSP_BERT_NQE")   return new QGSP_BERT_NQE;
  else if (name == "QGSP_EMV_NQE")    return new QGSP_EMV_NQE;
  else if (name == "QGSP_EMX")	      return new QGSP_EMX;
  else if (name == "QGSP_NQE")	      return new QGSP_NQE;
#if G4_VERSION_GE( 910 )
  else if (name == "QGSP_DIF")	      return new QGSP_DIF;
  else if (name == "QGSP_BERT_DIF")   return new QGSP_BERT_DIF;
#endif
#endif

  else
    Control::Abort("Physics list not found",
                        MOKKA_ERROR_PHYSICS_LIST_NOT_FOUND);
}

