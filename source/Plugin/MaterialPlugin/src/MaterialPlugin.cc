// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: MaterialPlugin.cc,v 1.3 2006/06/18 00:12:26 adrian Exp $
// $Name: mokka-07-00 $

#include "MaterialPlugin.hh"
#include "MaterialPluginMessenger.hh"

#include "G4Material.hh"
#include "G4UnitsTable.hh"

INITPLUGIN(MaterialPlugin, "MaterialPlugin")

void MaterialPlugin::Init(void)
{
  _messenger = new MaterialPluginMessenger(this);
}

void MaterialPlugin::Exit(void)
{
  delete _messenger;
}

void MaterialPlugin::PrintMaterialInfo(const G4String &name) const
{
  if (name == "all")
    G4cout << *(G4Material::GetMaterialTable());
  else
    G4cout << *(G4Material::GetMaterial(name)) << G4endl;
}

void MaterialPlugin::PrintMaterialLengths(void) const
{
  const std::ios::fmtflags savedFlags = G4cout.flags();
  const G4long savedPrecision = G4cout.precision(4);
  G4cout.setf(std::ios::left, std::ios::adjustfield);
  G4cout.setf(std::ios::showpoint | std::ios::uppercase);
  
  const G4MaterialTable *table = G4Material::GetMaterialTable();
  for (G4MaterialTable::const_iterator material = table->begin(); material != table->end(); material++) {
    const G4double radLength = (*material)->GetRadlen();
    const G4double nucLength = (*material)->GetNuclearInterLength();
    const G4double density   = (*material)->GetDensity();

    G4cout
      << std::setw(20) << (*material)->GetName()
      << " RadLength: " << std::setw(5) << G4BestUnit(radLength, "Length")
      << " (" << std::setw(5) << radLength * density / (g/cm2) << " g/cm2)"
      << "   NucLength: " << std::setw(5) << G4BestUnit(nucLength, "Length")
      << " (" << std::setw(5) << nucLength * density / (g/cm2) << " g/cm2)"
      << G4endl;
  }
  
  G4cout.precision(savedPrecision);
  G4cout.setf(savedFlags);
}
