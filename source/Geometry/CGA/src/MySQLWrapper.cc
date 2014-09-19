// ********************************************************
// *            Mokka    http://mokka.in2p3.fr            *
// *    -- A Detailed Geant4 Simulation for the ILC --    *
// ********************************************************
//
// $Id: MySQLWrapper.cc,v 1.16 2006/12/07 10:47:55 adrian Exp $
// $Name: mokka-07-00 $
//
// adapted from the following programm:
/*
  MySQL C++ Wrapper Library

  Author: Roland Haenel <rh@ginster.net>

  This program is in the public domain.
  Distribute and use it freely.
*/

#include <string>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>

#include "MySQLWrapper.hh"

Database::Database()
{
  connected = false; // no connection yet
  res = NULL;
  strcpy(error, "No connection established");
}

// Database::Database(const char *host, const char *user, const char *passwd, const char *port, const char *db)
Database::Database(const char *db, const char *host, const char *user, const char *passwd, const char *)
{
  connected = false; // no connection yet
  res = NULL;
  strcpy(error, "No connection established");

  fprintf(stdout, "Connecting to the database \"%s\"\n", db);
  
  // Initialise the database - for future releases
  if (init() != DB_COMMAND_OK) {
    fprintf(stdout, "Unable to initialize database \"%s\"\n", errorMessage());
    exit(1);
  }
  
  // Connect to database
  if (connect(host, user, passwd, "", db) != DB_CONNECTION_OK) {
    fprintf(stdout, "Unable connect to database \"%s\": %s\n", db, errorMessage());
    Control::Abort("Database connection failed", MOKKA_ERROR_NO_CONNECTION_TO_DATABASE);
  }
}

Database::~Database()
{
  disconnect(); // Disconnect if connected to database
  if (res != NULL) delete res;
}

int Database::status()
{
  if (!connected) return DB_CONNECTION_NONE;
  else            return DB_CONNECTION_OK;
}

int Database::init()
{
  return DB_COMMAND_OK;
}

const char *Database::errorMessage()
{
  if (!connected) return error;
  return mysql_error(&mysql);
}

int Database::connect(const char *host, const char *user, const char *passwd, const char *port, const char *db)
{
  if (std::strtol(port, NULL, 0)) {
    strcpy(error, "Use the notation \"<host>:<port>\" or \"localhost:<socket>\" to connect to a particular port or socket");
    return DB_ERROR;
  }

  std::string hostname(host);
  unsigned int portnum = 0; // 0 = use default port (usually 3306)
  std::string socketfile;
  const char *socketfile_char = NULL; // NULL = use default name (depends on MySQL version and settings)
  bool customsettings = false;

  // split the hostname at the colon, if there is one
  // possible formats: "<host>:<port>" (for TCP connection) or "localhost:<socket>" (for Unix socket connection)
  std::string::size_type colonpos = hostname.find(":");
  if (colonpos != std::string::npos) { // found a colon?
    customsettings = true; // return an error message in old versions of MySQL (see below)
    if ((colonpos == 9) && (hostname.substr(0, 9) == "localhost")) {
      socketfile = hostname.substr(colonpos + 1, std::string::npos);
      socketfile_char = socketfile.c_str();
    } else {
      portnum = std::strtol(hostname.substr(colonpos + 1, std::string::npos).c_str(), NULL, 0);
    }
    hostname.erase(colonpos, std::string::npos); // strip away the colon and everything behind it
  }

#if MYSQL_VERSION_ID >= 32200

  mysql_init(&mysql);

  if (!mysql_real_connect(&mysql, hostname.c_str(), user, passwd, db, portnum, socketfile_char, 0)) {
    sprintf(error, "Failed to connect to database. Error: %s\n", mysql_error(&mysql));
    return DB_ERROR;
  }

#else // MYSQL_VERSION_ID < 32200

  if (customsettings) {
    strcpy(error, "Cannot connect to a particular port or socket with this version of MySQL");
    return DB_ERROR;
  }

  if (mysql_connect(&mysql, hostname.c_str(), user, passwd) == NULL) {
    strcpy(error, "Connect to database failed");
    return DB_ERROR;
  }

#endif // MYSQL_VERSION_ID

  if (mysql_select_db(&mysql, db)) {
    mysql_close(&mysql);
    strcpy(error, "No such database");  
    return DB_ERROR;
  }

  connected = true;
  return DB_CONNECTION_OK;
}

void Database::disconnect()
{
  if (!connected) return;
  mysql_close(&mysql);
  connected = false;
}

