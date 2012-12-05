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

#ifndef _Manual_Queue_
#define _Manual_Queue_

#include <lbQueueInterface.h>
#include <vector>

namespace lbLib {

  /*
   * This is a queue for message indices
   */

  class lbManualQueue : public lbQueueInterface {
  public:
    lbManualQueue(messageIndexVec const & miv);
    virtual ~lbManualQueue() {}

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

    inline bool isPossible(messageIndex mi) const;
    inline bool isEnqueued(messageIndex mi) const;
 
    vector<messageIndex> _messageOrder;
    messageMask _enqueued;
    messageMask _possibleMessages;
    int _currentIndex;
    int _size;
  };

  inline lbManualQueue::lbManualQueue(messageIndexVec const & miv) 
    : lbQueueInterface() { 

    _currentIndex = 0;
    _size = 0; 

    _messageOrder = miv;
    _possibleMessages.clear();

    for (uint i = 0; i < miv.size(); i++) {
      messageIndex mi = miv[i];
      _possibleMessages[mi] = true;
    }
  }

  inline void lbManualQueue::clear() {
    while (!empty()) {
      pop();
    }
  }

  inline bool lbManualQueue::empty() const {
    return (size() == 0);
  }

  inline int lbManualQueue::size() const {
    return _size;
  }

  inline bool lbManualQueue::isPossible(messageIndex mi) const {
    return _possibleMessages.find(mi) != _possibleMessages.end();
  }

  inline bool lbManualQueue::isEnqueued(messageIndex mi) const {
    assert(isPossible(mi));
    return _enqueued.find(mi) != _enqueued.end() && _enqueued.find(mi)->second;
  }

  inline void lbManualQueue::push(messageIndex mi, double weight) {
    assert(weight == 0);

    // If the message has already been enqueued, ignore the push.
    if (isEnqueued(mi)) {
      return;
    }

    assert(isPossible(mi));
    _size++;
    _enqueued[mi] = true;
  }

  inline messageIndex lbManualQueue::pop() {
    messageIndex mi;
    assert(size() > 0);

    while (true) {
      mi = _messageOrder[_currentIndex];

      _currentIndex++;
      _currentIndex %= _messageOrder.size();
      
      if (isEnqueued(mi)) {
	_enqueued[mi] = false;
	_size--;
	break;
      }
    }

    return mi;
  }

  inline messageIndexVec lbManualQueue::pop(int size) {
    messageIndexVec qtop;

    for (int i = 0; i < size; i++) {
      if (empty()) {
	break;
      }
      
      qtop.push_back(pop());
    }

    return qtop;
  }

  inline messageIndexVec lbManualQueue::top(int size) const {
    messageIndexVec miv;

    
    for (uint i = 0; i < _messageOrder.size(); i++) {
      int index = (_currentIndex + i)%_messageOrder.size();
      
      messageIndex mi = _messageOrder[index];

      if (isEnqueued(mi)) {
	miv.push_back(mi);
      }

      if ((int) miv.size() >= size) {
	break;
      }
    }

    return miv;
  }  

  inline void lbManualQueue::print(ostream & out) const {
    out << "Printing queue: currentIndex(" << _currentIndex << ")" << endl;
    for (uint i = 0; i < _messageOrder.size(); i++) {
      messageIndex mi = _messageOrder[i];
      assert(isPossible(mi));
      out << "(" << mi.first << " --> " << mi.second << ")";
      
      if (!isEnqueued(mi)) {
	out << " NOT enqueued";
      }

      out << endl;
    }
  }

  inline void lbManualQueue::remove(messageIndex mi) {
    vector<messageIndex>::iterator it;

    for (it = _messageOrder.begin(); it != _messageOrder.end(); it++) {
      if (mi == *it) {
	break;
      }
    }
    
    if (it != _messageOrder.end()) {
      _messageOrder.erase(it);
    }
  }
};

#endif
