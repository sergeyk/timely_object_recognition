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


/*!

  This files implement basic (loopy) belief propagation. 
  The basic belief propagation algorithm is based on Pearl (1988).
  It has been shown to work also on graph with loops in Weiss,Murphey & Jordarn (UAI 99).
  Further insights on the reason why it works were srengthen by presentation of the loopy
  belief algorithm as a variational method optimizing the Bethe free Energy  by Yedidia, Freeman & Weiss (2002)

 */

#include <lbPropagationInference.h> 
#include <lbWeightedMessageBank.h>
#include <lbUnweightedMessageBank.h>
#include <lbBeliefPropagation.h>

using namespace lbLib;

/*
 * CONSTRUCTORS / DESTRUCTORS
 */
lbBeliefPropagation::lbBeliefPropagation(lbModel & model, lbMeasureDispatcher const & disp) :
  lbPropagationInference(model, disp) {

  _smoothParam = 0.5;
  _threshold = 1e-5;
  _compareType = C_MAX;
  _messageInitType = MIT_UNIFORM;
  _messageQueueType = MQT_WEIGHTED;
  _monitor = NULL;
  _messageBank = NULL;

  _maxMessages = 10000000;
  _maxSeconds = 10000;
  _updateSize = 1; // Asynchronous
  _WType = MWT_LINF;

  _unzeroCopiedMessages = false;
}

lbBeliefPropagation::~lbBeliefPropagation() {
  delete _messageBank;

  if (_monitor != NULL)
    delete _monitor;
}

lbBeliefPropagation::lbBeliefPropagation(lbBeliefPropagation const& otherPropagation) 
  : lbPropagationInference(otherPropagation) {
  NOT_IMPLEMENTED_YET;
}

void lbBeliefPropagation::setOptions(lbOptions & opt, int argc, char *argv[]) {
  int compareType = _compareType;
  int messageQueueType = _messageQueueType;
  int messageInitType = _messageInitType;
  double smoothParam = _smoothParam;
  double threshold = _threshold;
  int WType = _WType;

  lbPropagationInference::setOptions(opt, argc, argv);
  opt.addIntOption("Iqt", &messageQueueType, "queue type (0 - UNWEIGHTED, 1 - WEIGHTED, 2 - MANUAL");
  opt.addDoubleOption("Is", &smoothParam, "s < 0 is acceleration, s == 0 is no smoothing, 0 < s < 1 smoothing");
  opt.addIntOption("Ius", &_updateSize, "max number of messages (0 for synchronous)");
  opt.addIntOption("Imm", &_maxMessages, "max number of messages");
  opt.addIntOption("Ims", &_maxSeconds, "max number of seconds");
  opt.addIntOption("Ict", &compareType, "message compare type 0 - AVG, 1 - MAX, 2 - KL, 3 - C_AVG_LOG, 4 - C_MAX_LOG");
  opt.addIntOption("Iwt", &WType, "message weight type 0 - L1, 1 - L2, 2 - L_INF");
  opt.addIntOption("Iit", &messageInitType, "message init type 0 - RANDOM, 1 - UNIFORM");
  opt.addDoubleOption("It", &threshold, "threshold measure differences");
  opt.setOptions(argc, argv);

  if (compareType < 0 || compareType > 4) {
    opt.usageError("Compare type must be 0 - AVG, 1 - MAX, or 2 - KL, 3 - C_AVG_LOG, 4 - C_MAX_LOG");
  }
  else {
    _compareType = (lbMessageCompareType) compareType;
  }

  if (WType < 0 || WType > 2) {
    opt.usageError("Message weight type type must be 0 - L1, 1 - L2, or 2 - L_INF");
  }
  else {
    _WType = (lbMessageWeightType) WType;
  }

  if (messageQueueType < 0 || messageQueueType > 2) {
    opt.usageError("Queue type must be 0 - Unweighted, 1 - Weighted, or 2 - Manual");
  }
  else {
    _messageQueueType = (lbMessageQueueType) messageQueueType;
  }

  if (messageInitType < 0 || messageInitType > 2) {
    opt.usageError("Message init type must be 0 - RANDOM or 1 - UNIFORM");
  }
  else {
    _messageInitType = (lbMessageInitType) messageInitType;
  }

  if (smoothParam != 1.0) {
    _smoothParam = (probType) smoothParam;
  }
  else {
    opt.usageError("smooth param shouldn't be 1.0");
  }

  if (threshold >= 0) {
    _threshold = (probType) threshold;
  }
  else {
    opt.usageError("threshold must be >= 0");
  }
}

