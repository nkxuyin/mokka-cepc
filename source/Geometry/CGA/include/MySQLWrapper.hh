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
// $Id: MySQLWrapper.hh,v 1.5 2006/12/07 10:47:55 adrian Exp $
// $Name: mokka-07-00 $
//
/*
	mysql C++ wrapper library

	Author: Roland Haenel <rh@ginster.net>

	This program is in the public domain.
	Distribute and use it freely.
*/

#ifndef __DATABASE_H
#define __DATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>

#include "mysql.h"
#include "Control.hh"


#define DB_CONNECTION_NONE	0
#define DB_CONNECTION_OK	1
#define DB_CONNECTION_BAD	2

#define DB_COMMAND_OK		0	// OK - command executed
#define DB_EMPTY_QUERY		1	// Query didn't return tuples
#define DB_TUPLES_OK		2	// Query returned tuples
#define DB_ERROR		5
#define DB_BAD_RESPONSE		6
#define DB_UNEXPECTED		7	// This shouldn't ever happen

class DBResult {
public:
  DBResult();
  DBResult(MYSQL *mysql, char *query);
  ~DBResult();

  void init(MYSQL *mysql, char *query);
  
  int status();			// Return query status
  
  int nrTuples();			// Number of fetched tuples
  int nrFields();			// Number of fields per tuple
  
  char *fieldName(int n);		// Name of nth fiel
  int   fieldSize(int n);		// Size of nth field
  int   fieldSize(char *name);	// Size of nth field

  double      fetchDouble(const char *name); // fetch named field as double value
  double      fetchDouble(const int index);  // fetch indexed field as double value (starting from 0)
  int         fetchInt(const char *name);    // fetch named field as int value
  int         fetchInt(const int index);     // fetch indexed field as int value (starting from 0)
  std::string fetchString(const char *name); // fetch named field as string value
  std::string fetchString(const int index);  // fetch indexed field as string value (starting from 0)

  char **getTuple();              // Return the next tuple or NULL
  char **getTuple(int tuple);	// Return tuple

private:
  bool haveError;
  MYSQL_RES *result;
  MYSQL_FIELD *fields;
  MYSQL_ROW tuple;

  void seekTuple(int tuple);      // Sets internal cursor to tuple
};

class Database {
public:
  Database();
  Database(const char *db,
	   const char *host=Control::DBHOST, 
	   const char *user=Control::USER,
	   const char *passwd=Control::DBPASSWD,
	   const char *port="");
  
  virtual ~Database();
  
  int init();			// Initialize and do basic tests
  int status();			// Return status information
  const char *errorMessage();		// Return current error message
  
  // Connect to db
  int connect(const char *host, 
	      const char *user,
	      const char *passwd,
	      const char *port, 
	      const char *db);	
  
  void disconnect();		// Disconnect from database
  int reset();			// Reset connection
  
  DBResult *exec(const char *sqlFormat, ...);	// Execute arbitrary SQL cmd
  // Same as above, reuse res
  void exec(DBResult *res, char *sqlFormat, ...);

  // Access to the current result
  // Number of fetched tuples
  int nrTuples()                            { if (res) return res->nrTuples();         else return 0; }
  // skip to the next tuple or NULL
  char **getTuple()                         { if (res) return res->getTuple();         else return NULL; }
  // fetch field with a double value
  double fetchDouble(const char *name)      { if (res) return res->fetchDouble(name);  else return 0; }
  double fetchDouble(const int index)       { if (res) return res->fetchDouble(index); else return 0; }
  // fetch field with an int value
  int fetchInt(const char *name)            { if (res) return res->fetchInt(name);     else return 0; }
  int fetchInt(const int index)             { if (res) return res->fetchInt(index);    else return 0; }
  // fetch field with a string value
  std::string fetchString(const char *name) { if (res) return res->fetchString(name);  else return std::string(); }
  std::string fetchString(const int index)  { if (res) return res->fetchString(index); else return std::string(); }

private:
  MYSQL mysql;
  bool connected;
  char error[1000]; // Error description
  DBResult *res;
};

#endif
