//
//*******************************************************
//*                                                     *
//*                      Mokka                          * 
//*   - the detailed geant4 simulation for Tesla -      *
//*                                                     *
//* For more information about Mokka, please, go to the *
//*                                                     *
//*  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
//*                                                     *
//* Mokka home page.                                    *
//*                                                     *
//*******************************************************
//
// $Id: PrimaryContribution.cc,v 1.2 2004/06/23 08:45:00 frank Exp $
// $Name: mokka-07-00 $
//
// 
#include "globals.hh"
#include "PrimaryContribution.hh"
  
PrimaryContribution::PrimaryContribution(G4int pPDG, G4double pE, G4double pt, float*sp)
  : E(0.), time(0.) , _firstEdep(true) 
{
  AddEdep(pPDG,pE,pt,sp);
}

PrimaryContribution::PrimaryContribution(const PrimaryContribution &right)
{
  PDGContributions = right.PDGContributions;
}
 
 const PrimaryContribution& PrimaryContribution::operator=(const PrimaryContribution &right)
   {
  PDGContributions = right.PDGContributions;
  return *this;
}

void PrimaryContribution::AddEdep(G4int pPDG, G4double pE, G4double pt, float*sp )
{
// //   G4cout << "PrimaryContribution::AddEdep" << G4endl;
//   E += pE; // Total for the attached PID

//   G4double& E_PDG = PDGContributions[pPDG];
// //   G4cout << "E_PDG encontrado = " << E_PDG  << G4endl;

//   E_PDG += pE; // Total for this PDG for attached PID
// //   G4cout << "E_PDG depois da soma = " << E_PDG  << G4endl;


// fg: added time and create one antry for each secondary
  E += pE ;
  if( _firstEdep ) {
    time = pt ; // FIXME:  what is a reasonable logic for defining the time if not in PDG (detailed) mode ?? 
    _firstEdep = false ;
  }
  
  PDGEntry currentEntry;
  currentEntry.energy = pE;
  currentEntry.time = pt;
  currentEntry.stepPosition = sp;
  PDGContributions.insert( std::make_pair(   pPDG , currentEntry  )  )  ;
  //PDGContributions.insert( std::make_pair(   pPDG ,  std::make_pair( pE , pt   )  )  )  ;

}
