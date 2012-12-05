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

#ifndef _Message_Bank_H_
#define _Message_Bank_H_

#include <lbDefinitions.h>
#include <lbOptions.h>
#include <lbAssignedMeasure.h>
#include <lbQueueInterface.h>

namespace lbLib {
  typedef map<messageIndex, lbAssignedMeasure_ptr, lessMessageIndex> messageMap;
  
  /*
   *  The message bank stores two layers of messages the actual
   *  messages on the edge then a set of "next" messages which can be
   *  placed down on the edge at the end of the "iteration".  An
   *  asynchrounous iteration would then have an _updateSize of 1.  A
   *  fully synchronous version would have an _updateSize equal to the
   *  number of messages.
   *
   *  The general usage of this class would be as follows:
   *
   *  do {
   *    messageIndexVec iteration = bank.getIteration();
   *    
   *    for (uint i = 0; i < iteration.size(); i++) {
   *       bank.setMessage(iteration[i], computeMessage(iteration[i]));
   *    }
   *  } while (bank.update());
   *
   *
   *  where compute message makes use of getMessage() to get the
   *  previous iterations method on which to compute the new message.
   *
   */
  class lbMessageBank {
  public:

    lbMessageBank(adjListVec const& neighbors);
    virtual ~lbMessageBank();

    virtual messageIndexVec const & getIteration() = 0;
    virtual bool update() = 0;
    virtual lbQueueInterface * getQueue() = 0;


    lbAssignedMeasure_ptr getMessage(messageIndex mi);
    const lbAssignedMeasure_ptr getMessage(messageIndex mi) const;
    void initMessage(messageIndex mi, lbAssignedMeasure_ptr message);
    void setMessage(messageIndex mi, lbAssignedMeasure_ptr message);

    int getTotalDirty() const { return _totalDirty; }
    int getTotalUpdated() const { return _totalUpdated; }

    void setAffectAll(double sp) { _affectAll = sp; }
    double getAffectAll() const { return _affectAll; }

    void setUpdateSize(int s);
    int getUpdateSize() const { return _updateSize; }

    void setSmoothing(double sp) { _smoothParam = sp; }
    double getSmoothing() const { return _smoothParam; }

    void setThreshold(double thresh) { _threshold = thresh; }
    double getThreshold() const { return _threshold; }

    void setCompareType(lbMessageCompareType type) { _compareType = type; }
    lbMessageCompareType getCompareType() const { return _compareType; }

    void printRealMessages(ostream& O) const;
    void printRealMessages(messageIndex mi,ostream& O) const;
    messageMap const& getRealMessages() const { return _realMessages; };

    void randomizeMessages();
    void randomizeMessage(messageIndex mi);

    // Adds a new message to be sent between the cliques in the messageIndex mi.
    // This new message is assumed to have had a uniform previous message
    void addNewMessage(messageIndex mi, lbAssignedMeasure_ptr message,
		       bool normalize = true);
    void removeMessage(messageIndex mi);
    void setBothMessages(messageIndex mi, lbAssignedMeasure_ptr newMessage,
			 lbAssignedMeasure_ptr realMessage);
    
    void setRealMessage(messageIndex mi, lbAssignedMeasure_ptr real);
    void setNextMessage(messageIndex mi, lbAssignedMeasure_ptr next);

  protected:
    void setTotalDirty(int total) { _totalDirty = total; }

    lbAssignedMeasure_ptr getRealMessage(messageIndex mi);
    const lbAssignedMeasure_ptr getRealMessage(messageIndex mi) const;
    lbAssignedMeasure_ptr getNextMessage(messageIndex mi);

    void updateManyMessages(messageIndexVec & changed);
    void updateMessage(messageIndex mi);

    void clearMessages();

    bool isDifferent(messageIndex mi);
    messageIndexVec getAffectedMessages(messageIndexVec const & miv);
    cliquesVec const & getNeighbors(cliqIndex ci) const { return _neighbors[ci]; }

    virtual void pushNewMessageOnQueue(messageIndex mi);

  private:

    adjListVec const& _neighbors;

    messageMap _nextMessages;
    messageMap _realMessages;

