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

#ifndef _Message_Queue_
#define _Message_Queue_

#include <lbDefinitions.h>
#include <lbGraphBase.h>
#include <lbSupport.h>
#include <queue>

#define LOOP_MARKER -1

namespace lbLib {

  typedef pair<cliqIndex, cliqIndex> indexPair;
  
  /*
   * Abstract base class that defines the functionality necessary 
   * to implement a messageQueue usable by lbPropagationInf.
   * \version July 2009
   * \author Ian McGraw
   */
  class lbMessageQueue {
  public:

    lbMessageQueue(lbGraphBase const& structure);
    explicit lbMessageQueue(lbMessageQueue const& Mqueue);
    virtual ~lbMessageQueue() {}

    // Returns true if already enqueued.
    virtual void pushToQueue(cliqIndex fromCliq, cliqIndex toCliq) = 0;

    virtual bool refreshQueue() = 0;
    inline virtual void clearQueue();

    virtual indexPair popFromQueue() = 0;
    inline virtual int getIteration() const;
    inline virtual bool isLoopEnd() const;
    inline virtual bool isEmpty() const;
    inline virtual bool isEqual(lbMessageQueue const& Mqueue) const;

    inline void setDirtyNode(nodeIndex node);
    inline nodeIndexVec getDirtyWeightedGraphNodes() const { return _dirtyWeightedGraph.getNodeIndices(); }
    inline arcIndexVec getDirtyWeightedGraphArcs() const { return _dirtyWeightedGraph.getArcIndices(); }
    inline void clearDirtyWeightedGraph() { _dirtyWeightedGraph.clear(); }

    inline queue<indexPair> const& getQueue() const { return _messageQueue; }
    inline lbGraphBase const& getStructure() const { return _structure; }
    inline virtual void print(ostream & out);

  protected:
    
    lbGraphBase const& _structure;
    queue<indexPair> _messageQueue;
    int _iteration;
    lbSubgraph _dirtyWeightedGraph;
  };


  /*
   * Facilitates asynchronous message passing.
   * \version July 2009
   * \author Ian McGraw
   */
  class lbAsyncQueue : public lbMessageQueue {
  public:
    lbAsyncQueue(lbGraphBase const& structure);
    explicit lbAsyncQueue(lbAsyncQueue const& Aqueue);
    virtual ~lbAsyncQueue() {}

    inline virtual void pushToQueue(cliqIndex fromCliq, cliqIndex toCliq);
    virtual bool refreshQueue();

    inline virtual indexPair popFromQueue();

  };


  /*
   * Facilitates tree-based message passing.
   * \version July 2009
   * \author Ian McGraw
   */
  class lbTRPQueue : public lbMessageQueue {
  public:

    lbTRPQueue(lbGraphBase const& structure,
	       lbGraphStruct const* clusterGraph, 
	       varIndicesVecVec const& scopes);

    lbTRPQueue(lbTRPQueue const& Tqueue); 
    virtual ~lbTRPQueue() {}

    inline virtual void pushToQueue(cliqIndex fromCliq, cliqIndex toCliq);
    virtual bool refreshQueue();

    inline virtual indexPair popFromQueue();

    static inline void setHybrid(bool hybrid) { _hybrid = hybrid; }
  protected:

    inline void addArcsToQueue(arcVec const& arcs);

    lbGraphStruct const* _clusterGraph;
    varIndicesVecVec _scopes;
    static bool _hybrid;
  };


  /*
   * Methods common to all message queues
   */
  
  inline int lbMessageQueue::getIteration() const {
    return _iteration;
  }
  
  // Return if the front is the marker without popping it
  inline bool lbMessageQueue::isLoopEnd() const {
    if (_messageQueue.empty()) {
      return false;
    }

    indexPair message = _messageQueue.front();
    return (message.second == LOOP_MARKER);
  }
  
