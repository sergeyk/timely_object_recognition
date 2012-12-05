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

#ifndef _Weighted_Queue_
#define _Weighted_Queue_

#include <lbQueueInterface.h>
#include <lbRedBlackTree.h>

namespace lbLib {

  typedef TStorage<double, messageIndex> weightTree;
  typedef TStorageNode<double, messageIndex> weightNode;
  typedef map<messageIndex, weightNode *, lessMessageIndex> messageIndexMap;

  class lbWeightedQueue : public lbQueueInterface {
  public:
    lbWeightedQueue() : lbQueueInterface() { _size = 0; }
    virtual ~lbWeightedQueue() {}

    virtual inline void push(messageIndex mi, double weight);

    virtual inline messageIndex pop();
    virtual inline messageIndexVec pop(int size);
    virtual inline messageIndexVec top(int size) const;
    virtual inline void clear();

    virtual inline bool empty() const;
    virtual inline int size() const;

    virtual inline void remove(messageIndex mi);

    virtual inline void print(ostream & out) const;

  private:

    int _size;

    weightTree _rbt;
    messageIndexMap _rbtMap;
    messageMask _enqueued;
  };

  inline void lbWeightedQueue::clear() {
    while (!empty()) {
      pop();
    }
  }

  inline bool lbWeightedQueue::empty() const {
    return (size() == 0);
  }

  inline int lbWeightedQueue::size() const {
    return _size;
  }

  inline void lbWeightedQueue::push(messageIndex mi, double weight) {
    remove(mi);
    _rbtMap[mi] = _rbt.Insert(weight, mi);
    _size++;
    _enqueued[mi] = true;
  }

  inline messageIndex lbWeightedQueue::pop() {
    messageIndex mi;
    assert(size() > 0);

    weightNode * max = _rbt.GetLast();
    mi = max->GetData();
    _rbt.Delete(max);
    _rbtMap[mi] = NULL;
    _size--;

    _enqueued[mi] = false;
    return mi;
  }

  inline messageIndexVec lbWeightedQueue::pop(int s) {
    messageIndexVec qtop;

    for (int i = 0; i < s; i++) {
      if (empty()) {
	break;
      }
      
      qtop.push_back(pop());
    }

    return qtop;
  }

  inline messageIndexVec lbWeightedQueue::top(int s) const {
    messageIndexVec miv;

    // Look at the k top indices
    weightNode const * max = _rbt.GetLastConst();

    for (int i = 0; i < s; i++) {
      if (max == NULL) {
	break;
      }

      miv.push_back(max->GetData());
      max = max->GetPrevConst();
    }

    return miv;
  }  

  inline void lbWeightedQueue::print(ostream & out) const {
    // Look at the k top indices
    weightNode const * max = _rbt.GetLastConst();

    cerr << "Printing Queue: " << endl;
    for (int i = 0; i < size(); i++) {
      if (max == NULL) {
	break;
      }

      cerr << "(" << max->GetData().first << "," <<  max->GetData().second << "): " << max->GetKey() << endl;

      max = max->GetPrevConst();
    }
  }

  inline void lbWeightedQueue::remove(messageIndex mi) {
    if (_rbtMap.find(mi) != _rbtMap.end()) {
      if (_rbtMap[mi] != NULL) {
	_rbt.Delete(_rbtMap[mi]);
	_size--;
      }
    }

    _rbtMap[mi] = NULL;
  }
};

#endif
