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

// lbDAG.h
// Ian McGraw

#ifndef _LB_DAG_H_
#define _LB_DAG_H_

#include <lbDirectedGraph.h>

namespace lbLib {
  /*!
    
    \version  July 2009
    \author  Ian McGraw
  */
  class lbDAG : public lbDirectedGraph {

  public:
    /*
     * Constructors and Destructors
     */
    lbDAG();
    lbDAG(int numNodes, arcVec const& arcVec);
    explicit lbDAG(lbDAG const& dag);
    ~lbDAG();

    /*
     * Accessors
     */

    inline virtual nodeIndexVec getTopologicalNodeSort() const;
    inline virtual arcIndexVec getTopologicalArcSort() const;
    inline virtual nodeIndexVec getAncestors(nodeIndex ni, bool includeSelf = false) const;
    inline virtual nodeIndexVec getDescendants(nodeIndex ni, bool includeSelf = false) const;

    inline virtual void print(ostream & out) const;
    inline virtual bool isValid() const;
  };

  inline nodeIndexVec lbDAG::getTopologicalNodeSort() const {
    nodeIndexVec sort;
    bool worked = lbDirectedGraph::getTopologicalSort(sort);
    assert(worked);
    return sort;
  }

  inline arcIndexVec lbDAG::getTopologicalArcSort() const {
    arcIndexVec sortedArcs;
    nodeIndexVec sortedNodes;
    bool worked = lbDirectedGraph::getTopologicalSort(sortedNodes);
    assert (worked);

    for (uint i = 0; i < sortedNodes.size(); i++) {
      nodeIndex ni = sortedNodes[i];
      arcIndexVec outgoing = getOutgoingArcIndices(ni);

      for (uint j = 0; j < outgoing.size(); j++) {
	sortedArcs.push_back(outgoing[j]);
      }
    }

    return sortedArcs;
  }

  inline nodeIndexVec lbDAG::getAncestors(nodeIndex ni, bool includeSelf) const {
    return getReachable(ni, ARC_DIR_IN, includeSelf);
  }

  inline nodeIndexVec lbDAG::getDescendants(nodeIndex ni, bool includeSelf) const {
    return getReachable(ni, ARC_DIR_OUT, includeSelf);
  }

  inline void lbDAG::print(ostream & out) const {
    out << "Printing DAG, thus... " << endl;
    lbDirectedGraph::print(out);
uint i; 
    for (i= 0; i < _outgoingCache.size(); i++) {
      out << "Children for " << i << ": ";
      for (uint j = 0; j < _outgoingCache[i].first.size(); j++) {
	out << _outgoingCache[i].first[j] << " ";
      }
      out << endl;
    }
    out << endl;

    for (i = 0; i < _incomingCache.size(); i++) {
      out << "Parents for " << i << ": ";
      for (uint j = 0; j < _incomingCache[i].first.size(); j++) {
	out << _incomingCache[i].first[j] << " ";
      }
      out << endl;
    }
    out << endl;
  }

  inline bool lbDAG::isValid() const {
    if (!lbDirectedGraph::isValid()) {
      return false;
    }

    return isAcyclic();
  }
};

#endif