int Database::reset()
{
  return DB_COMMAND_OK;
}

DBResult* Database::exec(const char *sqlFormat, ...)
{
  va_list ap;
  char sqlCommand[5000];
  
  va_start(ap, sqlFormat);
  vsprintf(sqlCommand, sqlFormat, ap);
  va_end(ap);

  // since MySQL 3.23 we have to be sure the command has a ";"
  if (sqlCommand[strlen(sqlCommand) - 1] != ';') {
    G4cout << "MySQL commands must be terminated by \";\". "
      << "The offending command is:\n" << sqlCommand << G4endl;
    exit(1);
  }

  if (res != NULL) {
    res->init(&mysql, sqlCommand);
  } else {
    res = new DBResult(&mysql, sqlCommand);
  }
  if (res->status() == DB_ERROR) {
    G4cout << "MySQL command failed: " << sqlFormat << "\n"
      << mysql_error(&mysql) << G4endl;
    exit(1);
  }
  return res;
}

void Database::exec(DBResult *res, char *sqlFormat, ...)
{
  va_list ap;
  char sqlCommand[5000];
  
  va_start(ap, sqlFormat);
  vsprintf(sqlCommand, sqlFormat, ap);
  va_end(ap);

  // since MySQL 3.23 we have to be sure the command has a ";"
  if (sqlCommand[strlen(sqlCommand) - 1] != ';') {
    G4cout << "MySQL commands must be terminated by \";\". "
      << "The offending command is:\n" << sqlCommand << G4endl;
    exit(1);
  }
  
  res->init(&mysql, sqlCommand);
  if (res->status() == DB_ERROR) {
    G4cout << "MySQL command failed: " <<  sqlFormat << G4endl;
    exit(1);
  }
}

// ------------------- Database result implementation ------------------

DBResult::DBResult()
{
  result     = NULL;
  fields     = NULL;
  tuple      = NULL;
  haveError  = false;
}

DBResult::DBResult(MYSQL *mysql, char *query)
{
  result     = NULL;
  fields     = NULL;
  tuple      = NULL;
  init(mysql, query);
}

DBResult::~DBResult()
{
  if (result != NULL)      // Free memory resources
    mysql_free_result(result);
}

void DBResult::init(MYSQL *mysql, char *query)
{
  unsigned int n_fields = 0;
  if (result != NULL) {
    mysql_free_result(result);
    result = NULL;
  }

//  G4cout << "mysql_set_server_option returns "
//    << mysql_set_server_option(mysql, MYSQL_OPTION_MULTI_STATEMENTS_ON)
//    << G4endl;

  if (mysql_query(mysql, query) == 0) {    // query OK
    result = mysql_store_result(mysql);
    n_fields = mysql_field_count(mysql);
    
    if (result == NULL) {    // empty query
    // if (mysql_num_fields(result) == 0) // bad if the query is not a select! PMDF 06/2005
 
      if (n_fields == 0) { // it was not a select, test if query succeeded. PMDF 06/2005
        if (mysql_errno(mysql)) haveError = true;
        else haveError = false;
      } else haveError = true;
    } else haveError = false;
  } else haveError  = true;
  
  if (!haveError && n_fields > 0) { // good only if it is a select! PMDF 06/2005
    fields = mysql_fetch_fields(result);
  }
}

int DBResult::status()
{
  if (haveError)      return DB_ERROR;
  if (result == NULL) return DB_COMMAND_OK;
  return DB_TUPLES_OK;
}

int DBResult::nrTuples()
{
  if (result == NULL) return 0;
  return mysql_num_rows(result);
}

int DBResult::nrFields()
{
  if (result == NULL) return 0;
  return mysql_num_fields(result);
}

char *DBResult::fieldName(int n)
{
  MYSQL_FIELD *field;

  if (result == NULL) return NULL;
  mysql_field_seek(result, n);
  field = mysql_fetch_field(result);
  if (field == NULL) return NULL;
  return field->name;
}

int DBResult::fieldSize(int n)
{
  MYSQL_FIELD *field;

  if (result == NULL) return 0;
  mysql_field_seek(result, n);
  field = mysql_fetch_field(result);
  if (field == NULL) return 0;
  return field->length;
}

int DBResult::fieldSize(char *name)
{
  int i;
  
  if (result == NULL) return 0;
  for (i = 0; i < nrFields(); i++)
    if (!strcmp(name, fieldName(i)))
      return fieldSize(i);
  return 0;
}

