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

#include <lbBasicGraph.h>
using namespace lbLib;

#define NON_EXISTENT_VERTEX -1

lbBasicGraph::lbBasicGraph(set<int>** adjacencyList, cardVec* cards)
  : _adjacencyList(adjacencyList), _cards(cards), _numVertices(cards->size()) {
  for (int i = 0; i < static_cast<int>(_cards->size()); ++i) {
    if (_adjacencyList[i] == NULL) {
      --_numVertices;
    }
  }
}

lbBasicGraph::lbBasicGraph(const lbBasicGraph& g, const set<int>* keepVertices)
  : _numVertices(g._numVertices) {
  copyOtherlbBasicGraph(g,keepVertices);
}

lbBasicGraph::~lbBasicGraph() {
  this->deleteDataMembers();
}

lbBasicGraph& lbBasicGraph::operator= (const lbBasicGraph& g) {
  if (&g != this) {//no need to do anything if this is already g
    deleteDataMembers();
    _numVertices = g._numVertices;
    copyOtherlbBasicGraph(g);    
  }
  return *this;
}

void lbBasicGraph::copyOtherlbBasicGraph(const lbBasicGraph& g, const set<int>* subsetVertices) {
  _cards = new cardVec(*(g._cards));

  //copy _adjacencyList:
  _adjacencyList = lbBasicGraph::createNewAdjacencyList(_cards->size(), false);
  
  if (subsetVertices == NULL) {//copy all vertices, edges
    g.copyAdjacencyListData(_adjacencyList);
  }
  else {//only keep the vertices & edges induced by susbset of vertices
    _numVertices = 0;
    for (set<int>::const_iterator it = subsetVertices->begin(); it != subsetVertices->end(); ++it) {
      int currentResidue = *it;
      if (this->isOutOfBounds(currentResidue)) {//ignore bad indices
	continue;
      }
      ++_numVertices;
      _adjacencyList[currentResidue] = new set<int>();
      for (lbBasicGraph::neighborIterator neighb = g.neighbBegin(currentResidue);
	   neighb != g.neighbEnd(currentResidue); ++neighb) {
	if (subsetVertices->find(*neighb) != subsetVertices->end()) {
	  //the symmetric addition occurs when currentResidue == *neighb:
	  _adjacencyList[currentResidue]->insert(*neighb);	  
	}
      }
    }
  }
}

void lbBasicGraph::deleteDataMembers() {
  lbBasicGraph::deleteAdjacencyList(_adjacencyList,_cards->size());
  delete _cards;
}

set<int>** lbBasicGraph::createNewAdjacencyList(int size, bool allocateVertices) {
  set<int>** list = new set<int>*[size];
  for (int i = 0; i < size; ++i) {
    if (allocateVertices) {
      list[i] = new set<int>();
    }
    else {
      list[i] = NULL;
    }
  }
  return list;
}

void lbBasicGraph::deleteAdjacencyList(set<int>** list, int size) {
  if (list == NULL) {
    return;
  }
  for (int i = 0; i < size; ++i) {
    if (list[i] != NULL)
      delete list[i];
  }
  delete[] list;
}

set<int>** lbBasicGraph::copyAdjacencyList() const {
  set<int>** retList = lbBasicGraph::createNewAdjacencyList(this->_cards->size(), false);
  this->copyAdjacencyListData(retList);
  return retList;
}

void lbBasicGraph::copyAdjacencyListData(set<int>** newList) const {
  int size = static_cast<int>(this->_cards->size());
  
  for (int i = 0; i < size; ++i) {
    if (this->hasVertexNoChecks(i)) {
      newList[i] = new set<int>();
      copy(this->_adjacencyList[i]->begin(), this->_adjacencyList[i]->end(),
	   insert_iterator<set<int> >(*(newList[i]), newList[i]->begin()));
    }
  }
}

lbBasicGraph::vertexIterator::vertexIterator() {
  _iteratelbBasicGraph = NULL;
  _currentVertex = NON_EXISTENT_VERTEX;
}