lbMessageBank * lbBeliefPropagation::getNewMessageBank() {
  lbMessageBank * bank = NULL;

  if (_messageQueueType == MQT_WEIGHTED) {
    bank = new lbWeightedMessageBank(localMessagesAdjList(),_WType);
  }
  else if (_messageQueueType == MQT_UNWEIGHTED) {
    assert(_ordering.empty());
    bank = new lbUnweightedMessageBank(localMessagesAdjList(), _ordering);
  }
  else if (_messageQueueType == MQT_MANUAL) {
    assert(!_ordering.empty());
    bank = new lbUnweightedMessageBank(localMessagesAdjList(), _ordering);
  }
  else {
    NOT_REACHED;
  }

  bank->setSmoothing(_smoothParam);
  bank->setUpdateSize(_updateSize);
  bank->setThreshold(_threshold);
  bank->setCompareType(_compareType);

  return bank;
}

void lbBeliefPropagation::initialize(bool allocate,bool useOldInfo) {
  lbPropagationInference::initialize(allocate);
  resetMessages(false);
}

void lbBeliefPropagation::resetMessages(bool useOld)
{
  lbMessageBank* bank;
  setCalculatedBeliefs(false);
  _propConverged = false;
  _messageCount = 0;
  useOld = useOld && (_messageBank != NULL);
  bank = getNewMessageBank();
  
  adjListVec const & neighborsVecs = localMessagesAdjList();
  cliqIndex fromCliq;
  
  // Pushing all messages from leaf variables on first
  for (fromCliq = 0; fromCliq < (int) neighborsVecs.size(); fromCliq++) {
    cliquesVec const& neighbors = neighborsVecs[(int) fromCliq];

    if ((int) neighbors.size() == 1) {
      messageIndex mi(fromCliq, (cliqIndex) neighbors[0]);
      if (useOld) {
	lbAssignedMeasure_ptr amp = _messageBank->getMessage(mi);
	if (amp) {
	  bank->initMessage(mi, amp->duplicate());
	} else {
	  bank->initMessage(mi, createMessage(mi));
	}
      } else {
	bank->initMessage(mi, createMessage(mi));
      }
    }
  }
  
  // Pushing all other messages on
  for (fromCliq = 0; fromCliq < (int) neighborsVecs.size(); fromCliq++) {
    cliquesVec const& neighbors = neighborsVecs[(int) fromCliq];
    
    if (neighbors.size() != 1) {
      for (uint i = 0; i < neighbors.size(); i++) {
	messageIndex mi(fromCliq, (cliqIndex) neighbors[i]);
	if (useOld) {
	  lbAssignedMeasure_ptr amp = _messageBank->getMessage(mi);
	  if (amp) {
	    bank->initMessage(mi, amp->duplicate());
	  } else {
	    bank->initMessage(mi, createMessage(mi));
	  }
	} else {
	  bank->initMessage(mi, createMessage(mi));
	}
      }
    }
  }
  if (_messageBank) {
    delete _messageBank;
  }
  _messageBank = bank;
}

lbAssignedMeasure_ptr lbBeliefPropagation::createMessage(messageIndex mi) {

  // create new message
  varsVec separator = getScope(mi);
  cardVec separatorCards = _model.getCardForVars(separator);

  lbAssignedMeasure_ptr message;
  lbMeasure_Sptr measure;

  if (_messageInitType == MIT_RANDOM) {
    measure = _measDispatcher.getNewMeasure(separatorCards, true);
  }
  else if(_messageInitType == MIT_UNIFORM) {
    measure = _measDispatcher.getNewMeasure(separatorCards, false);
  }

  message = new lbAssignedMeasure(measure, separator);
  message->normalize();

  return message;
}

