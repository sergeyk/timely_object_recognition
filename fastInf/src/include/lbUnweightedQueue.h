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

#ifndef _Unweighted_Queue_
#define _Unweighted_Queue_

#include <lbQueueInterface.h>
#include <list>

namespace lbLib {

  /*
   * This is a queue for message indices
   */

  class lbUnweightedQueue : public lbQueueInterface {
  public:
    lbUnweightedQueue() : lbQueueInterface() {}
    virtual ~lbUnweightedQueue() {}

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
    
    list<messageIndex> _queue;
    messageMask _enqueued;
  };

  inline void lbUnweightedQueue::clear() {
    while (!empty()) {
      pop();
    }
  }

  inline bool lbUnweightedQueue::empty() const {
    return (size() == 0);
  }

  inline int lbUnweightedQueue::size() const {
    return _queue.size();
  }

  inline void lbUnweightedQueue::push(messageIndex mi, double weight) {
    assert(weight == 0);

    // If the message has already been enqueued, ignore the push.
    if (_enqueued.find(mi) != _enqueued.end() && _enqueued[mi]) {
      return;
    }

    _queue.push_back(mi);
    _enqueued[mi] = true;
  }

  inline messageIndex lbUnweightedQueue::pop() {
    messageIndex mi;
    assert(size() > 0);
    mi = _queue.front();
    _queue.pop_front();
    _enqueued[mi] = false;
    return mi;
  }

  inline messageIndexVec lbUnweightedQueue::pop(int size) {
    messageIndexVec qtop;

    for (int i = 0; i < size; i++) {
      if (empty()) {
	break;
      }
      
      qtop.push_back(pop());
    }

    return qtop;
  }

  inline messageIndexVec lbUnweightedQueue::top(int size) const {
    messageIndexVec miv;

    list<messageIndex>::const_iterator it = _queue.begin();
    int i = 0;

    for (it = _queue.begin(); i < size && it != _queue.end(); it++) {
      miv.push_back(*it);
      i++;
    }

    return miv;
  }  

  inline void lbUnweightedQueue::print(ostream & out) const {
    list<messageIndex>::const_iterator it = _queue.begin();

    out << "Printing queue: " << endl;
    for (it = _queue.begin(); it != _queue.end(); it++) {
      messageIndex mi = *it;
      out << "(" << mi.first << " --> " << mi.second << ")" << endl;
    }
  }

  inline void lbUnweightedQueue::remove(messageIndex mi) {
    list<messageIndex>::iterator it;

    for (it = _queue.begin(); it != _queue.end(); it++) {
      if (mi == *it) {
	break;
      }
    }
    
    if (it != _queue.end()) {
      _queue.erase(it);
    }
  }
};

#endif
