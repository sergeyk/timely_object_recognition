/* Copyright 2009 Ariel Jaimovich, Ofer Meshi, Ian McGraw and Gal Elidan */


/*
This file is part of FastInf library.

FastInf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FastInf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FastInf.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef __GNUG__
#ifdef PRAGMA_TEMPLATES
#pragma implementation "Log.h"
#endif
#endif

// TODO: replace LEDA's dictionary

// #ifndef _MSC_VER
// #include <iostream.h>
// #include <fstream.h>
// #else
#include<iostream>
#include <fstream>
// #endif

#include <stdlib.h>
#include <signal.h>
// #include <LEDA/d_array.h>
#include <string.h>
#include <Log.h>
#include <Memory.h>

#ifdef LOG_OPTION

#ifdef SIG_PFV 
static struct sigvec X;
static SIG_PFV old_SEGV_handler;

void LogTerminate(void);

static void
LogSignal(DOTDOTDOT)
{
  LogTerminate();
  sigvec( SIGSEGV, NULL, &X );
  X.sv_handler = old_SEGV_handler;
  sigvec( SIGSEGV, &X, NULL );
  *(char*) NULL = 0;
}

static void
SetSignal()
{

  sigvec( SIGSEGV, NULL, &X );
  old_SEGV_handler = X.sv_handler;
  X.sv_handler = LogSignal;
  sigvec( SIGSEGV, &X, NULL );
}

#else

static void
SetSignal()
{
}	

#endif

static
struct tLogStatus {
  ofstream* LogStream;
  static bool LogInitialized;

  bool On;
  int  Level;
  bool AllClasses;

  typedef map<string, bool, less<string> > tClassMap;

  tClassMap _Class;

#ifdef MEMCHK
  tMemoryLog _Mem;
#endif
  
  tLogStatus() {
    LogStream = new ofstream( "logfile", ios::out );
    LogInitialized = true;
    On = true;
    AllClasses = false;
    Level = 1;
    SetSignal();
  }

  void Terminate()
  {
    if( LogStream != NULL )
    {
      DebugPrintStatistics( *LogStream );
      PrintMemStatistics( *LogStream );
      LogStream->flush();
      LogStream->close();
      delete LogStream;
      LogStream = NULL;
    }
  }
  
  ~tLogStatus()
  {
    Terminate();
  }

} Log;

#ifdef MEMCHK
tMemoryLog* MemoryLog = &Log._Mem;
#endif

bool tLogStatus::LogInitialized = false;

ostream&
LogFile(void)
{
    if( !Log.LogInitialized || Log.LogStream == NULL )
      return cerr;
    else
      return *Log.LogStream;
}

bool LogClass( const char *type )
{
  if( !Log.On )
    return false;

  string Name(type);
  tLogStatus::tClassMap::iterator i = Log._Class.find( Name );
  
  if( i != Log._Class.end() )
    return (*i).second;
  else
    return Log.AllClasses;
}

void
LogCreation(const char *type, void *ptr)
{
  if( LogClass(type) && Log.LogStream != NULL )
    (*Log.LogStream) << "Creation: [" << ptr << "] " << type << "\n";
  DebugRecordConstruct( type, ptr );
}

void LogDestruction(const char *type, void *ptr)
{
  if( LogClass(type) && Log.LogStream != NULL )
    (*Log.LogStream) << "Destruct: [" << ptr << "] " << type << "\n";
  DebugRecordDestruct( type, ptr );
}

void LogDeleteBad(const char *type, void *ptr)
{
  (*Log.LogStream) << "Deleted twice!: [" << ptr << "] " << type << "\n";
#ifdef notdef
  Log.Terminate();
  // cause core dump
  *(char*)NULL = '\0';
#endif  
}

void LogAssert(char *expr, char *file, int line)
{
  if( Log.LogStream != NULL )
    (*Log.LogStream) << "Assert [" << file << ", " << line << "]" << expr << "\n";
  Log.Terminate();
}

void LogTerminate(void)
{
  Log.Terminate();
}

void LogOption( char *opt )
{

  if( *opt == '+' || *opt == '-' )
  {
    bool On = (*opt == '+');
    opt++;

    if( *opt == '\0' )
      Log.AllClasses = On ;
    else
    {
      tLogStatus::tClassMap::iterator i = Log._Class.find( opt );
  
      if( i != Log._Class.end() )
	(*i).second = On;
      else
	Log._Class.insert(
		     tLogStatus::tClassMap::value_type(opt, On) );
    }
  } 
  else
    Log.Level = atoi(opt);
}

void LogOptionFile( char *file )
{
}

int
LogLevel(void)
{
  return Log.Level;
}

#endif
