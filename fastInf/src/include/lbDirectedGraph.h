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

// lbDirectedGraph.h
// Author: Ian McGraw

/*
 * Here we inherit most of our functionality from the undirected graph
 * call lbGraphBase.  Now, however, we reinterpret the arc (x, y) to
 * mean that there exists an arc FROM x pointing TO y.
 */

#ifndef _LB_DIRECTED_GRAPH_H_
#define _LB_DIRECTED_GRAPH_H_

#include <lbGraphBase.h>
#include <queue>

namespace lbLib {
  typedef pair<nodeIndexVec, arcIndexVec> incomingData;
  typedef pair<nodeIndexVec, arcIndexVec> outgoingData;
  typedef enum { ARC_DIR_OUT, ARC_DIR_IN, ARC_DIR_ANY } ArcDirection;

  inline bool directedArcsEqual(arc a, arc b) {
    return (a.first == b.first && a.second == b.second);
  }

  class lbDirectedGraph : public lbGraphBase {

  public:

    /*
     * Constructors and Destructors
     */
    lbDirectedGraph();
    lbDirectedGraph(int numNodes, arcVec const& arcVec);
    explicit lbDirectedGraph(lbDirectedGraph const&);
    ~lbDirectedGraph();

    /*
     * Mutators
     */
    inline virtual nodeIndex addNode();
    inline virtual arcIndex addArc(nodeIndex nfrom, nodeIndex nto);
    inline virtual void removeArc(arcIndex ai);
    inline virtual void removeNode(nodeIndex ai);
    
    /*
     * Accessors
     */
    inline virtual bool isEmpty() const;
    inline virtual bool isAcyclic() const;
    inline virtual bool isMultiGraph() const;
    inline virtual bool arcExists(nodeIndex from, nodeIndex to) const;

    inline virtual nodeIndexVec const& getOutgoingNodeIndices(nodeIndex ni) const;
    inline virtual arcIndexVec const & getOutgoingArcIndices(nodeIndex ni) const;

    inline virtual nodeIndexVec const& getIncomingNodeIndices(nodeIndex ni) const;
    inline virtual arcIndexVec const& getIncomingArcIndices(nodeIndex ni) const;

    // Fills niv with sort. Returns false if not possible (cyclic).
    inline virtual bool getTopologicalSort(nodeIndexVec & niv) const;

    // Get the reachable nodes following arcs of a particular direction.
    // The node ni is itself reachable from ni by default.
    inline virtual nodeIndexVec getReachable(nodeIndex ni, ArcDirection ad, bool includeSelf = false) const;

    /* 
     * Printing.
     */
    inline virtual void print(ostream & out) const;

    /*
     * Debugging
     */
    inline virtual bool isValid() const;

  protected:

    inline int findIncomingIndexOf(arcIndex ai);
    inline int findOutgoingIndexOf(arcIndex ai);
    
    vector<incomingData> _incomingCache;
    vector<outgoingData> _outgoingCache;
  };

  inline bool lbDirectedGraph::isEmpty() const {
    return (lbGraphBase::isEmpty() && _incomingCache.empty() && _outgoingCache.empty());
  }

  inline nodeIndex lbDirectedGraph::addNode() {
    nodeIndex ni = lbGraphBase::addNode();

    _outgoingCache.push_back(outgoingData());
    _incomingCache.push_back(incomingData());
    return ni;
  }

  inline arcIndex lbDirectedGraph::addArc(nodeIndex nfrom, nodeIndex nto) {
    assert(nodeExists(nfrom) && nodeExists(nto));

    arcIndex ai = lbGraphBase::addArc(nfrom, nto);
    
    _outgoingCache[nfrom].first.push_back(nto);
    _outgoingCache[nfrom].second.push_back(ai);

    _incomingCache[nto].first.push_back(nfrom);
    _incomingCache[nto].second.push_back(ai);

    return ai;
  }

  inline void lbDirectedGraph::removeArc(arcIndex ai) {
    arc a = getArc(ai);
    int indexOutgoing = findOutgoingIndexOf(ai);
    int indexIncoming = findIncomingIndexOf(ai);

    assert(indexOutgoing != -1 && indexIncoming != -1);

    nodeIndexVec & outgoingNodes = _outgoingCache[a.first].first;
    arcIndexVec & outgoingArcs = _outgoingCache[a.first].second;
    nodeIndexVec & incomingNodes = _incomingCache[a.second].first;
    arcIndexVec & incomingArcs = _incomingCache[a.second].second;

    outgoingNodes.erase(outgoingNodes.begin() + indexOutgoing);
    outgoingArcs.erase(outgoingArcs.begin() + indexOutgoing);
    incomingNodes.erase(incomingNodes.begin() + indexIncoming);
    incomingArcs.erase(incomingArcs.begin() + indexIncoming);
    lbGraphBase::removeArc(ai);

	uint i;
    for (i = 0; i < _outgoingCache.size(); i++) {
      arcIndexVec & outgoingArcIndices = _outgoingCache[i].second;
      
      for (uint j = 0; j < outgoingArcIndices.size(); j++) {
	if (outgoingArcIndices[j] > ai) {
	  outgoingArcIndices[j] = outgoingArcIndices[j] - 1;
	}
      }
    }

    for (i = 0; i < _incomingCache.size(); i++) {
      arcIndexVec & incomingArcIndices = _incomingCache[i].second;
      
      for (uint j = 0; j < incomingArcIndices.size(); j++) {
	if (incomingArcIndices[j] > ai) {
	  incomingArcIndices[j] = incomingArcIndices[j] - 1;
	}
      }
    }
  }

