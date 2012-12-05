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

#ifndef _Weighted_Message_Bank_H_
#define _Weighted_Message_Bank_H_

#include <lbMessageBank.h>
#include <lbWeightedQueue.h>

namespace lbLib {

  class lbWeightedMessageBank : public lbMessageBank {

  public:

    lbWeightedMessageBank(adjListVec const& neighbors,lbMessageWeightType WType = MWT_L1);
    virtual ~lbWeightedMessageBank();

    virtual messageIndexVec const & getIteration();
    virtual bool update(); 
    virtual lbQueueInterface * getQueue() { return _messageQueue; }

  protected:
    virtual void pushNewMessageOnQueue(messageIndex mi);

  private:
    double getWeight(messageIndex mi);
    void setFirstIteration();

    lbQueueInterface * _messageQueue;
    messageIndexVec _iteration;
    lbMessageWeightType _WType;
    bool _firstIter;
  };
  
  inline lbWeightedMessageBank::lbWeightedMessageBank(adjListVec const& neighbors,lbMessageWeightType WType) :
    lbMessageBank(neighbors),_WType(WType),_firstIter(true) {
    _messageQueue = new lbWeightedQueue();

    // put all messages into iteration
    for (uint i = 0; i < neighbors.size(); i++) {
      for (uint j = 0; j < neighbors[i].size(); j++) {
	_iteration.push_back(messageIndex(i, neighbors[i][j]));
      }
    }
  }

  inline lbWeightedMessageBank::~lbWeightedMessageBank() {
    delete _messageQueue;
  }

  inline messageIndexVec const & lbWeightedMessageBank::getIteration() {
    return _iteration;
  }

  inline double lbWeightedMessageBank::getWeight(messageIndex mi) {
    lbAssignedMeasure_ptr real = getRealMessage(mi);
    lbAssignedMeasure_ptr next = getNextMessage(mi);

    assert(next != NULL && real != NULL);

    double res = -HUGE_VAL;
    switch ( _WType ) {
    case MWT_L1:
      res = next->getL1(*real);
      break;
    case MWT_L2:
      res = next->getL2(*real);
      break;
    case MWT_LINF:
      res = next->getMaxDiff(*real);
      break;
    default:
      assert(false);
      break;
    }
    return res;
  }

  inline void lbWeightedMessageBank::pushNewMessageOnQueue(messageIndex mi) {
    getQueue()->push(mi, getWeight(mi));
  }

  inline bool lbWeightedMessageBank::update() {
    assert(_messageQueue != NULL);

    // previous iteration is put into the weighted queuu
    for (uint i = 0; i < _iteration.size(); i++) {
      messageIndex mi = _iteration[i];
      
      if (isDifferent(mi)) {
	_messageQueue->push(mi, getWeight(mi));
      }
      else {
	_messageQueue->remove(mi);
      }
    }

    setTotalDirty(_messageQueue->size());

    // this is where the messages are pulled out of the queue
    messageIndexVec changed; 
    updateManyMessages(changed);
    // the effected messages are the ones that need to be computed
    _iteration = getAffectedMessages(changed);
    return (getTotalDirty() != 0);
  }

};

#endif