lbBasicGraph::vertexIterator::vertexIterator(lbBasicGraph const* iterateOver, int initialIndex) {
  _iteratelbBasicGraph = iterateOver;
  _currentVertex = NON_EXISTENT_VERTEX;  
  if (_iteratelbBasicGraph == NULL) {
    return;
  }
  advanceCurrentVertex(initialIndex);
}

int lbBasicGraph::vertexIterator::operator*() const {
  return _currentVertex;
}

lbBasicGraph::vertexIterator& lbBasicGraph::vertexIterator::operator++() {
  if (_iteratelbBasicGraph == NULL || _currentVertex == NON_EXISTENT_VERTEX) {
    return *this;
  }
  advanceCurrentVertex(_currentVertex+1);
  return *this;
}

lbBasicGraph::vertexIterator lbBasicGraph::vertexIterator::operator++(int) {
  vertexIterator tmp(*this);
  ++(*this);
  return tmp;
}

lbBasicGraph::vertexIterator& lbBasicGraph::vertexIterator::operator--() {
  if (_iteratelbBasicGraph == NULL || _currentVertex == 0) {
    return *this;
  }
  decrementCurrentVertex((_currentVertex != NON_EXISTENT_VERTEX)?
			 (_currentVertex-1):(_iteratelbBasicGraph->_cards->size() - 1));
  return *this;
}

lbBasicGraph::vertexIterator lbBasicGraph::vertexIterator::operator--(int) {
  vertexIterator tmp = *this;
  --(*this);
  return tmp;
}

void lbBasicGraph::vertexIterator::advanceCurrentVertex(int startFrom) {
  if (startFrom < 0) {
    startFrom = 0;
  }
  for (_currentVertex = startFrom; _currentVertex < static_cast<int>(_iteratelbBasicGraph->_cards->size()); ++_currentVertex) {
    if (_iteratelbBasicGraph->_adjacencyList[_currentVertex] != NULL) {
      break;
    }
  }
  if (_currentVertex >= static_cast<int>(_iteratelbBasicGraph->_cards->size()))
    _currentVertex = NON_EXISTENT_VERTEX;
}

void lbBasicGraph::vertexIterator::decrementCurrentVertex(int startFrom) {
  if (startFrom >= static_cast<int>(_iteratelbBasicGraph->_cards->size()) || startFrom == NON_EXISTENT_VERTEX) {
    startFrom = static_cast<int>(_iteratelbBasicGraph->_cards->size()) - 1;
  }
  for (_currentVertex = startFrom; _currentVertex >= 0; --_currentVertex) {
    if (_iteratelbBasicGraph->_adjacencyList[_currentVertex] != NULL) {
      break;
    }
  }
  if (_currentVertex < 0)
    _currentVertex = NON_EXISTENT_VERTEX;
}

bool lbBasicGraph::vertexIterator::operator==(lbBasicGraph::vertexIterator const& vi) const {
  return (_iteratelbBasicGraph == vi._iteratelbBasicGraph && _currentVertex == vi._currentVertex);
}

bool lbBasicGraph::isOutOfBounds(int index) const {
  return (index < 0 || index >= static_cast<int>(_cards->size()));
}

lbBasicGraph::vertexIterator lbBasicGraph::addVertex(int addIndex) {
  if (isOutOfBounds(addIndex)) {
    return this->verticesEnd();
  }
  if (_adjacencyList[addIndex] == NULL) {
    ++_numVertices;
    _adjacencyList[addIndex] = new set<int>();
  }
  return lbBasicGraph::vertexIterator(this,addIndex);  
}

bool lbBasicGraph::removeVertex(int removeIndex) {
  if (!hasVertex(removeIndex)) {
    return false;
  }
  removeVertexNoChecks(removeIndex);  
  return true;
}

void lbBasicGraph::removeVertexNoChecks(int removeIndex, lbBasicGraph::Clique* insertNeighbs) {
  for (lbBasicGraph::neighborIterator it = this->neighbBegin(removeIndex); it!= this->neighbEnd(removeIndex); ++it) {
    _adjacencyList[*it]->erase(removeIndex);
    if (insertNeighbs != NULL) {
      insertNeighbs->insert(*it);
    }
  }
  --_numVertices;
  delete _adjacencyList[removeIndex];
  _adjacencyList[removeIndex] = NULL;
}