void lbBeliefPropagation::setSmoothing(double param){
  if (param <= 1 && param >= 0) {
    _smoothParam = param;
  }
  else {
    cerr << "ERROR : Smooth parameter should be between 0 and 1" << endl;
  }
}

/*
 * We can change only part of the evidence if the 
 * other nodes are marked -1.
 */
void lbBeliefPropagation::changeEvidence(lbAssignment const& newEvidence, bool forceUpdate) 
{
  const lbAssignment oldEvidence = getEvidence();
  lbPropagationInference::changeEvidence(newEvidence, forceUpdate);
  varsVec allVars = getModel().getGraph().getVars().getVarsVec();
  if (oldEvidence.equals(newEvidence, allVars) && !forceUpdate)
    return;
  setCalculatedBeliefs(false);

  if (lbOptions::isVerbose(V_EVIDENCE)) {
    cerr << "Changing evidence... " << endl;
    cerr << "from: " << oldEvidence.getSize() << endl;
    oldEvidence.print(cerr, _graph.getNumOfVars());
    cerr << "to: " << newEvidence.getSize() << endl;
    newEvidence.print(cerr, getModel().getGraph().getNumOfVars());
  }

  // Get a new message bank so that propagation will start over. That
  // said we don't want to start with random messages, so we'll keep
  // the messages the same as before except with evidence added, when
  // we initialize the new bank.
  adjListVec const & neighborsVecs = localMessagesAdjList();
  cliqIndex fromCliq;
  lbMessageBank * bankWithEvidence = getNewMessageBank();

  // Go over all clique messages:
  for (fromCliq = 0; fromCliq < (int) neighborsVecs.size(); fromCliq++) {
    cliquesVec const& neighbors = neighborsVecs[(int) fromCliq];
    for (uint i = 0; i < neighbors.size(); i++) {
      messageIndex mi(fromCliq, (cliqIndex) neighbors[i]);
      bool createNew = false;
      if ( _messageBank == NULL ) 
	createNew = true;
      else if ( _messageBank->getMessage(mi) == NULL ) 
	createNew = true;
      else {
	const varsVec& messVars = _messageBank->getMessage(mi)->getVars();
	if (!oldEvidence.matches(newEvidence, messVars)) 
	  /* Every variable in the message and assigned in oldEvidence is
	     assigned the same thing in newEvidence, so keep previously zeroed-out values: */
	  createNew = true;
      }
      if ( ! createNew ) {
	bankWithEvidence->initMessage(mi, _messageBank->getMessage(mi)->duplicateValues());
	if (_unzeroCopiedMessages)
          bankWithEvidence->getMessage(mi)->replaceValues(0, 1); //change all 0s to 1s
      }
      else {
	bankWithEvidence->initMessage(mi, createMessage(mi));
      }

      lbAssignedMeasure_ptr messPtr = bankWithEvidence->getMessage(mi);
      const varsVec& messVars = messPtr->getVars();
      if (getEvidence().areAssigned(messVars) || (!messageIsActive(mi)) ) {
	_relevant[mi] = false;
      } else {
	_relevant[mi] = true;
	
	if (getEvidence().hasAnyAssigned(messVars))
	  //otherwise, updateAssign() will not zero out anything new:
	  messPtr->updateAssign(getEvidence(), *messPtr);
      } // relevant

    } // neighbors
  } // cliques

  if ( _messageBank != NULL ) 
    delete _messageBank;
  _messageBank = bankWithEvidence;
}