    bool _affectAll;
    int _updateSize;
    int _totalUpdated;
    int _totalDirty;

    lbMessageCompareType _compareType;
    probType _smoothParam;
    probType _threshold;
  };

  inline void lbMessageBank::setUpdateSize(int s) { 
    if (s == 0) {
      for (uint i = 0; i < _neighbors.size(); i++) {
	s += _neighbors[i].size();
      }
    }

    _updateSize = s; 
  }

  inline void lbMessageBank::clearMessages() {
    messageMap::iterator it; 

    // Delete everything in the "real" layer
    for (it = _realMessages.begin(); it != _realMessages.end(); it++) {
      lbAssignedMeasure_ptr real = it->second;
      assert(real != NULL);
      delete real;
    }

    // Delete anything in the "next" layer we missed
    for (it = _nextMessages.begin(); it != _nextMessages.end(); it++) {
      lbAssignedMeasure_ptr next = it->second;
      delete next;
    }

    _nextMessages.clear();
    _realMessages.clear();
  }

  // pops messages from queue and updates them
  inline void lbMessageBank::updateManyMessages(messageIndexVec & changed) {
    messageIndexVec update = getQueue()->pop(_updateSize);

    for (uint i = 0; i < update.size(); i++) {
      messageIndex mi = update[i];
      
      if (isDifferent(mi)) {
	changed.push_back(mi);
	updateMessage(mi);
	_totalUpdated++;
      }
    }
  }
  
  // update of messages causes the "next" message to be 
  // transfered (with smoothin) to the current "real" one
  inline void lbMessageBank::updateMessage(messageIndex mi) {
    lbAssignedMeasure_ptr real = getRealMessage(mi);
    lbAssignedMeasure_ptr next = getNextMessage(mi);
    assert(next != NULL);
    assert(real != NULL);

    if (lbOptions::isVerbose(V_MESSAGES) && real != NULL) {
      cerr << "MESSAGE: Updated (" << mi.first << "-->" << mi.second << ")" << endl;
      cerr << "\n---\nOLD\n---\n";
      getMessage(mi)->print(cerr);
    }

    real->updateMeasureValues(next->getMeasure(), _smoothParam);
    
    if (lbOptions::isVerbose(V_MESSAGES)) {
      cerr << "\n---\nNEW\n---\n";
      getMessage(mi)->print(cerr);
      cerr << endl;
    }
  }

  inline messageIndexVec lbMessageBank::getAffectedMessages(messageIndexVec const & change) {
    messageMask mask;
    messageIndexVec miv;

    for (uint i = 0; i < change.size(); i++) {
      messageIndex mi = change[i];
      
      // Push back the changed message if need be
      if (_smoothParam != 0 && mask.find(mi) == mask.end()) {
	mask[mi] = true;
	miv.push_back(mi);
      }

      cliquesVec const& neighbors = _neighbors[mi.second];
      
      // Push back the outgoing neighbors
      for (uint j = 0; j < neighbors.size(); j++) {
	if (neighbors[j] != mi.first || _affectAll) {
	  messageIndex neighmi(mi.second, neighbors[j]);
	  
	  if (mask.find(neighmi) == mask.end()) {
	    mask[neighmi] = true;
	    miv.push_back(neighmi);
	  }
	}
      }

      // If we're affecting everything (for gbp)
      // Push back the neighbors going into the toCliq
      if (_affectAll) {
	for (uint j = 0; j < neighbors.size(); j++) {
	  if (neighbors[j] != mi.first) {
	    messageIndex neighmi(neighbors[j], mi.second);
	  
	    if (mask.find(neighmi) == mask.end()) {
	      mask[neighmi] = true;
	      miv.push_back(neighmi);
	    }
	  }
	}
      }
    }
    
    return miv;
  }

  /*
   * If a message is NULL we consider it to be the same as the message
   * below.  Comes in handy when we have irrelevant messages that
   * never get computed (with evidence).
   */ 
  inline bool lbMessageBank::isDifferent(messageIndex mi) {
    lbAssignedMeasure_ptr next = getNextMessage(mi);
    lbAssignedMeasure_ptr real = getRealMessage(mi);

    if (next == NULL) {
      return false;
    }

    assert(real != NULL);
    bool changed = real->isDifferent(*next, _compareType, _threshold);
    return changed;
  }