  inline void lbDirectedGraph::removeNode(nodeIndex ni) {
    lbGraphBase::removeNode(ni);

    _outgoingCache.erase(_outgoingCache.begin() + ni);
    _incomingCache.erase(_incomingCache.begin() + ni);

	uint i;
    for (i = 0; i < _outgoingCache.size(); i++) {
      nodeIndexVec & outgoing = _outgoingCache[i].first;
      
      for (uint j = 0; j < outgoing.size(); j++) {
	if (outgoing[j] > ni) {
	  outgoing[j] = outgoing[j] - 1;
	}
      }
    }

    for (i = 0; i < _incomingCache.size(); i++) {
      nodeIndexVec & incoming = _incomingCache[i].first;
      
      for (uint j = 0; j < incoming.size(); j++) {
	if (incoming[j] > ni) {
	  incoming[j] = incoming[j] - 1;
	}
      }
    }
  }

  inline bool lbDirectedGraph::isAcyclic() const {
    nodeIndexVec sort;
    bool possible = getTopologicalSort(sort);
    return possible;
  }

  inline int lbDirectedGraph::findIncomingIndexOf(arcIndex ai) {
    arcIndexVec const& incomingArcs = _incomingCache[getArc(ai).second].second;
    
    for (uint index = 0; index < incomingArcs.size(); index++) {
      if (ai == incomingArcs[index]) {
	return index;
      }
    }

    return -1;
  }

  inline int lbDirectedGraph::findOutgoingIndexOf(arcIndex ai) {
    arcIndexVec const& outgoingArcs = _outgoingCache[getArc(ai).first].second;
    
    for (uint index = 0; index < outgoingArcs.size(); index++) {
      if (ai == outgoingArcs[index]) {
	return index;
      }
    }

    return -1;
  }

  inline bool lbDirectedGraph::arcExists(nodeIndex from, nodeIndex to) const {
    arc newArc(from, to);

    for (arcIndex aii = 0; (uint) aii < _arcs.size(); aii++) {
      if (directedArcsEqual(_arcs[aii], newArc)) {
	return true;
      }
    }

    return false;
  }

  inline nodeIndexVec const& lbDirectedGraph::getOutgoingNodeIndices(nodeIndex ni) const {
    assert(nodeExists(ni));
    return _outgoingCache[ni].first;
  }

  inline arcIndexVec const & lbDirectedGraph::getOutgoingArcIndices(nodeIndex ni) const {
    assert(nodeExists(ni));
    return _outgoingCache[ni].second;    
  }
  
  inline nodeIndexVec const& lbDirectedGraph::getIncomingNodeIndices(nodeIndex ni) const {
    assert(nodeExists(ni));
    return _incomingCache[ni].first;    
  }

  inline arcIndexVec const & lbDirectedGraph::getIncomingArcIndices(nodeIndex ni) const {
    assert(nodeExists(ni));
    return _incomingCache[ni].second;    
  }

  inline bool lbDirectedGraph::getTopologicalSort(nodeIndexVec & niv) const {
    assert(niv.empty());

    if (_numNodes == 0) {
      return true;
    }

    vector<bool> nodeMask(_numNodes, false);

    bool nodesLeft = true;

    while (nodesLeft) {
      int foundUnmarkedHighestAncestor = false;

	  nodeIndex ni;
      for (ni = 0; ni < _numNodes; ni++) {
	if (nodeMask[ni] == true) {
	  continue;
	}

	// Here we have an unmarked node ni.
	nodeIndexVec const& incomingIndices = _incomingCache[ni].first;
	int foundUnmarkedIncoming = false;
	
	for (uint i = 0; i < incomingIndices.size(); i++) {
	  if (nodeMask[incomingIndices[i]] == false) {
	    foundUnmarkedIncoming = true;
	    break;
	  }
	}
	
	if (foundUnmarkedIncoming) {
	  continue;
	}

	// Here we know ni is a highest unmarked ancestor
	// because we didn't find an unmarked incoming
	niv.push_back(ni);
	nodeMask[ni] = true;
	foundUnmarkedHighestAncestor = true;
	break;
      }

      nodesLeft = false;
      for (ni = 0; ni < _numNodes; ni++) {
	if (nodeMask[ni] == false) {
	  nodesLeft = true;
	  break;
	}
      }

      if (nodesLeft && !foundUnmarkedHighestAncestor) {
	return false;
      }
    }

    return true;

  }