/*
  Using a Region based free energy approximation :
  - ln Z = F(region) = sum of C(r)*F(r) over all regions r

  F(r) = U(r) - H(r)

  U(r) = sum over x(r) { b(r)ln(E(r)) }
  = - sum over x(r) {sum over all factors c in r {ln(psi(c)) }
  and if each region is a factor  U(r) = sum over x(r) { b(r)ln(psi(r) }

  H(r) = - sum over x(r) { b(r) ln b(r) }

  and we set C(r) for each clique to be 1
  and for each message c(r) is -1
  thus we get that each factor is counted once, and each var is counted once.
*/
void lbBeliefPropagation::calculatePartition(lbAssignedMeasure_ptr* exactBeliefs) {

  probType result = 0.0;

  if (getCalculatedPartition()) {
    return;
  }
  probType entropyResult = 0.0;

  for (cliqIndex cliq = 0; cliq < getGraph().getNumOfCliques(); cliq++){
    varsVec cliqVars = getGraph().getVarsVecForClique(cliq);
    lbAssignedMeasure const& meas = getModel().getAssignedMeasureForClique(cliq);

    lbAssignment assign = lbAssignment();
    assign.zeroise(cliqVars);
    varsVec movingVars = getMovingVars(cliqVars,assign);
    cardVec card = getModel().getCardForVars(movingVars);
    lbAssignedMeasure_ptr measBelief;

    if (exactBeliefs) {
      measBelief = exactBeliefs[cliq];
    }
    else {
      measBelief = prob(cliqVars, cliq);
    }

    do {
      probType lbelief = measBelief->logValueOfFull(assign);
      probType belief = exp(lbelief);
      probType lprobab = meas.logValueOfFull(assign);

      if (belief >= lbDefinitions::ZEPSILON) {//&& lprobab >= log(lbDefinitions::ZEPSILON) ) {//do not limit model's measure values
	probType negEntropyContrib = belief * lbelief;
        entropyResult -= negEntropyContrib;
        result += negEntropyContrib;
        result -= belief*lprobab;
      }
    } while (assign.advanceOne(card,movingVars));

    assign.zeroise(movingVars);
    if(!exactBeliefs)
      delete measBelief;

    // For each message coming into this cliq, substract its entropy
    // so it won't be counted twice.
    for (uint fromCliqInd=0;fromCliqInd<localSizes()[cliq];fromCliqInd++){
      cliqIndex fromCliq = localMessagesAdjList()[cliq][fromCliqInd];

      if (cliq < fromCliq) {   // so we won't count the same message twice
	varsVec messageVars = getScope(messageIndex(fromCliq, cliq));
	lbAssignment assign = lbAssignment();
	assign.zeroise(messageVars);

	lbAssignedMeasure_ptr measBeliefMessage;
	if (exactBeliefs) {
	  measBeliefMessage = exactBeliefs[cliq]->marginalize(messageVars,_measDispatcher); 
	}
	else {
	  measBeliefMessage = prob(messageVars,cliq);
	}

	varsVec movingMessageVars = getMovingVars(messageVars,assign);	
	cardVec card = getModel().getCardForVars(movingMessageVars);

	do {
	  probType lbelief = measBeliefMessage->logValueOfFull(assign);
	  probType belief = exp(lbelief);
	  
	  if (belief >= lbDefinitions::ZEPSILON ) {
	    probType negEntropyContrib = belief * lbelief;
            result -= negEntropyContrib;
            entropyResult += negEntropyContrib;
	  }
	} while (assign.advanceOne(card,movingMessageVars));

	delete measBeliefMessage;
      }
    }
  }

  if (!exactBeliefs) {
    setPartition(-result);
    setCalculatedPartition(true);
    setEntropy(entropyResult);
  }
}

// Returns current belief about cliq
lbAssignedMeasure_ptr lbBeliefPropagation::getBelief(cliqIndex cliq) const {
    return computeBelief(cliq);
}

varsVec lbBeliefPropagation::getMovingVars(varsVec const & origVars, lbAssignment & assign) {
  varsVec movingVars = varsVec();

  if ( ! getEvidence().isEmpty() ) {
    for (rVarIndex var=0;(uint)var<origVars.size();var++){
      if (getEvidence().isAssigned(origVars[var]))
	assign.setValueForVar(origVars[var],getEvidence().getValueForVar(origVars[var]));
      else 
	movingVars.push_back(origVars[var]);
    }
  } 
  else {
    movingVars = varsVec(origVars);
  }

  return movingVars;

}

