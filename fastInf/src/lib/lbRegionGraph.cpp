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

// lbRegionGraph.cpp
// Author: Ian McGraw

#include <lbRegionGraph.h>

using namespace lbLib;

const string lbRegionGraph::REGIONS_DELIM = "@Regions";

lbRegionGraph::lbRegionGraph()
  : lbDAG() {
  // Nothing to do.
}


lbRegionGraph::lbRegionGraph(varIndicesVecVec const& vivv, 
			     facIndicesVecVec const& fivv, 
			     arcVec const& arcVec) 
  : lbDAG(vivv.size(), arcVec) {
  assert(vivv.size() == fivv.size());
  
  for (uint i = 0; i < vivv.size(); i++) {
    _regions.push_back(lbRegion(vivv[i], fivv[i]));
  }
  assert(isValid());
}

lbRegionGraph::lbRegionGraph(lbRegionVec const& rgns, arcVec const& arcVec) 
  : lbDAG(rgns.size(), arcVec) {
  _regions = rgns;
  assert(isValid());
}

lbRegionGraph::lbRegionGraph(lbRegionGraph const& rgnGraph)
  : lbDAG(rgnGraph) {
  _regions = rgnGraph._regions;
  _countingNumsFile = rgnGraph.getCountingNumsFile() ;
  assert(isValid());
}



lbRegionGraph::~lbRegionGraph() {
  // Nothing to do.
}



/*
 * Implementation of the lbRegion.
 */

lbRegion::lbRegion() {
  // Nothing to do.  Empty region.
}

lbRegion::lbRegion(varsVec vars, facIndicesVec fiv) {
  _varIndices = vars;
  _facIndices = fiv;
}

void lbRegion::readRegion(istream & in) {
  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);

  // Get the name (I don't use it for now)
  assert(in.good() && !in.eof());
  in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE, lbDefinitions::DELIM);
  string str(buffer.get());

  readVarIndices(in);
  readFacIndices(in);
}

void lbRegion::readVarIndices(istream & in) {
  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);

  istringstream iss;

  in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE, lbDefinitions::DELIM);
  string strnumvars(buffer.get());

  iss.str(strnumvars);
  int numVars;
  iss >> numVars;

  assert(numVars >= 0);

  istringstream iss2;
  in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE, lbDefinitions::DELIM);
  string strvars(buffer.get());
  iss2.str(strvars);

  for (int i = 0; i < numVars; i++) {
    int var;
    iss2 >> var;
    _varIndices.push_back(var);
  }
}

void lbRegion::readFacIndices(istream & in) {
  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);

  istringstream iss;

  in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE, lbDefinitions::DELIM);
  string strnumfacs(buffer.get());

  iss.str(strnumfacs);
  int numFacs;
  iss >> numFacs;

  assert(numFacs >= 0);

  istringstream iss2;
  in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE);
  string strfacs(buffer.get());
  iss2.str(strfacs);

  for (int i = 0; i < numFacs; i++) {
    int fac;
    iss2 >> fac;
    _facIndices.push_back(fac);
  }
}

bool lbRegion::isEmpty() const {
  return (_varIndices.empty() && _facIndices.empty());
}

bool lbRegion::isEqual(lbRegion const& rgn) const {
  return (isSubsetOf(rgn) && rgn.isSubsetOf(*this));
}

bool lbRegion::isSubsetOf(lbRegion const& rgn , bool withFactors) const {
  return (vecSubset(_varIndices, rgn._varIndices) &&
	  (!withFactors || vecSubset(_facIndices, rgn._facIndices)));
}

lbRegion lbRegion::getIntersection(lbRegion const& rgn) const {
  varsVec vars = vecIntersection(_varIndices, rgn._varIndices);
  sort(vars.begin(), vars.end());
  
  facIndicesVec facs = vecIntersection(_facIndices, rgn._facIndices);
  sort(facs.begin(), facs.end());
  
  return lbRegion(vars, facs);
}

void lbRegion::print(ostream & out) const {
  out << "Vars: ";
  printVector(_varIndices, out);
  out << "Facs: ";
  printVector(_facIndices, out);
}

void lbRegion::printFormatted(ostream & out) const {
  out << _varIndices.size() << "\t";
  uint  i;
  for (i = 0; i < _varIndices.size(); i++) {
    out << _varIndices[i] << " ";
  }
 
  out << "\t" << _facIndices.size() << "\t";
  for (i = 0; i < _facIndices.size(); i++) {
    out << _facIndices[i] << " ";
  }
  out << endl;
}
