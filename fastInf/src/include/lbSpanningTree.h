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


#ifndef _Loopy__Spanning__Tree
#define _Loopy__Spanning__Tree

#include <lbDefinitions.h>
#include <lbRandomProb.h>
#include <set>
#include <queue>
#include <stack>

using namespace std;

namespace lbLib {

  class lbSpanningTree;
  class lbPropagationInference;
  typedef int node;
  //initIntegerStruct(node);
  
  struct edge {
    edge(node node1,node node2){
      _node1=node1;
      _node2=node2;
    }
    
    inline node first() const{
      return _node1;
    }
    
    inline node second() const{
      return _node2;
    }
    
    /* The following operator and ltedge are used for set operations, which
     * are useful when the graph an inference object is using gets updated
     * and inference needs to know which messages between which cliques to
     * update.
     */
    friend inline bool operator== (const edge & a, const edge & b) {
      if ((a.first() == b.first() && a.second() == b.second()) ||
	  (a.second() == b.first() && a.first() == b.second())) {
	return true;
      }
      return false;
    }
    friend inline bool operator!= (const edge & a, const edge & b) {
      return !(a == b);
    }
    
    node _node1,_node2;
  };

  /* A struct for comparing to edges (weak ordering on edges). */
  struct ltedge {
    bool operator() (const edge & a, const edge & b) const {
      if (a == b) return false;
      if (a.first() < b.first()) {
	return true;
      } else if (a.first() == b.first()) {
	return a.second() < b.second();
      }
      return false;
    }
  };

  typedef safeVec<edge> edgeSet;
  //  typedef safeVec<node> nodesSet;
  typedef cliquesVec nodesSet;


  /* A function that returns the score of an edge that may be added to 
   * a spanning tree, using auxiliary data AUX.
   * As a note, one should NOT call computeMessage within this function,
   * because the scopes of all the local variables will not have been
   * updated, and there is the chance that this edge scoring may be called
   * on an edge we are considering adding to a graph. */
  typedef double edgeScoreFunction (const edge & e, void * aux);
  
  class lbSpanningTree {
  public:
    
    lbSpanningTree(edgeSet const& edges,nodesSet const& vertices,int maxNode);
    
    ~lbSpanningTree();
    
    void buildSpanningTree();
    
    edgeSet const& getSpanningTree();
    
    void printTree(ostream& out) const;
    
    /* Removes the edge with endpoints at e.first() and e.second()
     * (order does not matter). */
    void removeEdge(const edge & e);
    
    /* Adds an edge to the tree and the edge set. Automatically
     * re-establishes spanning tree properties. If it removes an
     * edge, will return the edge it removed. Otherwise, returns an
     * edge with -1, -1 as its nodes. */
    edge addEdge(const edge & e);
    
    /* Method to set the maximum node possible in this spanning tree (usually
     * the number of cliques in the graph over which it may build a tree). */
    void setMaxNode(int numCliques);
    /* Method that returns the maximum indexed node possible in the spanning tree. */
    int getMaxNode() { return _maxNode; }
    
    /* Method to set the function used to select edges (defaults to random) along
     * with specificing data that the function may need. */
    void setEdgeScoreFunction (edgeScoreFunction * f, void * auxiliary);
    
  protected:
    
    const set<int> & getAdjacent(int node) const { return _adjacencyList[node]; }
    
    /* Uses the selector given as a metric to remove an edge from
     * the given spanning tree if there is a cycle. Returns the edge
     * that was removed if there was one, edge(-1, -1) if not. Will
     * remove the lowest scoring edge. */
    edge tryRemoveEdge(lbSpanningTree & tree, const edge & add);
    
    /* Using the edges on a cycle starting at predecessors[index], returns
     * the lowest weight cycle according to the scoring function we have. */
    edge lowEdgeOnCycle(int * predecessors, int index);
    
  private:
    
    edgeSet _edges;
    // Just a list of the node indices in this graph (aka the cliqIndexes)
    nodesSet _nodes;
    // The set of pairs of integers 
    edgeSet _tree;
    
    int _maxNode;
    /* A vector of nodes used to maintain an adjacency list representation of
     * the tree entire graph. */
    vector<set<int> > _adjacencyList;
    
    double edgeScore(edge e);
    
    // Data used to select edges to remove from a spanning tree in case of cycle
    void * _edgeScoreAux;
    edgeScoreFunction * _edgeScorer;
    static lbRandomGenerator _randomGenerator;

  };

};

#endif
