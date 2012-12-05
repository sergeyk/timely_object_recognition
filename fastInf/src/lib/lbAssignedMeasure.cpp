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

#include <lbAssignedMeasure.h>

using namespace lbLib ;

//int lbAssignedMeasure::_counter = 0;


lbAssignedMeasure_ptr lbAssignedMeasure::readOneMeasure(ifstream_ptr in, lbMeasureDispatcher const& disp,
							bool readLogValues) {
  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);

  (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE, lbDefinitions::DELIM);
  string str(buffer.get());
  istringstream iss(str);
  
  int size;
  iss >> size;
  
  (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE, lbDefinitions::DELIM);
  string strvars(buffer.get());
  istringstream iss2(strvars);
  
  varsVec vec;
  for (int i = 0; i < size; i++) {
    int var;
    iss2 >> var;
    vec.push_back(var);
  }
  
  lbMeasure_Sptr mes = disp.getNewMeasure();
  
  paramPtrMap sharedMap;
  paramPtrMap idleMap;
  mes->readOneMeasure(in, sharedMap, idleMap, readLogValues);
  return (new lbAssignedMeasure(mes, vec));
}

bool lbAssignedMeasure::raiseToThePower(probType power) {
  return _measure->raiseToThePower(power);
}