  inline nodeIndexVec lbDirectedGraph::getReachable(nodeIndex ind, 
						    ArcDirection ad, 
						    bool includeSelf) const {
    if (ad == ARC_DIR_ANY) {
      /* Might want to implement getReachable in base class and just call it here
	 for this particular case. */
      NOT_IMPLEMENTED_YET;
    }

    assert(nodeExists(ind));
    vector<bool> nodeMask(_numNodes, false);
    queue<nodeIndex> nodesToFollow;
    nodesToFollow.push(ind);
    
    nodeIndexVec reachable;
    if (includeSelf) {
      reachable.push_back(ind);
    }
    nodeMask[ind] = true;

    while (!nodesToFollow.empty()) {
      nodeIndex ni = nodesToFollow.front();
      nodesToFollow.pop();

      nodeIndexVec nextLevel;

      assert (ad == ARC_DIR_IN || ad == ARC_DIR_OUT);
      if (ad == ARC_DIR_IN) {
	nextLevel = _incomingCache[ni].first;
      }
      else {
	nextLevel = _outgoingCache[ni].first;
      }

      for (uint i = 0; i < nextLevel.size(); i++) {
	if (nodeMask[nextLevel[i]] == false) {
	  nodeMask[nextLevel[i]] = true;
	  reachable.push_back(nextLevel[i]);
	  nodesToFollow.push(nextLevel[i]);
	}
      }
    }

    return reachable;
  }

  inline void lbDirectedGraph::print(ostream & out) const {
    out << "Printing DirectedGraph, thus... " << endl;
    lbGraphBase::print(out);

    out << "Is a DAG: " << isAcyclic() << endl;
    out << "Is a multigraph: " << isMultiGraph() << endl;

    for (arcIndex ai = 0; ai < getNumArcs(); ai++) {
      out << "Arc " << ai << ": ";
      out << getArc(ai).first << "-->";
      out << getArc(ai).second << endl;
    }

    out << endl;
  }


  inline bool lbDirectedGraph::isMultiGraph() const {
    for (nodeIndex ni = 0; ni < getNumNodes(); ni++) {
      nodeIndexVec const& outgoingNeighbors = _outgoingCache[ni].first;
      
      for (uint i = 0; i < outgoingNeighbors.size(); i++) {
	for (uint j = 0; j < outgoingNeighbors.size(); j++) {
	  if (i != j && outgoingNeighbors[i] == outgoingNeighbors[j]) {
	    return true;
	  }
	}
      }
    }
    
    return false;
  }

  inline bool lbDirectedGraph::isValid() const {
    if (!lbGraphBase::isValid()) {
      return false;
    }

    if ((uint) _numNodes != _incomingCache.size()) {
      return false;
    }

    if ((uint) _numNodes != _outgoingCache.size()) {
      return false;
    }
    
    // Make sure we have a one to one mapping from incoming + outgoing caching to arcs
    vector<bool> arcMask(_arcs.size(), 0);

    // Check incoming caching
	nodeIndex ind;
    for (ind = 0; (uint ) ind < _incomingCache.size(); ind++) {
      const incomingData & n = _incomingCache[ind];
      const nodeIndexVec & niv = n.first;
      const arcIndexVec & aiv = n.second;

      if (niv.size() != aiv.size()) {
	return false;
      }

      for (uint j = 0; j < niv.size(); j++) {
	nodeIndex ni = niv[j];
	arcIndex ai = aiv[j];

	if (ni < 0 || ni >= _numNodes) {
	  return false;
	}

	if (ai < 0 || (uint) ai >= _arcs.size()) {
	  return false;
	}
	
	arcMask[ai] = true;

	arc testArc(ni, ind);
	if (!directedArcsEqual(testArc, _arcs[ai])) {
	  // These nodes indices should be represented by this arc!
	  return false;
	}
      }
    }

    for (ind = 0; (uint ) ind < _neighborsCache.size(); ind++) {
      const outgoingData & cd = _outgoingCache[ind];
      const nodeIndexVec & niv = cd.first;
      const arcIndexVec & aiv = cd.second;

      if (niv.size() != aiv.size()) {
	return false;
      }

      for (uint j = 0; j < niv.size(); j++) {
	nodeIndex ni = niv[j];
	arcIndex ai = aiv[j];

	if (ni < 0 || ni >= _numNodes) {
	  return false;
	}

	if (ai < 0 || (uint) ai >= _arcs.size()) {
	  return false;
	}
	
	arcMask[ai] = true;

	arc testArc(ind, ni);
	if (!directedArcsEqual(testArc, _arcs[ai])) {
	  // These nodes indices should be represented by this arc!
	  return false;
	}
      }
    }

    // Here's where we ensure the mapping.
    for (uint i = 0; i < arcMask.size(); i++) {
      if (arcMask[i] == false) {
	return false;
      }
    }

    return true;
    
  }
};

#endif