bool lbBeliefPropagation::calcProbs() {
  
  if (!isBuilt()) {
    reset();
  }
   
  if (_calculatedBeliefs) {
    //  cerr<<"In belief propagation, (not computed) converged ? "<<_propConverged<<endl;
    return _propConverged;
  }
  if (areFactorsUpdated() && _messageBank->getIteration().size() == 0) {
    // TODO: FIX THIS?
    resetMessages(true);
  }
  if (lbOptions::isVerbose(V_PROPAGATION)) {
    cerr << "Calc probs for model with " << _messageBank->getQueue()->size() << " messages." << endl;
  }
  
  /*
  // DEBUG
  cerr << "[DEBUG] In calcProbs with cns: " ;
  const vector<double> & curCountNums = getModel().getCountingNums() ;
  for (uint i=0 ; i<curCountNums.size() ; ++i) {
    cerr << curCountNums[i] << " " ;
  }
  cerr << endl ;
  // END DEBUG
  */

  _timeStart = clock();
  _messageCount = 0;
  _lastUpdateT = _timeStart;
  
  _propConverged = true;
  
  do {
    if (stoppingCriterionReached()) {
      _propConverged = false;
      break;
    }
    
    singlePropagationIteration();
  } while (_messageBank->update());
  
  _timeEnd = clock();
  setCalculatedBeliefs(true);
  setFactorsUpdated(false);  
  if (_monitor != NULL) {
    _monitor->finishStatistics();
  }
  //  cerr<<"In belief propagation (computed), converged ? "<<_propConverged<<endl;
  return _propConverged;
} 

probVector lbBeliefPropagation::getNegMutualInfos (const cliquesVec & cliqs) {

  probVector res ;

  for (uint i=0 ; i<cliqs.size() ; ++i) {
    //cerr << "[DEBUG] In getNegMutualInfos, cliq=" << i << endl ;
    cliqIndex cliq = cliqs[i] ;
    varsVec cliqVars = getGraph().getVarsVecForClique(cliq);
    lbAssignment assign = lbAssignment();
    assign.zeroise(cliqVars);
    varsVec movingVars = getMovingVars(cliqVars,assign);
    cardVec card = getModel().getCardForVars(movingVars);
    lbAssignedMeasure_ptr measBelief = prob(cliqVars, cliq) ;

    int curPos = (int)res.size() ;
    res.push_back (0.0) ;

    // H_factor:
    do {
      probType lbelief = measBelief->logValueOfFull(assign);
      probType belief = exp(lbelief);

      if (belief >= lbDefinitions::ZEPSILON) {//&& lprobab >= log(lbDefinitions::ZEPSILON) ) {//do not limit model's measure values
	probType negEntropyContrib = belief * lbelief;
        res[curPos] -= negEntropyContrib ;
        //cerr << "[DEBUG] b=" << belief << " lnb=" << lbelief << " blnb=" << negEntropyContrib << endl ;
      }
    } while (assign.advanceOne(card,movingVars));
    delete measBelief;

    // - sum_i Hi
    for (uint j=0 ; j<cliqVars.size() ; ++j) {
      //cerr << "[DEBUG] In getNegMutualInfos, cliqVar=" << j << endl ;      
      varsVec singleVar (1, cliqVars[j]) ;
      assign = lbAssignment();
      assign.zeroise(singleVar);
      movingVars = getMovingVars(singleVar,assign);
      card = getModel().getCardForVars(movingVars);
      lbAssignedMeasure_ptr measBeliefVar = prob (singleVar) ;
      
      do {
        probType lbelief = measBeliefVar->logValueOfFull(assign);
        probType belief = exp(lbelief);

        if (belief >= lbDefinitions::ZEPSILON) {//&& lprobab >= log(lbDefinitions::ZEPSILON) ) {//do not limit model's measure values
          probType negEntropyContrib = belief * lbelief;
          res[curPos] += negEntropyContrib ;
          //cerr << "[DEBUG] b=" << belief << " lnb=" << lbelief << " blnb=" << negEntropyContrib << endl ;
        }
      } while (assign.advanceOne(card,movingVars));

      delete measBeliefVar;
    }
  }

  return res ;
}
