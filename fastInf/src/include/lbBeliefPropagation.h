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

#ifndef _Belief_Propagation_
#define _Belief_Propagation_

#include <lbPropagationInference.h>
#include <lbMessageBank.h>
#include <lbInferenceMonitor.h>

#define UPDATE_CLOCK_PER_MESSAGES 10000

namespace lbLib {


  /*
   *  lbBeliefPropagation implements all the necessary methods to run
   *  inference on a model.
   */
  class lbBeliefPropagation : public lbPropagationInference {
    
  public:

    /*
     **********************************************************
     Ctors and Dtors :
     **********************************************************
     */

    lbBeliefPropagation(lbModel & model, lbMeasureDispatcher const & disp);
    explicit lbBeliefPropagation(lbBeliefPropagation const & otherPropagation);
    virtual ~lbBeliefPropagation();

    /********************
    Mutators
    *********************/

    virtual void changeEvidence(lbAssignment const& assign,bool forceUpdate = false);

    // Main method for running inference needs to be public so that
    // users can control when the bulk of the processing happens if
    // they would like.  Also called automatically from functions such
    // as prob, etc.
    virtual bool calcProbs();


    /*  
     **********************************************************
     change \ view properties of message passing
     **********************************************************
     */
    
    virtual void setOptions(lbOptions & opt, int argc, char *argv[]);
    void createInferenceMonitor() { if ( _monitor == NULL )_monitor = new lbInferenceMonitor(this); }
    lbInferenceMonitor * getInferenceMonitor() { return _monitor; }

    void setMaxMessages(int maxMessages) { _maxMessages = maxMessages; }
    int getMaxMessages() const { return _maxMessages; }

    void setMaxSeconds(int maxSeconds) { _maxSeconds = maxSeconds; }
    int getMaxSeconds() const { return _maxSeconds; }

    void setUpdateSize(int s) { _updateSize = s; }
    int getUpdateSize() const { return _updateSize; }

    void setMessageWeightType(lbMessageWeightType WType) { _WType = WType; };
    lbMessageWeightType getMessageWeightType() const { return _WType; };

    void setCompareType(lbMessageCompareType type) { _compareType = type; }
    lbMessageCompareType getCompareType() const { return _compareType; }

    void setInitType(lbMessageInitType type) { _messageInitType = type; }
    lbMessageInitType getInitType() const { return _messageInitType; }

    void setQueueType(lbMessageQueueType type) { _messageQueueType = type; }
    lbMessageQueueType getQueueType() const { return _messageQueueType; }

    void setSmoothing(double smooth);
    probType getSmoothing() const { return _smoothParam; }

    void setThreshold(probType newThresh) { _threshold = newThresh; }
    probType getThreshold() const { return _threshold; }

    void setUnzeroCopiedMessages(bool unzeroCopiedMessages) { _unzeroCopiedMessages = unzeroCopiedMessages; }
    bool getUnzeroCopiedMessages() const { return _unzeroCopiedMessages; }

    void setManualQueueOrder(messageIndexVec const & miv) { _ordering = miv; }

    lbMessageBank const& MessageBank() const { return *_messageBank; }

    int getMessageCount() const { return _messageCount; }

    // Compute a message
    virtual lbAssignedMeasure_ptr computeMessage(messageIndex messIndex) const;

    // randomize messages
    void randomizeMessages();
    void randomizeMessage(messageIndex mi);

    probVector getNegMutualInfos (const cliquesVec & cliqs) ;

    void resetMessages(bool useOld = false);

  protected:

    /*
     ********************************
     Initialization Methods
     ********************************
     */
    virtual void initialize(bool allocate,bool useOldInfo);

    virtual lbMessageBank * getNewMessageBank();

    /*
     ********************************
     Running Propagation Methods
     ********************************
     */
    
    // Run a single iteration of the propagation
    virtual void singlePropagationIteration();

    // Check if we're over time or passed max message count
    bool stoppingCriterionReached();

    // Returns current belief about cliq from cache
    virtual lbAssignedMeasure_ptr getBelief(cliqIndex cliq) const;
    
    // Determine if this message is relevant (should it be in queue?)
    virtual bool messageIsRelevant(messageIndex messIndex) const;

    //
    virtual void setMessageActive(messageIndex messIndex,bool a);
    virtual bool messageIsActive(messageIndex messIndex) const;

    // Calculate the partition function (after calcProbs())
    virtual void calculatePartition(lbAssignedMeasure_ptr* exactBeliefs = NULL);

    // Determine whether messages must stay dirty after changed
    virtual bool stayDirty() { return _smoothParam != 0; }


    /********************
     Helper methods
    ********************/
    bool getCalculatedBeliefs() { return _calculatedBeliefs; }
    void setCalculatedBeliefs(bool b) { _calculatedBeliefs = b; }

    lbAssignedMeasure_ptr computeBelief(cliqIndex cliq) const;
    varsVec getMovingVars(varsVec const & origVars, lbAssignment & assign);
    lbAssignedMeasure_ptr createMessage(messageIndex messIndex);
    lbAssignedMeasure_ptr multiplyCliqByNeighbors(cliqIndex cliq,
						  varsVec const& vvec,
						  cliquesVec const& exluded = cliquesVec(),
						  bool normalize = false) const;

  private:

    lbMessageBank * _messageBank;

    // Optional manual queue
    messageIndexVec _ordering;

    lbMessageQueueType _messageQueueType;
    lbMessageCompareType _compareType;
    lbMessageInitType _messageInitType;
    bool _maintainBeliefs;
    probType _smoothParam;
    probType _threshold;