void DBResult::seekTuple(int tuple)
{
  if (result == NULL) return;
  mysql_data_seek(result, tuple);
}

char **DBResult::getTuple()
{
  if (result == NULL) tuple = NULL;
  else tuple = mysql_fetch_row(result);

  return tuple;
}

char **DBResult::getTuple(int tuple)
{
  seekTuple(tuple);
  return getTuple();
}

double DBResult::fetchDouble(const char *fieldName)
{
  for (int i = 0; i < nrFields(); i++) {
    if (std::strcmp(fieldName, fields[i].name) == 0) { // field name matches
      if (tuple[i]) { // does the field have some content or is it NULL?
        return std::strtod(tuple[i], NULL);
      } else {
        std::fprintf(stdout, "DBResult::fetchDouble: Unexpected NULL value in field \"%s\".\n", fieldName);
        std::exit(1); // abort the program
      }
    }
  }
  std::fprintf(stdout, "DBResult::fetchDouble: Field \"%s\" not found.\n", fieldName);
  std::exit(1); // abort the program
  return 0; // dummy statement to suppress a compiler warning
}

double DBResult::fetchDouble(const int fieldIndex)
{
  if ((fieldIndex < 0) || (fieldIndex >= nrFields())) {
    std::fprintf(stdout, "DBResult::fetchDouble: No field number %d.\n", fieldIndex);
    std::exit(1); // abort the program
  }
  if (!tuple[fieldIndex]) { // does the field have some content or is it NULL?
    std::fprintf(stdout, "DBResult::fetchDouble: Unexpected NULL value in field number %d.\n", fieldIndex);
    std::exit(1); // abort the program
  }
  return std::strtod(tuple[fieldIndex], NULL);
}

int DBResult::fetchInt(const char *fieldName)
{
  for (int i = 0; i < nrFields(); i++) {
    if (std::strcmp(fieldName, fields[i].name) == 0) { // field name matches
      if (tuple[i]) { // does the field have some content or is it NULL?
        return std::strtol(tuple[i], NULL, 10); // or use symbolic base 0 to allow for
        // octal numbers (with leading 0) and hex numbers (with leading 0x) as well?
      } else {
        std::fprintf(stdout, "DBResult::fetchInt: Unexpected NULL value in field \"%s\".\n", fieldName);
        std::exit(1); // abort the program
      }
    }
  }
  std::fprintf(stdout, "DBResult::fetchInt: Field \"%s\" not found.\n", fieldName);
  std::exit(1); // abort the program
  return 0; // dummy statement to suppress a compiler warning
}

int DBResult::fetchInt(const int fieldIndex)
{
  if ((fieldIndex < 0) || (fieldIndex >= nrFields())) {
    std::fprintf(stdout, "DBResult::fetchInt: No field number %d.\n", fieldIndex);
    std::exit(1); // abort the program
  }
  if (!tuple[fieldIndex]) { // does the field have some content or is it NULL?
    std::fprintf(stdout, "DBResult::fetchInt: Unexpected NULL value in field number %d.\n", fieldIndex);
    std::exit(1); // abort the program
  }
  return std::strtol(tuple[fieldIndex], NULL, 10);
}

std::string DBResult::fetchString(const char *fieldName)
{
  for (int i = 0; i < nrFields(); i++) {
    if (std::strcmp(fieldName, fields[i].name) == 0) { // field name matches
      if (tuple[i]) { // does the field have some content or is it NULL?
        return std::string(tuple[i]);
      } else {
        std::fprintf(stdout, "DBResult::fetchString: Unexpected NULL value in field \"%s\".\n", fieldName);
        std::exit(1); // abort the program
      }
    }
  }
  std::fprintf(stdout, "DBResult::fetchString: Field \"%s\" not found.\n", fieldName);
  std::exit(1); // abort the program
  return std::string(); // dummy statement to suppress a compiler warning
}

std::string DBResult::fetchString(const int fieldIndex)
{
  if ((fieldIndex < 0) || (fieldIndex >= nrFields())) {
    std::fprintf(stdout, "DBResult::fetchString: No field number %d.\n", fieldIndex);
    std::exit(1); // abort the program
  }
  if (!tuple[fieldIndex]) { // does the field have some content or is it NULL?
    std::fprintf(stdout, "DBResult::fetchString: Unexpected NULL value in field number %d.\n", fieldIndex);
    std::exit(1); // abort the program
  }
  return std::string(tuple[fieldIndex]);
}
