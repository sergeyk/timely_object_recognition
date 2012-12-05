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

#ifndef _LIB_OPTIONS_H_
#define _LIB_OPTIONS_H_

#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace lbLib {
  
  /*
   * Generic classes for reading command line options.
   *
   * Standard usage is to create a new lbOptions instance.  Add the
   * appropriate options passing in a reference to their storage
   * (usually filled with its default value).
   */

  class lbOptions {
  public:
    lbOptions() {}
    ~lbOptions() {}

    void setUsagePrefix(string prefix);
    void usageError(string error) const;

    void addStringOption(string opt, string * value, string description);
    void addDoubleOption(string opt, double * value, string description);
    void addFloatOption(string opt, float * value, string description);
    void addIntOption(string opt, int * value, string description);
    void addBoolOption(string opt, bool * value, string description);

    void setOptions(int argc, char * argv[]);
    void ensureArgsHandled(int argc, char * argv[]) const;
    void printOptions(bool onlyUserSetOptions = false) const;

    bool isOption(string opt) const;
    bool isOptionSetByUser(string opt) const;

    static bool isVerbose(int verbosity);
    static void clearVerbose();
    static void printVerbosities();
    static void addVerboseOption(int verbosity, string description);

    void addVerbose(int verbosity);

  private:
    bool verbosityExists(int verbosity);

    string getValueString(int index) const;
    bool handleOption(string opt, string val);

    union ValueReference 
    {
      int * i;
      double * d;
      float * f;
      bool * b;
      string * s;
    };

    struct ValueStorage
    {
      int i;
      double d;
      float f;
      bool b;
      string s;
    };

    typedef enum { 
      OPT_TYPE_STRING, 
      OPT_TYPE_DOUBLE, 
      OPT_TYPE_FLOAT, 
      OPT_TYPE_INT,
      OPT_TYPE_BOOL } OptionType;

    string _usagePrefix;
    vector<string> _descriptions;
    vector<string> _optionsPossible;
    vector<ValueReference> _valueReferences;
    vector<ValueStorage> _valueStorages;
    vector<OptionType> _types;
    vector<bool> _setByUser;

    static vector<int> _verbositiesPossible;
    static vector<int> _verbositiesSetByUser;
    static vector<string> _verbosityDescriptions;
  };

};

#endif
