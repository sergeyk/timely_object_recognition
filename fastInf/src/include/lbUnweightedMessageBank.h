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

#ifndef _Unweighted_Message_Bank_H_
#define _Unweighted_Message_Bank_H_

#include <lbMessageBank.h>
#include <lbUnweightedQueue.h>
#include <lbManualQueue.h>

namespace lbLib {  

  class lbUnweightedMessageBank : public lbMessageBank {
  public:

    // The miv passed in is an optional forced message ordering (a
    // manual queue).  It is ignored if it is empty.
    lbUnweightedMessageBank(adjListVec const& neighbors, messageIndexVec const & miv);
    virtual ~lbUnweightedMessageBank();

    virtual messageIndexVec const & getIteration();
    virtual bool update();
    virtual lbQueueInterface * getQueue() { return _messageQueue; }

  private:

    lbQueueInterface * _messageQueue;
    messageIndexVec _iteration;
  };

  inline lbUnweightedMessageBank::lbUnweightedMessageBank(adjListVec const& neighbors, messageIndexVec const & order)
    : lbMessageBank(neighbors) {
  
    if (order.empty()) {
      _messageQueue = new lbUnweightedQueue();
    }
    else {
      _messageQueue = new lbManualQueue(order);
    }
  }

  inline lbUnweightedMessageBank::~lbUnweightedMessageBank() {
    delete _messageQueue;
  }

  inline messageIndexVec const & lbUnweightedMessageBank::getIteration() {
    _iteration = _messageQueue->top(getUpdateSize());
    return _iteration;
  }
  
  inline bool lbUnweightedMessageBank::update() {
    assert(_messageQueue != NULL);

    messageIndexVec changed; 
    updateManyMessages(changed);

    messageIndexVec affected = getAffectedMessages(changed);
    for (uint i = 0; i < affected.size(); i++) {
      _messageQueue->push(affected[i]);
    }

    setTotalDirty(_messageQueue->size());

    return (getTotalDirty() != 0);
  }
};

#endif
