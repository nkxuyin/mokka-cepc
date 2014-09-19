// *********************************************************
// *                         Mokka                         *
// *    -- A detailed Geant 4 simulation for the ILC --    *
// *                                                       *
// *  polywww.in2p3.fr/geant4/tesla/www/mokka/mokka.html   *
// *********************************************************
//
// $Id: UserInit.hh,v 1.6 2006/08/16 18:21:24 adrian Exp $
// $Name: mokka-07-00 $
//
// UserInit is used to store user-defined steering variables
// of different data types, set by "/Mokka/init/userInit..."

#ifndef UserInit_hh
#define UserInit_hh 1

#include <string>
#include <map>

class UserInit
{
public:
  // UserInit is a singleton
  static UserInit *getInstance(void);

  const std::string &getString(const std::string &name) const;
  double getDouble(const std::string& name) const;
  int getInt(const std::string &name) const;
  bool getBool(const std::string &name) const;

  void setUserVariable(const std::string &name, const std::string &value);
  void setUserVariable(const std::string &name, double value);
  void setUserVariable(const std::string &name, int value);
  void setUserVariable(const std::string &name, bool value);
  
protected:
  typedef std::map<std::string, std::string> StringMap;
  typedef std::map<std::string, double> DoubleMap;
  typedef std::map<std::string, int> IntMap;
  typedef std::map<std::string, bool> BoolMap;

protected:
  UserInit() {}

  static UserInit *_me;

  StringMap _stringMap;
  DoubleMap _doubleMap;
  IntMap _intMap;
  BoolMap _boolMap;
};

#endif
