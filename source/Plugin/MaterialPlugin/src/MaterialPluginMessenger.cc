// *********************************************************
// *                         Mokka                         *
// *    -- A Detailed Geant 4 Simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: MaterialPluginMessenger.cc,v 1.2 2006/06/18 00:12:26 adrian Exp $
// $Name: mokka-07-00 $

#include "MaterialPluginMessenger.hh"
#include "MaterialPlugin.hh"

#include "G4UIcommand.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithoutParameter.hh"

MaterialPluginMessenger::MaterialPluginMessenger(MaterialPlugin *handler): _materialPlugin(handler)
{
  G4String candidates;
  const G4MaterialTable *table = G4Material::GetMaterialTable();
  for (G4MaterialTable::const_iterator material = table->begin(); material != table->end(); material++)
    candidates += (*material)->GetName() + " ";
  candidates += "all";

  _printMaterialInfoCmd = new G4UIcmdWithAString("/Mokka/printMaterialInfo", this);
  _printMaterialInfoCmd->SetParameterName("material", true, false);
  _printMaterialInfoCmd->SetCandidates(candidates);
  _printMaterialInfoCmd->SetDefaultValue("all");
  _printMaterialInfoCmd->SetGuidance("Prints various information about known G4Materials.");

  _printMaterialLengthsCmd = new G4UIcmdWithoutParameter("/Mokka/printMaterialLengths", this);
  _printMaterialLengthsCmd->SetGuidance("Prints radiation length and nuclear interaction length of known G4Materials.");
    
  G4cout << "MaterialPlugin has added two commands to \"/Mokka/\"." << G4endl;
  G4cout << "See the Mokka help for further explanations." << G4endl;
}

MaterialPluginMessenger::~MaterialPluginMessenger(void)
{
  delete _printMaterialLengthsCmd;
  delete _printMaterialInfoCmd;
}

void MaterialPluginMessenger::SetNewValue(G4UIcommand *command, G4String newValue)
{
  if      (command == _printMaterialInfoCmd)    _materialPlugin->PrintMaterialInfo(newValue);
  else if (command == _printMaterialLengthsCmd) _materialPlugin->PrintMaterialLengths();
}
