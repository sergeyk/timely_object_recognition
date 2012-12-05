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

#include <lbMessageQueue.h>

using namespace lbLib;


lbMessageQueue::lbMessageQueue(lbGraphBase const& structure)
  : _structure(structure), 
    _dirtyWeightedGraph(&structure) {
  _iteration = 0;

  for (int i = 0; i < structure.getNumArcs(); i++) {
    _dirtyWeightedGraph.addArc(i);
  }
}

lbMessageQueue::lbMessageQueue(lbMessageQueue const& Mqueue) 
  : _structure(Mqueue._structure),
    _messageQueue(Mqueue._messageQueue),
    _iteration(Mqueue._iteration), 
    _dirtyWeightedGraph(Mqueue._dirtyWeightedGraph) {
}

lbAsyncQueue::lbAsyncQueue(lbGraphBase const& structure) 
  : lbMessageQueue(structure) {
  
}

lbAsyncQueue::lbAsyncQueue(lbAsyncQueue const& Aqueue)
  : lbMessageQueue(Aqueue) {
}

bool lbAsyncQueue::refreshQueue() {
  if (!isEmpty()) {
    return false;
  }

  clearQueue();
  
  cliqIndex fromCliq;
  
  // Pushing all messages from leaf variables on first
  for (fromCliq = 0; fromCliq < _structure.getNumNodes(); fromCliq++) {
    nodeIndexVec const& neighbors = _structure.getNeighboringNodeIndices((int) fromCliq);
    
    if (neighbors.size() == 1) {
      _messageQueue.push(indexPair(fromCliq, (cliqIndex) neighbors[0]));
    }
  }
  
  // Pushing all other messages on
  for (fromCliq = 0; fromCliq < _structure.getNumNodes(); fromCliq++) {
    nodeIndexVec neighbors =  _structure.getNeighboringNodeIndices((int) fromCliq);
    
    if (neighbors.size() != 1) {
      for (uint i = 0; i < neighbors.size(); i++) {
	_messageQueue.push(indexPair(fromCliq, (cliqIndex) neighbors[i]));
      }
    }
  }
  
  return true;
}





bool lbTRPQueue::_hybrid = false;

lbTRPQueue::lbTRPQueue(lbGraphBase const& structure,
		       lbGraphStruct const* clusterGraph, 
		       varIndicesVecVec const& scopes) 
  : lbMessageQueue(structure) {
  _clusterGraph = clusterGraph;
  _scopes = scopes;
  
}

lbTRPQueue::lbTRPQueue(lbTRPQueue const& Tqueue)   
  : lbMessageQueue(Tqueue),
    _clusterGraph(Tqueue._clusterGraph),
    _scopes(Tqueue._scopes) {
}


bool lbTRPQueue::refreshQueue() {
  clearQueue();
  
  lbSupport *support = NULL;
  
  support = new lbSupport(&_structure, _clusterGraph, _scopes);

  assert (support != NULL);
  
  for (int i = 0; i < support->getNumForests(); i++) {
    lbSubgraph const& spanning = support->getForest(i);
    
    arcVec ordering;
    bool worked = spanning.getRootedArcs(ordering);
    addArcsToQueue(ordering);
    assert (worked);

    if (_hybrid) {
      cerr << "NOT IMPLEMENTED HYBRID (need invert arcs)" << endl;
      NOT_REACHED;
      lbSubgraph remaining(spanning);
      //remaining.invertArcs();
      ordering.clear();
      remaining.getRootedArcs(ordering);
      addArcsToQueue(ordering);
    }
  } 
  
  delete support;
  return true;
}

