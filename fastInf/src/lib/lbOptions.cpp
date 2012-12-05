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

#include <lbDefinitions.h>
#include <lbOptions.h> 
#include <sstream>
#include <assert.h>

using namespace lbLib;

vector<int> lbOptions::_verbositiesPossible;
vector<int> lbOptions::_verbositiesSetByUser;
vector<string> lbOptions::_verbosityDescriptions; 

void lbOptions::setUsagePrefix(string prefix) {
  _usagePrefix = prefix;
}

void lbOptions::usageError(string error) const {
  printOptions();
  cerr << endl << error << endl;
  exit(1);
}

void lbOptions::addStringOption(string opt, string * value, string description) {
  ValueReference val;
  val.s = value;

  ValueStorage valstore;
  valstore.s = *value;

  assert(!isOption(opt));
  _descriptions.push_back(description);
  _valueReferences.push_back(val);
  _valueStorages.push_back(valstore);
  _optionsPossible.push_back(opt);
  _types.push_back(OPT_TYPE_STRING);
  _setByUser.push_back(false);
}

void lbOptions::addDoubleOption(string opt, double * value, string description) {
  ValueReference val;
  val.d = value;

  ValueStorage valstore;
  valstore.d = *value;

  assert(!isOption(opt));
  _descriptions.push_back(description);
  _valueReferences.push_back(val);
  _valueStorages.push_back(valstore);
  _optionsPossible.push_back(opt);
  _types.push_back(OPT_TYPE_DOUBLE);
  _setByUser.push_back(false);
}

void lbOptions::addFloatOption(string opt, float * value, string description) {
  ValueReference val;
  val.f = value;

  ValueStorage valstore;
  valstore.f = *value;

  assert(!isOption(opt));
  _descriptions.push_back(description);
  _valueReferences.push_back(val);
  _valueStorages.push_back(valstore);
  _optionsPossible.push_back(opt);
  _types.push_back(OPT_TYPE_FLOAT);
  _setByUser.push_back(false);
}

void lbOptions::addIntOption(string opt, int * value, string description) {
  ValueReference val;
  val.i = value;

  ValueStorage valstore;
  valstore.i = *value;

  assert(!isOption(opt));
  _descriptions.push_back(description);
  _valueReferences.push_back(val);
  _valueStorages.push_back(valstore);
  _optionsPossible.push_back(opt);
  _types.push_back(OPT_TYPE_INT);
  _setByUser.push_back(false);
}

void lbOptions::addBoolOption(string opt, bool * value, string description) {
  ValueReference val;
  val.b = value;

  ValueStorage valstore;
  valstore.b = *value;

  assert(!isOption(opt));
  _descriptions.push_back(description);
  _valueReferences.push_back(val);
  _valueStorages.push_back(valstore);
  _optionsPossible.push_back(opt);
  _types.push_back(OPT_TYPE_BOOL);
  _setByUser.push_back(false);
}

bool lbOptions::handleOption(string opt, string val) {
  bool handled = false;
  uint i;

  if (opt == "v") {
    addVerbose(atoi(val.c_str()));
    return true;
  }

  for (i = 0; i < _optionsPossible.size(); i++) {
    if (_optionsPossible[i] == opt) {
      _setByUser[i] = true;
      handled = true;

      switch (_types[i]) {
      case OPT_TYPE_STRING:
	*_valueReferences[i].s = val;
	_valueReferences[i].s = NULL;
	_valueStorages[i].s = val;
	break;
      case OPT_TYPE_DOUBLE:
	*_valueReferences[i].d = atof(val.c_str());
	_valueReferences[i].d = NULL;
	_valueStorages[i].d = atof(val.c_str());
	break;
      case OPT_TYPE_FLOAT:
	*_valueReferences[i].f = atof(val.c_str());
	_valueReferences[i].f = NULL;
	_valueStorages[i].f = atof(val.c_str());
	break;
      case OPT_TYPE_INT:
	*_valueReferences[i].i = atoi(val.c_str());
	_valueReferences[i].i = NULL;
	_valueStorages[i].i = atoi(val.c_str());
	break;
      case OPT_TYPE_BOOL:
	if (val.size() != 1 || !(val[0] == '+' || val[0] == '-')) {
	  ostringstream oss;
	  oss << "Boolean operator \"" << opt << "\" must have a '+' or '-' value.";
	  usageError(oss.str());
	}

	*_valueReferences[i].b = (val[0] == '+');
	_valueReferences[i].b = NULL;
	_valueStorages[i].b = (val[0] == '+');
	break;
      default:
	cerr << "Unknown option type (see lbOptions.cpp)" << endl;
	assert(false);
      }
    }
  }

  return handled;
}

