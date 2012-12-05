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

#ifndef _LB_BASIC_GRAPH__
#define _LB_BASIC_GRAPH__

#include <set>
#include <utility>
#include <ostream>
#include <list>
using namespace std;

#include <lbDefinitions.h>
using namespace lbLib;

namespace lbLib {
  
  class lbBasicGraph {
  public:

    typedef set<int> Clique;
    typedef set<pair<const Clique*,const Clique*> > CliqueEdges;

    typedef set<const Clique*> CliqueSet;
    typedef list<const Clique*> CliqueList;
    typedef vector<const Clique*> CliqueVector;
 
    /* Builds the graph (Note: assumes all data structurs are correct.
       i.e., does not check that adjacencyList is SYMMETRYIC or that no self-edges exist)
    */
    lbBasicGraph(set<int>** adjacencyList, cardVec* cards);

    /* copy ctor [builds the sub-graph induced by the subset of vertices of g (keepVertices)]
     */
    lbBasicGraph(lbBasicGraph const& g, const set<int>* keepVertices = NULL);

    virtual ~lbBasicGraph();
  
    /*Sets this graph = to lbBasicGraph g, and returns a pointer to the new this
      Does nothing if &g == this
    
      arguments: g - the graph to which to set this equal to
      returns: a pointer to this, after it was set = to g
    */
    lbBasicGraph& operator= (const lbBasicGraph& g);

    //creates a new, allocated adjacency list of size:
    static set<int>** createNewAdjacencyList(int size, bool allocateVertices = true);
    //deletes all elements from list (of size), and list itself:
    static void deleteAdjacencyList(set<int>** list, int size);

    //Return a new copy of this lbBasicGraph's adjacency list.
    set<int>** copyAdjacencyList() const;

    class vertexIterator {
      friend class lbBasicGraph;    
    public:
      vertexIterator();
      int operator*() const;
      vertexIterator& operator++();
      vertexIterator operator++(int);
      vertexIterator& operator--();
      vertexIterator operator--(int);    
      bool operator==(vertexIterator const& vi) const;
      inline bool operator!=(vertexIterator const& vi) const {return !(*this == vi);}
    
    private:
      int _currentVertex;
      const lbBasicGraph* _iteratelbBasicGraph;

      vertexIterator(lbBasicGraph const* iterateOver, int initialIndex = 0);
      void advanceCurrentVertex(int startFrom);
      void decrementCurrentVertex(int startFrom);
    };

    class neighborIterator {
      friend class lbBasicGraph;
    public:
      neighborIterator() {}
      int operator*() const {return *_inner;}
      neighborIterator& operator++() {++_inner; return *this;}
      neighborIterator operator++(int) {neighborIterator tmp(*this); ++(*this); return tmp;}
      bool operator==(neighborIterator const& vi) const {return _inner == vi._inner;}
      inline bool operator!=(neighborIterator const& vi) {return !(*this == vi);}
    
    private:
      set<int>::iterator _inner;
    
      neighborIterator(lbBasicGraph const* g, int vertIndex) {_inner = g->_adjacencyList[vertIndex]->begin();}
    };
    
    inline lbBasicGraph::vertexIterator verticesBegin() const;
    inline lbBasicGraph::vertexIterator verticesEnd() const;
  
    inline lbBasicGraph::neighborIterator neighbBegin(int v) const;
    inline lbBasicGraph::neighborIterator neighbEnd(int v) const;

    /*Adds a new vertex to this lbBasicGraph at addIndex, if it does not exist.
      If a vertex with this name already exists, then no new vertex is added.
    
      returns: an iterator pointing to the newly added vertex, or to the one that
      already exists in this lbBasicGraph.
      If addIndex is out-of-bounds, returns this->verticesEnd()
    */
    lbBasicGraph::vertexIterator addVertex (int addIndex);
  
    /*Removes the vertex removeIndex from the graph (and all incident edges),
      if such a vertex exists and returns true.  If removeIndex is already removed
      or is out-of-bounds, then nothing is done (returns false)       
    */
    bool removeVertex(int removeIndex);

    /*Returns true iff checkIndex is in-bounds and it is currently a vertex in this lbBasicGraph
     */
    bool hasVertex(int checkIndex) const;

