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

// lbGraphBase.h


/*!
 * A very general graph structure. This doesn't even specify whether the graph 
 * has self loops or if it is simple, etc. It's designed to be extended from so 
 * that more specific functionality can be implemented.  By default the functions
 * are implemented as if the graph were undirected.  This is so that it's easy
 * to cast directly to the underlying undirected graph of some more specialized
 * graph such as a DAG.  Note that this means that if we have the edge (0, 1) and
 * the edge (1, 0) we have a multi graph with two undirected arcs between 1 and 0.
 * Also this means that if we have a multi graph a node's neighbor indices might
 * include itself (twice in fact) and there may be other repititions as well. Thus
 * We have property checking functions such as isMultiGraph and containsSelfLoops,
 * for those that require a graph to have certain properties.
 \version  July 2009
 \author  Ian McGraw
*/

#ifndef _LB_GRAPH_BASE_
#define _LB_GRAPH_BASE_

#include <lbDefinitions.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>

using namespace std;

namespace lbLib {

  // Might want to move this to lbDefinitions.h if this class is worth 
  // keeping around for any serious amount of time.
  initIntegerStruct(nodeIndex);
  initIntegerStruct(arcIndex);
  initIntegerStruct(genericIndex);
  implementIntegerStruct(genericIndex);
  implementIntegerStruct(nodeIndex);
  implementIntegerStruct(arcIndex);

  typedef vector<nodeIndex> nodeIndexVec;
  typedef pair<nodeIndex, nodeIndex> arc;  /* 'edge' taken by spanning tree */
  typedef vector<arc> arcVec;
  typedef vector<arcIndex> arcIndexVec;
  typedef vector<arcIndexVec> arcIndexVecVec;
  typedef pair<nodeIndexVec, arcIndexVec> neighborData;

  implementIntegerStructVectorOps(nodeIndexVec);
  implementIntegerStructVectorOps(arcIndexVec);

  // A few useful operations on index vecs. (might also go to lbDefinitions.h)
  // All of these ensure that the indices returned are unique.
  inline nodeIndexVec nodeIndexVecUnique(nodeIndexVec const& v);
  inline nodeIndexVec nodeIndexVecMinus(nodeIndexVec const& v1, nodeIndexVec const& v2);
  inline nodeIndexVec nodeIndexVecIntersection(vector<int> const& v1, nodeIndexVec const& v2);
  inline nodeIndexVec nodeIndexVecUnion(nodeIndexVec const& v1, nodeIndexVec const&v2);

  inline bool undirectedArcsEqual(arc a, arc b) {
    return ((a.first == b.first && a.second == b.second) ||
	    (a.first == b.second && a.second == b.first));
  }

  struct lbWeightedArcIndex {
    bool operator<(const lbWeightedArcIndex &other) const {
      return weight < other.weight;
    }
    
    double weight;
    arcIndex ai;
  };
  
