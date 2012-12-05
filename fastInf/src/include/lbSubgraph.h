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

#ifndef _LIB_SUBGRAPH_H_
#define _LIB_SUBGRAPH_H_

#include <lbDefinitions.h>
#include <lbGraphBase.h>
#include <queue>
#include <vector>
#include <map>

using namespace std; 

/*
 * This class essentially stores a set of arc indices and
 * a reference to the graph from which the arc indices come.
 * These arcs represent the subgraph in question.  We are
 * also able to ask questions about the subgraph from here.
 *
 * If an arc is in the subgraph, then the nodes that it
 * connects are also guaranteed to be in the subgraph, however
 * singleton nodes can be in the subgraph.
 *
 * The class is meant to export a lot of the same functionality
 * that a graph does without doubling the storage size.
 * Hence the bitmaps.
 */
namespace lbLib {

  class lbSubgraph {
    public:

    /*
     * Constructors and Destructors
     */
    lbSubgraph(lbGraphBase const* graph); /* Empty subgraph over graph */
    lbSubgraph(lbSubgraph const&);
    virtual ~lbSubgraph();

    /*
     * Mutators
     */
    inline virtual void clear();
    inline virtual void addArc(arcIndex ai); 
    inline virtual void addVar(rVarIndex vi);
    inline virtual void addNode(nodeIndex ni);

    /*
     * Accessors
     */
    inline virtual bool isEmpty() const;
    inline virtual bool isAcyclic() const;
    inline virtual int getNumArcs() const;
    inline virtual int getNumNodes() const;

    inline virtual lbGraphBase const* getGraph() const;
    inline virtual arcIndexVec const getArcIndices() const;
    inline virtual nodeIndexVec const getNodeIndices() const;

    inline virtual nodeIndexVec getNeighboringNodeIndices(nodeIndex ni) const;
    inline virtual arcIndexVec getNeighboringArcIndices(nodeIndex ni) const;
    inline virtual bool getRootedArcs(arcVec & arcs) const;

    inline virtual bool nodePresent(nodeIndex ni) const;
    inline virtual bool arcPresent(arcIndex ai) const;
    inline virtual bool varPresent(rVarIndex vi) const;

    /*
     * Print
     */
    inline virtual void print(ostream & out) const;

    /*
     * Debugging
     */

    inline virtual bool isValid() const;

  protected:

    inline bool addRootedComponent(map<int,int> & nodeMask, map<int,int> & arcMask,
					       queue<nodeIndex> &nodesToFollow,
					       arcVec & ordering) const;


    lbGraphBase const* _graph;
    map<int, int> _nodesPresent;
    map<int, int> _arcsPresent;
    map<int, int> _varsPresent;
  };

  inline void lbSubgraph::addVar(rVarIndex vi) {
    _varsPresent[vi] = vi;
  }

  inline bool lbSubgraph::varPresent(rVarIndex vi) const {
    return _varsPresent.find(vi) != _varsPresent.end();
  }

  inline void lbSubgraph::clear() {
    _nodesPresent.clear();
    _arcsPresent.clear();
    _varsPresent.clear();
  }

  inline void lbSubgraph::addArc (arcIndex ai) {
    arc a = _graph->getArc(ai);
    addNode(a.first);
    addNode(a.second);
    _arcsPresent[ai] = ai;
  }

  inline void lbSubgraph::addNode (nodeIndex ni) {
    _nodesPresent[ni] = ni;
  }

  inline bool lbSubgraph::isEmpty() const {
    return (_nodesPresent.size() == 0 &&
	    _arcsPresent.size() == 0);
  }

  inline int lbSubgraph::getNumArcs() const {
    return _arcsPresent.size();
  }

  inline int lbSubgraph::getNumNodes() const {
    return _nodesPresent.size();
  }
  
  inline lbGraphBase const* lbSubgraph::getGraph() const {
    return _graph;
  }

  inline arcIndexVec const lbSubgraph::getArcIndices() const {
    map<int, int>::const_iterator it = _arcsPresent.begin();

    arcIndexVec aiv;
    for (; it != _arcsPresent.end(); it++) {
      aiv.push_back((*it).first);
    }

    return aiv;
  }

  inline nodeIndexVec const lbSubgraph::getNodeIndices() const {
    map<int, int>::const_iterator it = _nodesPresent.begin();

    nodeIndexVec niv;
    for (; it != _nodesPresent.end(); it++) {
      niv.push_back((*it).first);
    }

    return niv;
  }
  
