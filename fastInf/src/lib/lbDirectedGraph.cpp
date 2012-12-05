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

// lbDirectedGraph.cpp
// Author: Ian McGraw

#include <lbDirectedGraph.h>

using namespace lbLib;

lbDirectedGraph::lbDirectedGraph() : lbGraphBase() {
  // Creates an empty directed graph
}


lbDirectedGraph::lbDirectedGraph(int numNodes, arcVec const& arcVec) 
  : lbGraphBase(numNodes, arcVec) {
  // By now we've been initialized as a GraphBase.  Time to fix
  // up the outgoing and incoming caches.

  _outgoingCache.resize(_numNodes);
  _incomingCache.resize(_numNodes);

  for (arcIndex ai = 0; (uint) ai < _arcs.size(); ai++) {
    nodeIndex nfrom = _arcs[ai].first;
    nodeIndex nto = _arcs[ai].second;

    _outgoingCache[nfrom].first.push_back(nto);
    _outgoingCache[nfrom].second.push_back(ai);
    
    _incomingCache[nto].first.push_back(nfrom);
    _incomingCache[nto].second.push_back(ai);
  }
}
	    
lbDirectedGraph::lbDirectedGraph(lbDirectedGraph const& graph) 
  : lbGraphBase(graph) {
  _outgoingCache = graph._outgoingCache;
  _incomingCache = graph._incomingCache;
}

lbDirectedGraph::~lbDirectedGraph() {
  // Nothing to do.
}