  inline void swallowComments(istream & in) {
    char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
    while (in.peek() != '@') {
      in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE);

      if (!in.good() || in.eof()) {
	break;
      }
    }
  }

  class lbGraphBase {
    public:

    /*
     * Constructors and Destructors
     */
    lbGraphBase(); /* Empty graph */
    lbGraphBase(int numNodes, arcVec const& arcVec);
    explicit lbGraphBase(lbGraphBase const&);
    lbGraphBase& operator=(lbGraphBase const&);
    virtual ~lbGraphBase();

    /*
     * Mutators
     */
    inline virtual void readGraph(istream & in);
    inline virtual arcIndex addArc(nodeIndex a, nodeIndex b);
    inline virtual nodeIndex addNode();
    inline virtual void removeArc(arcIndex ai);
    inline virtual void removeNode(nodeIndex ni);
    inline virtual void removeSpecifiedNodes(nodeIndexVec & niv);
    inline virtual void removeSpecifiedArcs(arcIndexVec & aiv);

    /*
     * Accessors
     */
    inline virtual bool isEmpty() const;
    inline virtual int getNumNodes() const;
    inline virtual int getNumArcs() const;

    inline virtual arc getArc(arcIndex ai) const;
    inline virtual arcIndexVec getArcIndices(nodeIndex na, nodeIndex nb) const;
    inline virtual nodeIndexVec const& getNeighboringNodeIndices(nodeIndex ni) const;
    inline virtual arcIndexVec const & getNeighboringArcIndices(nodeIndex ni) const;

    inline virtual bool containsSelfLoops() const;
    inline virtual bool nodeExists(nodeIndex ni) const;

    inline virtual bool isConnected() const;
    inline virtual bool isAcyclic() const;
    inline virtual bool isMultiGraph() const;
    inline virtual bool arcExists(nodeIndex a, nodeIndex b) const;

    /*
     * Print
     */
    inline virtual void print(ostream & out) const;
    inline virtual void printFormatted(ostream & out) const;

    /*
     * Debugging
     */

    /* 
     * The isValid method can be used both for debugging the implementation
     * of the graph itself, and for those programs that use the graph.
     * Standard usage would be to override isValid for specifics of the
     * subclass and then call isValid for the base class as well so that
     * we guarantee a graph is valid at all levels.  This can get expensive
     * so it might be wise to add an isProbablyValid when the code needs
     * to be tight.
     */
    inline virtual bool isValid() const;

  protected:

    /*
     * Reading helpers
     */

    const static string SIZE_DELIM;
    const static string ARCS_DELIM;
    const static string END_DELIM;

    inline void readSize(istream & in);
    inline void readArcs(istream & in);
    inline int findNeighborIndexOfFirst(arcIndex ai);
    inline int findNeighborIndexOfSecond(arcIndex ai);

    int _numNodes;

    /* Maintain arcs multiple ways for easy access. */
    vector<neighborData> _neighborsCache;
    arcVec _arcs;
  };

  inline void lbGraphBase::readGraph(istream & in) {
    swallowComments(in);
    readSize(in);
    swallowComments(in);
    readArcs(in);
  }

  inline void lbGraphBase::readSize(istream & in) {
    char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);

    in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE);
    assert(string(buffer.get()) == SIZE_DELIM);

    in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE);
    string str(buffer.get());
    istringstream iss(str);
    
    int numNodes;
    iss >> numNodes;

    for (int i = 0; i < numNodes; i++) {
      addNode(); /* Calling virtual functions if need be */
    }

    in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE);
    assert(string(buffer.get()) == END_DELIM);
  }

  inline void lbGraphBase::readArcs(istream & in) {
    char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);

    in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE);
    assert(string(buffer.get()) == ARCS_DELIM);

    while (true) {
      assert(in.good() && !in.eof());
      in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE);
      string str(buffer.get());
      
      if (str == END_DELIM) {
	break;
      }

      int na, nb;
      istringstream iss(str);
      iss >>  na;
      iss >>  nb;
      
      addArc(na, nb);
    }    
  }

  inline arcIndex lbGraphBase::addArc(nodeIndex na, nodeIndex nb) {
    arc newArc(na, nb);

    assert(nodeExists(na));
    assert(nodeExists(nb));

    _arcs.push_back(newArc);
    arcIndex ai = (int) (_arcs.size() - 1);

    _neighborsCache[na].first.push_back(nb);
    _neighborsCache[na].second.push_back(ai);

    _neighborsCache[nb].first.push_back(na);
    _neighborsCache[nb].second.push_back(ai);

    return ai;
  }

  inline nodeIndex lbGraphBase::addNode() {
    _numNodes++;
    _neighborsCache.push_back(neighborData());
    return (_numNodes-1);
  }

  inline bool lbGraphBase::isConnected() const {
    if (_arcs.empty()) {
      return _numNodes <= 1;
    }

    vector<bool> nodeMask(_numNodes, false);
    
    queue<nodeIndex> nodesToFollow;
    nodesToFollow.push(_arcs[0].first);

    while (!nodesToFollow.empty()) {
      nodeIndex ni = nodesToFollow.front();
      nodesToFollow.pop();
      nodeMask[ni] = true;

      for (uint i = 0; i < _neighborsCache[ni].first.size(); i++) {
	nodeIndex curNeighbor = _neighborsCache[ni].first[i];
	if (!nodeMask[curNeighbor]) {
	  nodesToFollow.push(curNeighbor);
	}
      }
    }

    bool connected = true;
    for (uint i = 0; i < nodeMask.size(); i++) {
      if (!nodeMask[i]) {
	printf ("Not connected: %d\n", i);
	connected = false;
      }

    }

    return connected;
  }


  inline bool lbGraphBase::isAcyclic() const {
    NOT_IMPLEMENTED_YET;
	return false;
  }

  inline bool lbGraphBase::isMultiGraph() const {
    arcIndex aii, aij;

    for (aii = 0; (uint) aii < _arcs.size(); aii++) {
      for (aij = 0; (uint) aij < _arcs.size(); aij++) {
	if (aii != aij) {
	  if (undirectedArcsEqual(_arcs[aii], _arcs[aij])) {
	    return true;
	  }
	}
      }
    }
    
    return false;
  }

  inline bool lbGraphBase::arcExists(nodeIndex na, nodeIndex nb) const {
    // Regardless of direction. (thus just check neighbors)
    if (!nodeExists(na) || !nodeExists(nb)) {
      return false;
    }

    nodeIndexVec const& neigh = _neighborsCache[na].first;
    
    for (uint i = 0; i < neigh.size(); i++) {
      if (neigh[i] == nb) {
	return true;
      }
    }

    return false;
  }

  inline void lbGraphBase::removeArc(arcIndex ai) {
    arc a = getArc(ai);

    int indexFirst = findNeighborIndexOfFirst(ai);
    int indexSecond = findNeighborIndexOfSecond(ai);

    assert(indexFirst != -1 && indexSecond != -1);

    nodeIndexVec & neighborNodesFirst = _neighborsCache[a.first].first;
    arcIndexVec & neighborArcsFirst = _neighborsCache[a.first].second;
    nodeIndexVec & neighborNodesSecond = _neighborsCache[a.second].first;
    arcIndexVec & neighborArcsSecond = _neighborsCache[a.second].second;
    
    neighborNodesFirst.erase(neighborNodesFirst.begin() + indexFirst);
    neighborArcsFirst.erase(neighborArcsFirst.begin() + indexFirst);
    neighborNodesSecond.erase(neighborNodesSecond.begin() + indexSecond);
    neighborArcsSecond.erase(neighborArcsSecond.begin() + indexSecond);
    _arcs.erase(_arcs.begin() + ai);

    // Shift arc indices above ai down by one.
    for (uint i = 0; i < _neighborsCache.size(); i++) {
      arcIndexVec & neighborArcIndices = _neighborsCache[i].second;

      for (uint j = 0; j < neighborArcIndices.size(); j++) {
	if (neighborArcIndices[j] > ai) {
	  neighborArcIndices[j] = neighborArcIndices[j] - 1;
	}
      }
    }
  }

  inline int lbGraphBase::findNeighborIndexOfFirst(arcIndex ai) {
    nodeIndex ni = getArc(ai).first;

    arcIndexVec const& neighborArcs = _neighborsCache[ni].second;
    
    for (uint index = 0; index < neighborArcs.size(); index++) {
      if (ai == neighborArcs[index]) {
	return index;
      }
    }

    return -1;
  }

  inline int lbGraphBase::findNeighborIndexOfSecond(arcIndex ai) {
    nodeIndex ni = getArc(ai).second;

    arcIndexVec const& neighborArcs = _neighborsCache[ni].second;
    
    for (uint index = 0; index < neighborArcs.size(); index++) {
      if (ai == neighborArcs[index]) {
	return index;
      }
    }

    return -1;
  }

  inline void lbGraphBase::removeNode(nodeIndex ni) {
    assert (ni >= 0 && ni < _numNodes);

    arcIndexVec const& aiv = getNeighboringArcIndices(ni); 
    
    // Removes all arcs associated with this node.
    while (aiv.size() > 0) {
      removeArc(aiv[0]);
    }

    _neighborsCache.erase(_neighborsCache.begin() + ni);
    
    // Since we've essentially decremented all the node indices above
    // ni we must correct the arcs and neighbor cachings.
    uint i;
	for (i = 0; i < _arcs.size(); i++) {
      if (_arcs[i].first > ni) {
	_arcs[i].first = _arcs[i].first - 1;
      }
      
      if (_arcs[i].second > ni) {
	_arcs[i].second = _arcs[i].second - 1;
      }
    }
    
    for (i = 0; i < _neighborsCache.size(); i++) {
      nodeIndexVec & neighbors = _neighborsCache[i].first;
      
      for (uint j = 0; j < neighbors.size(); j++) {
	if (neighbors[j] > ni) {
	  neighbors[j] = neighbors[j] - 1;
	}
      }
    }

    _numNodes--;
  }
  
  inline void lbGraphBase::removeSpecifiedNodes(nodeIndexVec & niv) {
    sort(niv.begin(), niv.end());
    
    // Go in descending order so we don't renumber things ahead of
    // ourselves.
    for (int i = (int) niv.size() - 1; i >= 0; i--) {
      removeNode(niv[i]);
    }
  }

  inline void lbGraphBase::removeSpecifiedArcs(arcIndexVec & aiv) {
    sort(aiv.begin(), aiv.end());
    
    // Go in descending order so we don't renumber things ahead of
    // ourselves.
    for (int i = (int) aiv.size() - 1; i >= 0; i--) {
      removeArc(aiv[i]);
    }
  }

  inline bool lbGraphBase::isEmpty() const {
    return (_numNodes == 0 && _arcs.empty() && _neighborsCache.empty());
  }

  inline int lbGraphBase::getNumNodes() const {
    return _numNodes;
  }

  inline int lbGraphBase::getNumArcs() const {
    return (int) _arcs.size();
  }

  inline arc lbGraphBase::getArc(arcIndex ai) const {
    assert(ai >= 0 && (uint) ai < _arcs.size());
    return _arcs[ai];
  }

  inline arcIndexVec lbGraphBase::getArcIndices(nodeIndex na, nodeIndex nb) const {
    /* The vectorness is for multi graphs.  That is there could be more than one
       arc between nodes na and nb.  Here we'd return a vector of their indices */
    NOT_IMPLEMENTED_YET;
arcIndexVec vec;
return vec;
  }

  inline nodeIndexVec const& lbGraphBase::getNeighboringNodeIndices(nodeIndex ni) const {
    assert(nodeExists(ni));
    return _neighborsCache[ni].first;
  }

  inline arcIndexVec const& lbGraphBase::getNeighboringArcIndices(nodeIndex ni) const {
    assert(nodeExists(ni));
    return _neighborsCache[ni].second;
  }

  inline bool lbGraphBase::containsSelfLoops() const {
    for (arcIndex ai = 0; (uint) ai < _arcs.size(); ai++) {
      if (_arcs[ai].first == _arcs[ai].second) {
	return true;
      }
    }
    return false;
  }

  inline bool lbGraphBase::nodeExists(nodeIndex ni) const {
    assert ((uint) _numNodes == _neighborsCache.size());
    return (ni >= 0 && ni < _numNodes);
  }

  inline void lbGraphBase::print(ostream & out) const {
    out << "Printing GraphBase" << endl << endl;

    out << "Node count: " << _numNodes << "." << endl;
    out << "Arc count: " << _arcs.size() << "." << endl;
    
    for (uint i = 0; i < _arcs.size(); i++) {
      out << "Arc " << i << ": ";
      out << getArc(i).first;
      out << "--";
      out << getArc(i).second;
      out << endl;
    }

    for (nodeIndex ni = 0; (uint) ni < _neighborsCache.size(); ni++) {
      out << "Neighbors for " << ni << ": ";

      for (uint i = 0; i < _neighborsCache[ni].first.size(); i++) {
	out << _neighborsCache[ni].first[i] << " ";
      }

      out << endl;
    }
    out << endl;
  }

  inline void lbGraphBase::printFormatted(ostream & out) const {
    out << "# Format: Number of nodes" << endl;
    out << SIZE_DELIM << endl;
    out << _numNodes << endl;
    out << END_DELIM << endl << endl;

    out << "# Format: fromNode \\t toNode" << endl;
    out << ARCS_DELIM << endl;
    for (uint i = 0; i < _arcs.size(); i++) {
      out << _arcs[i].first << " " << _arcs[i].second << endl;
    }
    out << END_DELIM << endl;
    out << endl;
  }

  inline bool lbGraphBase::isValid() const {
    if ((uint) _numNodes != _neighborsCache.size()) {
      return false;
    }

    // Make sure we have a two to one mapping from neighbor caching to arcs
    vector<short> arcMask(_arcs.size(), 0);

    for (nodeIndex ind = 0; (uint ) ind < _neighborsCache.size(); ind++) {
      const neighborData & n = _neighborsCache[ind];
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
	
	arcMask[ai]++;

	arc testArc(ni, ind);
	if (!undirectedArcsEqual(testArc, _arcs[ai])) {
	  // These nodes indices should be represented by this arc!
	  return false;
	}
      }
    }
    
    // Here's where we ensure the mapping.
    for (uint i = 0; i < arcMask.size(); i++) {
      if (arcMask[i] != 2) {
	return false;
      }
    }

    return true;
  }

};

#endif
