// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: UserInit.cc,v 1.5 2006/08/16 18:21:24 adrian Exp $
// $Name: mokka-07-00 $

#include "UserInit.hh"
#include "globals.hh"

UserInit *UserInit::_me = 0; // UserInit is a singleton

UserInit *UserInit::getInstance(void)
{
  if (_me == 0)
    _me = new UserInit;
  return _me;
}

const std::string &UserInit::getString(const std::string &name) const
{
  StringMap::const_iterator position = _stringMap.find(name);
  if (position == _stringMap.end()) {
    G4cout << "WARNING: User variable \"" << name << "\" of type string is unknown, you'll get the empty string." << G4endl;
    static const std::string empty = std::string();
    return empty; // emergency return value
  }
  return position->second;
}

double UserInit::getDouble(const std::string &name) const
{
  DoubleMap::const_iterator position = _doubleMap.find(name);
  if (position == _doubleMap.end()) {
    G4cout << "WARNING: User variable \"" << name << "\" of type double is unknown, you'll get \"0.0\"." << G4endl;
    return 0;
  }
  return position->second;
}

int UserInit::getInt(const std::string &name) const
{
  IntMap::const_iterator position = _intMap.find(name);
  if (position == _intMap.end()) {
    G4cout << "WARNING: User variable \"" << name << "\" of type int is unknown, you'll get \"0\"." << G4endl;
    return 0;
  }
  return position->second;
}

bool UserInit::getBool(const std::string &name) const
{
  BoolMap::const_iterator position = _boolMap.find(name);
  if (position == _boolMap.end()) {
    G4cout << "WARNING: User variable \"" << name << "\" of type bool is unknown, you'll get \"false\"." << G4endl;
    return false;
  }
  return position->second;
}

void UserInit::setUserVariable(const std::string &name, const std::string &value)
{
  if (_stringMap.find(name) != _stringMap.end())
    G4cout << "WARNING: User variable \"" << name << "\" of type string already exists, skipping assignment." << G4endl;
  else
    _stringMap.insert(StringMap::value_type(name, value));
}

void UserInit::setUserVariable(const std::string &name, double value)
{
  if (_doubleMap.find(name) != _doubleMap.end())
    G4cout << "WARNING: User variable \"" << name << "\" of type double already exists, skipping assignment." << G4endl;
  else
    _doubleMap.insert(DoubleMap::value_type(name, value));
}

void UserInit::setUserVariable(const std::string &name, int value)
{
  if (_intMap.find(name) != _intMap.end())
    G4cout << "WARNING: User variable \"" << name << "\" of type int already exists, skipping assignment." << G4endl;
  else
    _intMap.insert(IntMap::value_type(name, value));
}

void UserInit::setUserVariable(const std::string &name, bool value)
{
  if (_boolMap.find(name) != _boolMap.end())
    G4cout << "WARNING: User variable \"" << name << "\" of type bool already exists, skipping assignment." << G4endl;
  else
    _boolMap.insert(BoolMap::value_type(name, value));
}