  inline bool lbMessageQueue::isEmpty() const {
    if (_structure.getNumArcs() == 0) {
      return true;
    }

    return (_messageQueue.empty() ||
	    (_messageQueue.size() == 1 &&
	     isLoopEnd()));
  }

  inline bool lbMessageQueue::isEqual(lbMessageQueue const& q) const {
    queue<indexPair> copy0 = _messageQueue;
    queue<indexPair> copy1 = q._messageQueue;

    while (!copy0.empty() && !copy1.empty()) {
      indexPair p0 = copy0.front();
      indexPair p1 = copy1.front();
      copy0.pop();
      copy1.pop();

      if (p0.first != p1.first || p0.second != p1.second) {
	return false;
      }
    }

    if (!copy0.empty() || !copy1.empty()) {
      return false;
    }
    
    return true;
  }

  inline void lbMessageQueue::clearQueue() {
    _iteration = 0;

    while (!_messageQueue.empty()) {
      _messageQueue.pop();
    }

    _messageQueue.push(indexPair(0, LOOP_MARKER));
  }

  inline void lbMessageQueue::print(ostream & out) {

    queue<indexPair> q = _messageQueue;
    out << "Printing Message Queue: " << endl;
    while (!q.empty()) {
      cliqIndex toCliq = q.front().second;
      cliqIndex fromCliq = q.front().first;

      if (toCliq == LOOP_MARKER) {
	out << "(loop marker)" << endl;
      }
      else if (toCliq >= 0 && toCliq < _structure.getNumNodes() &&
	       fromCliq >= 0 && fromCliq < _structure.getNumNodes()) {
	out << "(" << fromCliq << " --> " << toCliq << ")" << endl;
      }
      else {
	  out << "(unrecognized)" << endl;
      }

      q.pop();
    }
  } 
  
  inline void lbMessageQueue::setDirtyNode(nodeIndex node) {
    arcIndexVec const& aiv = _structure.getNeighboringArcIndices(node);
    for (uint i = 0; i < aiv.size(); i++) {
      _dirtyWeightedGraph.addArc(aiv[i]);
    }
  }





  /*
   * Methods specific to passing messages asynchronously.
   */
  inline void lbAsyncQueue::pushToQueue(cliqIndex fromCliq, cliqIndex toCliq) {
    _messageQueue.push(indexPair(fromCliq, toCliq));
  }
  
  inline indexPair lbAsyncQueue::popFromQueue() {
    assert (!isEmpty());
    
    indexPair message;
    
    message = _messageQueue.front();
    _messageQueue.pop();

    if (message.second == LOOP_MARKER) {
      _iteration++;
      _messageQueue.push(message);  // Push marker back on
      message = _messageQueue.front();
      _messageQueue.pop();
    }
    
    return message;
  }
  
  




  
  inline void lbTRPQueue::pushToQueue(cliqIndex fromCliq, cliqIndex toCliq) {
  }
  
  inline void lbTRPQueue::addArcsToQueue(arcVec const& arcs) {
    // Add upward pass.
    for (uint j1 = 0; j1 < arcs.size(); j1++) {
      cliqIndex fromCliq = (int) arcs[j1].first;
      cliqIndex toCliq = (int) arcs[j1].second;
      _messageQueue.push(indexPair(fromCliq, toCliq));
    }
    
    // Add downward pass.
    for (int j2 = (int) arcs.size() - 1; j2 >= 0; j2--) {
      cliqIndex fromCliq = (int) arcs[j2].second;
      cliqIndex toCliq = (int) arcs[j2].first;
      _messageQueue.push(indexPair(fromCliq, toCliq));
    }
  }


  inline indexPair lbTRPQueue::popFromQueue() {
    assert (!isEmpty());

    indexPair message;
    bool found = false;

    while (!found) {
      message = _messageQueue.front();
      _messageQueue.pop();
      _messageQueue.push(message);

      if (message.second == LOOP_MARKER) {
	_iteration++;
      }
      else {
	found = true;
      }
    }

    return message;
  }
};

#endif