    /*If v is currently a valid vertex in this lbBasicGraph, then removes v from lbBasicGraph (and all incident edges).
      Adds edges to make all of v's former neighbors a clique (i.e. ensures that for every pair
      of v's former neighbors that there is an edge between them).
      Returns a newly allocated Clique of v's neighbors indices [INCLUDING v].
      If v is not a vertex in the graph, returns NULL.
    */
    Clique* eliminateVertex(int v);

    inline unsigned int numVertices() const {return _numVertices;}

    /*Returns the number of neighbors of v, if it exists.
      If not, returns -1.
    */
    int numNeighbors(int v) const;
  
    /*Creates a new edge between the vertices v1, v2, or ensures
      that one exists already
      returns false iff error (if no such vertices exist currently)
    */
    bool addEdge(int v1, int v2);
  
    /*Returns true iff the undirected edge (v1, v2) exists in this graph
     */
    bool hasEdge(int v1, int v2) const;

    /*If the vertices v1, v2 exist and there is an edge between them,
      removes it and returns true.  Otherwise, does nothing and returns false.
    */
    bool removeEdge(int v1, int v2);

    /*Returns true iff this lbBasicGraph and g are the same graphs in the sense
      that all vertices are == and all edges in one graph are in the other, and vice versa.
    
      arguments: g - the lbBasicGraph to be compared to this
      returns: true iff this and g are graphically equivalent
    */
    bool operator==(const lbBasicGraph& g) const;  

    /*Returns -1 if v is invalid
     */
    int getStateSpaceCardinality(int v) const;

    /* Write the graph to the given stream.
     *
     * arguments: stream - stream to write the graph to.
     *            g - the graph to be written
     *
     * return: a reference to the given stream
     */
    friend ostream& operator<<(ostream& stream, lbBasicGraph const& g);

  protected:
    set<int>** _adjacencyList;
    cardVec* _cards;
    unsigned int _numVertices;

    /*Prints the graph to the given stream,
      and returns this stream.
    
      arguments: stream - the stream to which to print this
      returns: stream - after printing this graph to it*/
    virtual ostream& print(ostream& stream) const;

    //same as hasVertex, but no error checks
    inline bool hasVertexNoChecks(int v) const;

    bool isOutOfBounds(int index) const;

    //same as numNeighbors, but no checks
    inline int numNeighborsNoChecks(int v) const { return _adjacencyList[v]->size();}
  
    //same as hasEdge, but no error checks
    inline bool hasEdgeNoChecks(int v1, int v2) const;

  private:
    void deleteDataMembers();
    void copyOtherlbBasicGraph(const lbBasicGraph& g, const set<int>* subsetVertices = NULL);
  
    /*same as removeVertex, but no error checks
      [inserts neighbors of removed vertex into insertNeighbs, if != NULL]*/
    void removeVertexNoChecks(int removeIndex, Clique* insertNeighbs = NULL);

    //same as addEdge, but no error checks
    void addEdgeNoChecks(int v1, int v2);

    //same as removeEdge, but no error checks
    void removeEdgeNoChecks(int v1, int v2);

    //Copy the actual _adjacencyList data into newList:
    void copyAdjacencyListData(set<int>** newList) const;
  };

}

inline lbBasicGraph::vertexIterator lbBasicGraph::verticesBegin() const {
  return lbBasicGraph::vertexIterator(this);
}

inline lbBasicGraph::vertexIterator lbBasicGraph::verticesEnd() const {
  lbBasicGraph::vertexIterator end;
  end._iteratelbBasicGraph = this;
  return end;
}

inline lbBasicGraph::neighborIterator lbBasicGraph::neighbBegin(int v) const {  
  if (hasVertex(v)) {
    return lbBasicGraph::neighborIterator(this,v);
  }
  return lbBasicGraph::neighborIterator();
}

inline lbBasicGraph::neighborIterator lbBasicGraph::neighbEnd(int v) const {
  lbBasicGraph::neighborIterator end;
  if (hasVertex(v)) {
    end._inner = this->_adjacencyList[v]->end();
  }
  return end;
}

inline bool lbBasicGraph::hasVertexNoChecks(int v) const {
  return _adjacencyList[v] != NULL;
}

inline bool lbBasicGraph::hasEdgeNoChecks(int v1, int v2) const {
  /*_adjacencyList should always be symmetric, so both
    terms of the or should always have the same truth values
  */  
  return (_adjacencyList[v1]->find(v2) != _adjacencyList[v1]->end() ||
	  _adjacencyList[v2]->find(v1) != _adjacencyList[v2]->end());
}


#endif