  inline bool lbSubgraph::isAcyclic() const {
    arcVec ordering;
    bool possible = getRootedArcs(ordering);
    return possible;
  }

  inline nodeIndexVec lbSubgraph::getNeighboringNodeIndices(nodeIndex ni) const {
    arcIndexVec subgraphNeighborArcs = getNeighboringArcIndices(ni);

    nodeIndexVec niv;
   
    for (uint i = 0; i < subgraphNeighborArcs.size(); i++) {
      arc a = _graph->getArc(subgraphNeighborArcs[i]);
      niv.push_back((a.first == ni) ? a.second : a.first);
    }

    return vecUnique(niv);
  }

  inline arcIndexVec lbSubgraph::getNeighboringArcIndices(nodeIndex ni) const {
    arcIndexVec neighborArcs = _graph->getNeighboringArcIndices(ni);
    arcIndexVec aiv;
    
    for (uint i = 0; i < neighborArcs.size(); i++) {
      if (arcPresent(neighborArcs[i])) {
	aiv.push_back(neighborArcs[i]);
      }
    }
    
    return aiv;
  }

  inline bool lbSubgraph::getRootedArcs(arcVec & ordering) const {
    assert (ordering.empty());

    map<int,int> nodeMask;
    map<int,int> arcMask;

    nodeIndexVec nodes = getNodeIndices();

    if (nodes.empty()) {
      return true;
    }

    queue<nodeIndex> nodesToFollow;
    nodesToFollow.push(nodes[0]);

    bool success = true;
    while (!nodesToFollow.empty()) {
      if (!addRootedComponent(nodeMask, arcMask, nodesToFollow, ordering)) {
	success = false;
      }

      for (uint i = 0; i < nodes.size(); i++) {
	if (nodeMask.find(nodes[i]) == nodeMask.end()) {
	  // Found node we haven't looked at.
	  nodesToFollow.push(nodes[i]);
	  break;
	}
      }
    }

    // Orient ordering so that we progress to root as we go along.
    reverse (ordering.begin(), ordering.end());
    return success;
  }

  inline bool lbSubgraph::addRootedComponent(map<int,int> & nodeMask, map<int,int> & arcMask,
					     queue<nodeIndex> &nodesToFollow,
					     arcVec & ordering) const {	     
    bool success = true;

    while (!nodesToFollow.empty()) {
      nodeIndex ni = nodesToFollow.front();
      nodesToFollow.pop();
      nodeMask[ni] = ni;

      arcIndexVec neighborArcs = getNeighboringArcIndices(ni);
      bool neighborFound = false;
      for (uint i = 0; i < neighborArcs.size(); i++) {
	arc a = _graph->getArc(neighborArcs[i]);
	nodeIndex curNeighbor = (a.first == ni) ? a.second : a.first;

	if (arcMask.find(neighborArcs[i]) != arcMask.end()) {
	  if (neighborFound) {
	    success = false;
	  }
	  
	  neighborFound = true;
	}
	else {
	  arcMask[neighborArcs[i]] = neighborArcs[i];
	  ordering.push_back(arc(curNeighbor, ni));
	  nodesToFollow.push(curNeighbor);
	}
      }
    }

    return success;
  }

  inline bool lbSubgraph::nodePresent(nodeIndex ni) const {
    return _nodesPresent.find(ni) != _nodesPresent.end();
  }

  inline bool lbSubgraph::arcPresent(arcIndex ai) const {
    return _arcsPresent.find(ai) != _arcsPresent.end();
  }
  
  inline void lbSubgraph::print(ostream & out) const {
    out << "Printing subgraph: " << endl;
    out << "Num Arcs: " << getNumArcs() << endl;
    out << "Num Nodes: " << getNumNodes() << endl << endl;

    out << "Node Mask: ";
    //_nodesPresent.print(out);
    out << endl;

    out << "Arc Mask: ";
    //_arcsPresent.print(out);
    out << endl;

    arcIndexVec aiv = getArcIndices();
    for (uint i = 0; i < aiv.size(); i++) {
      out << _graph->getArc(aiv[i]).first << " -- " << _graph->getArc(aiv[i]).second << endl;
    }

    out << "Graph over which the subgraph is defined: " << endl;
    _graph->print(out);
  }

  inline bool lbSubgraph::isValid() const {
    return true;
  }
};

#endif
