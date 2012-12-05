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

#include <lbMessageBank.h>
#include <iomanip>
using namespace std;

using namespace lbLib;

lbMessageBank::lbMessageBank(adjListVec const& neighbors)
  : _neighbors(neighbors) {

  _updateSize = 1;
  _totalDirty = 0;
  _totalUpdated = 0;

  _affectAll = false;
  _compareType = C_MAX;
  _threshold = 10e-5;
  _smoothParam = 0.5;
}

lbMessageBank::~lbMessageBank() {
  clearMessages();
}

void lbMessageBank::printRealMessages(messageIndex mi,ostream& O) const {
  lbAssignedMeasure_ptr real = getMessage(mi);
  probType* vec = new probType[real->getMeasure().getSize()];
  int v;
  for(v=0;v<real->getMeasure().getSize();v++)
    vec[v] = 0.0;
  real->getMeasure().extractValuesAddToVector(vec,0,false);
  //    O << " " << m << ":";
  for(v=0;v<real->getMeasure().getSize();v++)
    O << " " << vec[v];
  delete[] vec;
}

void lbMessageBank::printRealMessages(ostream& O) const {
  messageMap::const_iterator it;
  O << setprecision(10);
  for (it=_realMessages.begin(); it!=_realMessages.end(); it++ )
    printRealMessages(it->first,O);
  O << endl;
}