    // The class that does statistics tracking:
    friend class lbInferenceMonitor;
    lbInferenceMonitor * _monitor;

    clock_t _timeStart;
    clock_t _timeEnd;

    clock_t _lastUpdateT;

    int _messageCount;
    int _maxMessages;
    int _maxSeconds;
    int _updateSize;
    lbMessageWeightType _WType;

    mutable messageMask _relevant;
    mutable messageMask _active;

    bool _calculatedBeliefs;
    bool _propConverged;

    bool _unzeroCopiedMessages;
  };

  // 
  inline void lbBeliefPropagation::singlePropagationIteration() {
    messageIndexVec miv = _messageBank->getIteration();

    for (uint i = 0; i < miv.size(); i++) {
      messageIndex mi = miv[i];

      if (stoppingCriterionReached()) {
	return;
      }
      
      if (messageIsRelevant(mi)) {
	if (_monitor != NULL) {
	  _monitor->updateStatistics();
	}

	lbAssignedMeasure_ptr newmessage = computeMessage(mi);
	//	cerr << "Computing message " << mi.first << " --> " << mi.second << endl;
	if ( lbOptions::isVerbose(V_FRUSTRATION) )
	  _messageBank->printRealMessages(cerr);
	newmessage->normalize();
	_messageBank->setMessage(mi, newmessage);
	_messageCount++;
      }
    }
  }

  inline bool lbBeliefPropagation::stoppingCriterionReached() {
    if (getMessageCount() % UPDATE_CLOCK_PER_MESSAGES == 0)
      _lastUpdateT = clock();
    
    clock_t nowT = _lastUpdateT;
    
    bool messageStop = (getMessageCount() > getMaxMessages());
    bool timeStop = ((double) (nowT - _timeStart))/CLOCKS_PER_SEC > getMaxSeconds();
    return messageStop || timeStop;
  }

  // Compute a message
  inline lbAssignedMeasure_ptr lbBeliefPropagation::computeMessage(messageIndex messIndex) const {
    varsVec messageVars = getScope(messIndex);
    cliquesVec excluded(1);
    excluded[0] = messIndex.second;

    lbAssignedMeasure_ptr messageTable;
    messageTable = multiplyCliqByNeighbors(messIndex.first, messageVars, excluded);

    return messageTable;
  }

  inline lbAssignedMeasure_ptr lbBeliefPropagation::computeBelief(cliqIndex cliq) const {
    varsVec const& vars = _model.getGraph().getVarsVecForClique(cliq);
    lbAssignedMeasure_ptr belief = multiplyCliqByNeighbors(cliq, vars);
    belief->normalize();
    return belief;
  }

  // Determine if this message is relevant implement me
  inline bool lbBeliefPropagation::messageIsRelevant(messageIndex mi) const {
    return (_relevant.find(mi) == _relevant.end() || _relevant[mi]);
  }

  inline bool lbBeliefPropagation::messageIsActive(messageIndex mi) const {
    return (_active.find(mi) == _active.end() || _active[mi]);
  }

  inline void lbBeliefPropagation::setMessageActive(messageIndex mi,bool a) {
    _active[mi] = a;
    if ( a == false )
      _relevant[mi] = false;
  }

  inline lbAssignedMeasure_ptr lbBeliefPropagation::multiplyCliqByNeighbors(cliqIndex cliq,
									    varsVec const& vars,
									    cliquesVec const& excluded,
									    bool normalize) const {
    varsVec cliqVars =_graph.getVarsVecForClique(cliq);
    lbAssignedMeasure_ptr assignedMeasurePtr = getFactor(cliq)->duplicateValues();

    uint excludedSize = excluded.size();

    for (uint neighborCliqInd = 0; neighborCliqInd < getNeighbors(cliq).size(); neighborCliqInd++) {
      cliqIndex fromCliq = getNeighbors(cliq)[neighborCliqInd];
      bool included = true;
      for (uint c = 0; c < excludedSize; c++) {
	if (fromCliq == excluded[c])
	  included = false;
      }
      messageIndex mi(fromCliq, cliq);
    
      if (included && messageIsRelevant(mi)){
	lbAssignedMeasure_ptr neighborMesPtr = _messageBank->getMessage(mi);
	neighborMesPtr->marginalizeAndMultiply(*assignedMeasurePtr,cliqVars);
      }
    } // neighbors

    // It's important that these be EXACTLY equal or we have to marginalize
    if (vars != cliqVars) {
      if (vars.size() == cliqVars.size()) {
	//cerr << "BEFORE VARIABLE REARRANGING" << endl;
	//assignedMeasurePtr->print(cerr);
      }
      lbAssignedMeasure_ptr tmp = assignedMeasurePtr;
      assignedMeasurePtr = assignedMeasurePtr->marginalize(vars,_measDispatcher);
      delete tmp;
      if (vars.size() == cliqVars.size()) {
	//	cerr << "AFTER VARIABLE REARRANGING" << endl;
	//assignedMeasurePtr->print(cerr);
      }
    }

    if (normalize) {
      assignedMeasurePtr->normalize();
    }

    return assignedMeasurePtr;
  }

  inline void lbBeliefPropagation::randomizeMessage(messageIndex mi) {  
    _messageBank->randomizeMessage(mi);
  }

  inline void lbBeliefPropagation::randomizeMessages() {  
    _messageBank->randomizeMessages();
  }
};

#endif