void lbOptions::setOptions(int argc, char *argv[]) {
  int i;

  assert(_types.size() == _valueReferences.size());
  assert(_valueReferences.size() == _optionsPossible.size());
  assert(_optionsPossible.size() == _descriptions.size());
  assert(_descriptions.size() == _setByUser.size());

  if (argc%2 != 1) {
    // Wrong number of arguments (should be odd because of program
    // name and pairs of options/arguments)
    usageError("Options must come in -opt value pairs");
  }

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '\0') // already handled
      continue;

    if (!argv[i][0] == '-') {
      string error = "Invalid option ";
      error += argv[i][0];
      error += ".  Options must begin with '-'.";
      usageError(error);
    }

    bool handled = handleOption(argv[i] + 1, argv[i+1]);

    if (handled) {
      argv[i][0] = '\0';
      argv[i+1][0] = '\0';
    }

    i++;
  }
}

void lbOptions::printOptions(bool onlyUserSetOptions) const {
  assert(_types.size() == _valueReferences.size());
  assert(_valueReferences.size() == _optionsPossible.size());
  assert(_optionsPossible.size() == _descriptions.size());
  assert(_descriptions.size() == _setByUser.size());
  
  uint i;
  for (i = 0; i < _descriptions.size(); i++) {
    if (!onlyUserSetOptions || _setByUser[i]) {
      cerr << "-" << _optionsPossible[i] << "\t" << _descriptions[i];
      cerr << " (" << getValueString(i) << ")" << endl;
    }
  }

  printVerbosities();
}

string lbOptions::getValueString(int index) const {
  assert (index >= 0 && index < (int) _types.size());
  assert (index >= 0 && index < (int) _valueReferences.size());

  ostringstream oss;
  switch (_types[index]) {
  case OPT_TYPE_STRING:
    oss << _valueStorages[index].s;
    break;
  case OPT_TYPE_DOUBLE:
    oss << _valueStorages[index].d;
    break;
  case OPT_TYPE_FLOAT:
    oss << _valueStorages[index].f;
    break;
  case OPT_TYPE_INT:
    oss << _valueStorages[index].i;
    break;
  case OPT_TYPE_BOOL:
    if (_valueStorages[index].b) {
      oss << "+";
    }
    else {
      oss << "-";
    }
    break;
  }

  return oss.str();
}

void lbOptions::ensureArgsHandled(int argc, char * argv[]) const {
  string error = "Unrecognized options: ";
  bool found = false;

  // check if there is unhandled option
  for ( int i=1 ; i<argc ; i++ ) {
    if ( argv[i][0] != '\0' ) {
      error += argv[i];
      error += " ";
      found = true;
    }  
  }

  if (found) {
    usageError(error);
  }
}

bool lbOptions::isOption(string opt) const {
  uint i;

  for (i = 0; i < _optionsPossible.size(); i++) {
    if (_optionsPossible[i] == opt) {
      return true;
    }
  }

  return false;
}

bool lbOptions::isOptionSetByUser(string opt) const {
  uint i;

  for (i = 0; i < _optionsPossible.size(); i++) {
    if (_optionsPossible[i] == opt && _setByUser[i]) {
      return true;
    }
  }

  return false;
}

bool lbOptions::isVerbose(int verbosity) {
  uint i;

  for (i = 0; i < _verbositiesSetByUser.size(); i++) {
    if (_verbositiesSetByUser[i] == verbosity) {
      return true;
    }
  }

  return false;
} 

void lbOptions::printVerbosities() {
  if (_verbositiesPossible.size() > 0) {
    cerr << "Verbosities: " << endl;
  }

  for (uint i = 0; i < _verbositiesPossible.size(); i++) {
    cerr << _verbositiesPossible[i] << ": " << _verbosityDescriptions[i];
    if (isVerbose(_verbositiesPossible[i])) {
      cerr << " (+)" << endl;
    }
    else {
      cerr << " (-)" << endl;
    }
  }
}

void lbOptions::clearVerbose() {
  _verbositiesPossible.clear();
  _verbosityDescriptions.clear();
}

bool lbOptions::verbosityExists(int verbosity) {
  uint i;

  for (i = 0; i < _verbositiesPossible.size(); i++) {
    if (_verbositiesPossible[i] == verbosity) {
      return true;
    }
  }

  return false;
}

void lbOptions::addVerboseOption(int verbosity, string description) {
  _verbositiesPossible.push_back(verbosity);
  _verbosityDescriptions.push_back(description);
}

void lbOptions::addVerbose(int verbosity) {

  if (!verbosityExists(verbosity)) {
    ostringstream oss;
    oss << "Verbosity " << verbosity << " does not exist";
    usageError(oss.str());
  }

  _verbositiesSetByUser.push_back(verbosity);
}