bool lbBasicGraph::hasVertex(int checkIndex) const {
  return (!isOutOfBounds(checkIndex) && hasVertexNoChecks(checkIndex));
}

lbBasicGraph::Clique* lbBasicGraph::eliminateVertex(int v) {
  if (!hasVertex(v)) {
    return NULL;
  }
  lbBasicGraph::Clique* neighbs = new lbBasicGraph::Clique();
  removeVertexNoChecks(v,neighbs);

  //add edges between all pairs of ex-neighbors:
  for (lbBasicGraph::Clique::const_iterator n1 = neighbs->begin(); n1 != neighbs->end(); ++n1) {
    lbBasicGraph::Clique::const_iterator n2 = n1;
    for (++n2; n2 != neighbs->end(); ++n2) {//Start at n1 + 1:
      addEdgeNoChecks(*n1,*n2);
    }
  }
  neighbs->insert(v); //whole clique created now
  return neighbs;
}

int lbBasicGraph::numNeighbors(int v) const {
  if (!hasVertex(v)) {
    return -1;
  }
  return numNeighborsNoChecks(v);
}

bool lbBasicGraph::addEdge(int v1, int v2) {
  if ((v1 == v2) || !hasVertex(v1) || !hasVertex(v2)) {
    return false;
  }
  addEdgeNoChecks(v1,v2);
  return true;
}

void lbBasicGraph::addEdgeNoChecks(int v1, int v2) {
  _adjacencyList[v1]->insert(v2);
  _adjacencyList[v2]->insert(v1);
}

bool lbBasicGraph::hasEdge(int v1, int v2) const {
  if (!hasVertex(v1) || !hasVertex(v2)) {
    return false;
  }
  return hasEdgeNoChecks(v1,v2);
}

bool lbBasicGraph::removeEdge(int v1, int v2) {
  if (!hasEdge(v1,v2)) {
    return false;
  }
  removeEdgeNoChecks(v1,v2);
  return true;
}

void lbBasicGraph::removeEdgeNoChecks(int v1, int v2) {
  _adjacencyList[v1]->erase(v2);
  _adjacencyList[v2]->erase(v1);
}

bool lbBasicGraph::operator==(const lbBasicGraph& g) const {
  if (this->_cards->size() != g._cards->size() || this->numVertices() != g.numVertices()) {
    return false;
  }
  for (lbBasicGraph::vertexIterator gIt = g.verticesBegin();
       gIt != g.verticesEnd(); ++gIt) {
    int vert = *gIt;
    if (!this->hasVertexNoChecks(vert)) {//missing vertex that is in g
      return false;
    }    
    if (this->numNeighborsNoChecks(vert) != g.numNeighborsNoChecks(vert)) {
      return false;
    }
    for (lbBasicGraph::neighborIterator vertNeighbsIt = g.neighbBegin(vert);
	 vertNeighbsIt != g.neighbEnd(vert); ++vertNeighbsIt) {
      if (!this->hasEdge(vert,*vertNeighbsIt)) {
	return false;
      }
    }
  }
  return true;    
}

int lbBasicGraph::getStateSpaceCardinality(int v) const {
  if (!hasVertex(v)) {
    return -1;
  }
  int card = (*_cards)[v];
  return (card != 0) ? card : 1;
}

ostream& lbBasicGraph::print(ostream& stream) const {
  stream << "Number of vertices = " << _numVertices << endl;
  //print vertices and their corresponding neighbors:
  for (lbBasicGraph::vertexIterator vi = this->verticesBegin();
       vi != this->verticesEnd(); ++vi) {
    int vert = *vi;
    stream << vert << ": ";
    for (lbBasicGraph::neighborIterator neighb = this->neighbBegin(vert);
	 neighb != this->neighbEnd(vert); ++neighb) {
      stream << *neighb << " ";
    }
    stream << endl;
  }
  return stream;
}

namespace lbLib {
  
  //prints g using print()
  ostream& operator<<(ostream& stream, lbBasicGraph const& g) {
    return g.print(stream);
  }
  
}