  inline lbAssignedMeasure_ptr lbMessageBank::getMessage(messageIndex mi) {
    lbAssignedMeasure_ptr real = getRealMessage(mi);
    assert(real != NULL);
    return real;
  }

  inline const lbAssignedMeasure_ptr lbMessageBank::getMessage(messageIndex mi) const {
    const lbAssignedMeasure_ptr real = getRealMessage(mi);
    assert(real != NULL);
    return real;
  }

  inline void lbMessageBank::initMessage(messageIndex mi, lbAssignedMeasure_ptr newmessage) {
    setRealMessage(mi, newmessage);
    getQueue()->push(mi);
    _totalDirty++;
  }

  inline void lbMessageBank::setMessage(messageIndex mi, lbAssignedMeasure_ptr newmessage) {
    setNextMessage(mi, newmessage);
  }

  inline lbAssignedMeasure_ptr lbMessageBank::getRealMessage(messageIndex mi) {
    if (_realMessages.find(mi) != _realMessages.end()) {
      return _realMessages[mi];
    }

    return NULL;
  }

  inline const lbAssignedMeasure_ptr lbMessageBank::getRealMessage(messageIndex mi) const {
    messageMap::const_iterator it = _realMessages.find(mi);
    if ( it != _realMessages.end())
      return it->second;
    return NULL;
  }

  inline lbAssignedMeasure_ptr lbMessageBank::getNextMessage(messageIndex mi) {
    if (_nextMessages.find(mi) != _nextMessages.end()) {
      return _nextMessages[mi];
    }

    return NULL;
  }

  inline void lbMessageBank::setRealMessage(messageIndex mi, lbAssignedMeasure_ptr real) {
    lbAssignedMeasure_ptr oldmessage = getRealMessage(mi);

    if (oldmessage != NULL) {
      delete oldmessage;
    }

    _realMessages[mi] = real;
  }

  inline void lbMessageBank::setNextMessage(messageIndex mi, lbAssignedMeasure_ptr next) {
    lbAssignedMeasure_ptr oldmessage = getNextMessage(mi);

    if (oldmessage != NULL) {
      delete oldmessage;
    }

    _nextMessages[mi] = next;
  }

  inline void lbMessageBank::randomizeMessages() {
    messageMap::iterator it;
    for (it=_realMessages.begin(); it!=_realMessages.end(); it++ ) {
      //      cerr << "Randomizing " << it->first.first << "->" << it->first.second << endl;
      randomizeMessage(it->first);
    }
  }
  
  inline void lbMessageBank::randomizeMessage(messageIndex mi) {
    lbAssignedMeasure_ptr meas = getRealMessage(mi);
    meas->makeRandom();
    lbAssignedMeasure_ptr meas2 = getNextMessage(mi);
    meas2->makeRandom();
  }
  
  inline void lbMessageBank::addNewMessage(messageIndex mi,
					   lbAssignedMeasure_ptr message,
					   bool normalize) {
    lbAssignedMeasure_ptr realMessage = message->duplicate();
    realMessage->makeUniform();
    if (normalize) {
      realMessage->normalize();
    }
    setBothMessages(mi, message, realMessage);
    pushNewMessageOnQueue(mi);
  }

  inline void lbMessageBank::removeMessage(messageIndex mi) {
    getQueue()->remove(mi);
  }
  inline void lbMessageBank::setBothMessages(messageIndex mi,
					     lbAssignedMeasure_ptr newMessage,
					     lbAssignedMeasure_ptr realMessage) {
    messageMap::iterator rit = _realMessages.find(mi);
    messageMap::iterator nit;
    if (rit != _realMessages.end()) {
      nit = _nextMessages.find(mi);
      delete nit->second;
      delete rit->second;
      _nextMessages.erase(nit);
      _realMessages.erase(rit);
      
    }
    setNextMessage(mi, newMessage);
    setRealMessage(mi, realMessage);
  }
 
  inline void lbMessageBank::pushNewMessageOnQueue(messageIndex mi) {
    getQueue()->push(mi);
  }


};

#endif
