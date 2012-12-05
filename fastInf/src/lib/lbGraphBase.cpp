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

// lbGraphBase.cpp
// Author: Ian McGraw

#include <lbGraphBase.h>

using namespace lbLib;


const string lbGraphBase::SIZE_DELIM = "@Size";
const string lbGraphBase::ARCS_DELIM = "@Arcs";
const string lbGraphBase::END_DELIM = "@End";


lbGraphBase::lbGraphBase() {
  _numNodes = 0;
} 

lbGraphBase::lbGraphBase(int numNodes, arcVec const& arcVec) {
  assert(numNodes > 0);

  _numNodes = numNodes;
  _neighborsCache.resize(numNodes);
  _arcs = arcVec;

  for (arcIndex ai = 0; (uint) ai < _arcs.size(); ai++) {
    nodeIndex na = _arcs[ai].first;
    nodeIndex nb = _arcs[ai].second;

    _neighborsCache[na].first.push_back(nb);
    _neighborsCache[na].second.push_back(ai);
    
    _neighborsCache[nb].first.push_back(na);
    _neighborsCache[nb].second.push_back(ai);
  }
}
	    
lbGraphBase::lbGraphBase(lbGraphBase const& graph) {
  _numNodes = graph._numNodes;
  _arcs = graph._arcs;
  _neighborsCache = graph._neighborsCache;
}

lbGraphBase& lbGraphBase::operator=(lbGraphBase const& graph) {
  _numNodes = graph._numNodes;
  _arcs = graph._arcs;
  _neighborsCache = graph._neighborsCache;
  return *this;
}


lbGraphBase::~lbGraphBase() {
  // Nothing to be done.
}
