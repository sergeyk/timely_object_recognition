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
  Mean Field is an approximate inference procedure that belongs to the wide
  family of 'variational approximations'. More infromation about this
  can be founf in the next excellent tutorial:
  Martin J. Wainwright and Michael I. Jordan (2008) "Graphical Models, Exponential Families, 
  and Variational Inference" Foundations and Trends® in Machine Learning: Vol. 1: No 1–2, pp 1-305.
 */

#include <lbMeanField.h>
#include <climits> // for INT_MAX
using namespace lbLib;

const double MF_THRESH = lbDefinitions::SMALL;

lbMeanField::lbMeanField(lbModel & model,lbMeasureDispatcher const& disp) :
  lbInferenceObject(model,disp),_MFMethod(IM_NORMAL)
{
  reset();
}

lbMeanField::lbMeanField(lbMeanField const& otherMeanField) 
  : lbInferenceObject(otherMeanField),_MFMethod(IM_NORMAL)
{
  assert(false);
}

lbMeanField:: ~lbMeanField() 
{    
  reset();
}

void lbMeanField::reset(bool reBuild)
{
  for (uint i = 0; i < _factors.size(); i++)
    delete _factors[i];
  _factors.clear();
  _measures.clear();
  _evidence = lbAssignment();
  _built = false;

  _calculated = false;
  _calculatedInitialPartition = false; 
  _calculatedPartition = false;

  // We calculate it only when necessary
  _initialPartition = 0;
  _currentPartition = 0;

  initialize() ;
}

void lbMeanField::initialize()
{
  if ( _built ) 
    return;

  //reset();

  // build a measure for each variable
  int N = _model.getGraph().getNumOfVars();
  for ( int i=0 ; i<N ; i++ ) {
    cardVec cards;
    varsVec vars;
    cards.push_back(_model.getCards().getCardForVar(i));
    vars.push_back(i);
    //lbMeasure_Sptr measurePtr = _measDispatcher.getNewMeasure(cards,true); // create random measure
    lbMeasure_Sptr measurePtr = _measDispatcher.getNewMeasure(cards,false); // create uniform marginals
    measurePtr->normalize();
    _measures.push_back(measurePtr);
    _factors.push_back(new lbAssignedMeasure(measurePtr,vars));
  }

  _built = true;
}

void lbMeanField::resetEvidence() {
  setEvidence(lbAssignment());
}
    
void lbMeanField::setEvidence(lbAssignment const& assign) {
  changeEvidence(assign,true);
}

void lbMeanField::changeEvidence(lbAssignment const& assign, bool forceUpdate) 
{
  initialize();
  
  for ( uint i=0 ; i<_factors.size() ; i++ )
    _factors[i]->updateAssign(assign);
  _evidence = assign;
  _calculated = false;
}

bool lbMeanField::calcProbs()
{
  switch ( _MFMethod ) {
  case IM_NORMAL:
    iterate();
    break;
  case IM_RESIDUAL:
    iterateResiduals();
    break;
  case IM_MB_RESIDUAL:
    iterateMBResiduals();
    break;
  case IM_LARGE_EFFECT:
    iterateLargestEffect();
    break;
  default:
    assert(false);
    break;
  }
  return true;
}

void lbMeanField::testRandomOrderings()
{
  _built = false;
  initialize();
  // store starting point
  measurePtrVec initMeasures;
  int N = (int)_measures.size();
  for ( int i=0 ; i<N ; i++ )
    initMeasures.push_back(lbMeasure_Sptr(_measures[i]->duplicate()));

  // do K orderings
  int minIter = INT_MAX;
  int maxIter = 0;
  double avgIter = 0;
  int k;
  for ( k=0 ; k<100 ; k++ ) {
    // copy initial measures into place
    for ( int i=0 ; i<N ; i++ ) {
      _measures[i] = lbMeasure_Sptr(initMeasures[i]->duplicate());
      _factors[i]->replaceMeasure(_measures[i]);
    }
    // iterate
    _calculated = false;
    int iter = iterate();
    avgIter += iter;
    if ( iter < minIter )
      minIter = iter;
    if ( iter > maxIter )
      maxIter = iter;
  }

  // 
  avgIter /= k;
  cerr << "*** Orderings: " << k << "\tMin: " << minIter << "\tMax: " << maxIter << "\tAvg: " << avgIter << " ***" << endl;
}

int lbMeanField::iterateResiduals()
{
  if ( _calculated )
    return 0;

  initialize();

  // compute num of effected marginals for each variable
  vector< set<int> > affected(_measures.size());
  for ( uint i=0 ; i<_measures.size() ; i++ ) {
    cliquesVec const& cliques = _model.getGraph().getAllCliquesForVar(i);
    for ( uint c=0 ; c<cliques.size() ; c++ ) {
      varsVec const& vars = _model.getGraph().getVarsVecForClique(cliques[c]);
      for ( uint v=0 ; v<vars.size() ; v++ )
	if ( vars[v] != (int) i ) 
	  affected[i].insert(vars[v]);
    }
  }

  // create initial "future"
  measurePtrVec futureMeasures;
  multimap< double , int, greater<double> > rqueue;
  int computations = 0;
  for ( uint i=0 ; i<_measures.size() ; i++ ) {
    futureMeasures.push_back(computeMarginal(i));
    lbAssignedMeasure_ptr assignMeas = new lbAssignedMeasure(futureMeasures[i],_factors[i]->getVars());
    double residual = assignMeas->getMaxDiff(*_factors[i]);
    rqueue.insert(std::make_pair(residual,i));
    computations++;
  }

  int updates = 0;
  cerr << "Iterating mean field equations\n";

  while ( rqueue.size() > 0 ) {
      
    double res = rqueue.begin()->first;
    int index = rqueue.begin()->second;
    rqueue.erase(rqueue.begin());
    
    // propagate measures
    lbMeasure_Sptr meas = computeMarginal(index);
    if ( res > MF_THRESH ) {
      // move measure from future into present
      _measures[index] = futureMeasures[index];
      _factors[index]->replaceMeasure(_measures[index]);
      updates++;
      
      // compute new future for all dependent messages
      set<int>::iterator it;
      for ( it=affected[index].begin() ; it!=affected[index].end() ; it++ ) {
	futureMeasures[*it] = computeMarginal(*it);
	lbAssignedMeasure_ptr assignMeas = new lbAssignedMeasure(futureMeasures[*it],_factors[*it]->getVars());
	double residual = assignMeas->getMaxDiff(*_factors[*it]);
	// take old value out of queue
	multimap< double , int, greater<double> >::iterator qit;
	for ( qit=rqueue.begin() ; qit!=rqueue.end() ; qit++ )
	  if ( qit->second == *it ) {
	    rqueue.erase(qit);
	    break;
	  }
	// insert new value
	rqueue.insert(std::make_pair(residual,*it));
	computations++;
      } // affected
      
    } // there was an update
    
  } // queue is not empty
  
  cerr << "Mean field converged after " << updates << " updates\n";
  cerr << "Mean field required " << computations << " computations\n";
  _calculated = true;
  return updates;
}

int lbMeanField::iterateMBResiduals()
{
  cerr << "MB residual iteration is not implemented yet!\n";
  assert(false);
  return 0;
}

int lbMeanField::iterateLargestEffect()
{
  if ( _calculated )
    return 0;

  initialize();

  // compute num of effected marginals for each variable
  vector< set<int> > affected(_measures.size());
  for ( uint i=0 ; i<_measures.size() ; i++ ) {
    cliquesVec const& cliques = _model.getGraph().getAllCliquesForVar(i);
    for ( uint c=0 ; c<cliques.size() ; c++ ) {
      varsVec const& vars = _model.getGraph().getVarsVecForClique(cliques[c]);
      for ( uint v=0 ; v<vars.size() ; v++ )
	if ( vars[v] != (int) i ) 
	  affected[i].insert(vars[v]);
    }
  }

  multimap< double , int , less<double> > rqueue;
  // do one round robin propagation
  int updates = 0;
  for ( uint i=0 ; i<_measures.size() ; i++ ) {
    lbMeasure_Sptr meas = computeMarginal(i);
    lbAssignedMeasure_ptr assignMeas = new lbAssignedMeasure(meas,_factors[i]->getVars());
    double res = assignMeas->getMaxDiff(*_factors[i]);
    if ( res > MF_THRESH ) {
      _measures[i] = meas;
      _factors[i]->replaceMeasure(_measures[i]);
      rqueue.insert(std::make_pair(res,i));
    }
    updates++;
  }

  cerr << "Iterating mean field equations\n";
  while ( rqueue.size() > 0 ) {

    int index = rqueue.begin()->second;
    rqueue.erase(rqueue.begin());

    // iterate messages that are effected by largest change
    set<int>::iterator it;
    for ( it=affected[index].begin() ; it!=affected[index].end() ; it++ ) {
      lbMeasure_Sptr meas = computeMarginal(*it);
      lbAssignedMeasure_ptr assignMeas = new lbAssignedMeasure(meas,_factors[*it]->getVars());
      double res = assignMeas->getMaxDiff(*_factors[*it]);
      if ( res > MF_THRESH ) {
	updates++;
	_measures[*it] = meas;
	_factors[*it]->replaceMeasure(_measures[*it]);
	multimap< double , int , less<double> >::iterator qit;
	for ( qit=rqueue.begin() ; qit!=rqueue.end() ; qit++ ) 
	  if ( qit->second == *it ) {
	    rqueue.erase(qit);
	    break;
	  }
	rqueue.insert(std::make_pair(res,*it));
      } // above threshold
    } // all affected measures
  } // while there is still something in the queue
    
  cerr << "Mean field converged after " << updates << " updates\n";
  _calculated = true;
  return updates;
}

int lbMeanField::iterate()
{
  if ( _calculated )
    return 0;

  initialize();

  // create a random order
  int N = (int)_factors.size();
  vector<bool> inOrder(N,false);
  vector<int> order;
  cerr << "Message order:";
  while ( (int) order.size() < N ) {
    int i = _lbRandomProbGenerator.RandomInt(_factors.size());
    if ( !inOrder[i] ) {
      inOrder[i] = true;
      order.push_back(i);
      cerr << " " << i;
    }
  }
  cerr << endl;
  /*  
  // default order
  order.clear();
  for ( int i=0 ; i<N ; i++ )
    order.push_back(i);
  */

  bool converged = false;
  int updates = 0;
  cerr << "Iterating mean field equations\n";
  while ( ! converged ) {

    converged = true;
    // go over marginals in order and update them
    for ( int o=0 ; o<N ; o++ ) {
      int i = order[o];
      lbMeasure_Sptr meas = computeMarginal(i);
      // compare
      if ( meas->isDifferent(*_measures[i],C_MAX,MF_THRESH) ) {
	converged = false;
	_measures[i] = meas;
	_factors[i]->replaceMeasure(meas);
	updates++;
      }
    }

  } // until convergence

  cerr << "Mean field converged after " << updates << " updates\n"; 
  _calculated = true;
  return updates;
}

lbMeasure_Sptr lbMeanField::computeMarginal(int index)
{
  cardVec cards = _measures[index]->getCards();
  varsVec vars(1,index);
  lbMeasure_Sptr meas = _measDispatcher.getNewMeasure(cards,false);
  lbAssignedMeasure_ptr assignMeas = new lbAssignedMeasure(meas,vars);

  // go over all cliques that include the variable
  cliquesVec const& cliques = _model.getGraph().getAllCliquesForVar(index);
  for ( uint c=0 ; c<cliques.size() ; c++ ) {
    // get potential and probability
    lbAssignedMeasure const& potRef = _model.getAssignedMeasureForClique(cliques[c]);
    varsVec const& fvars = _model.getGraph().getVarsVecForClique(cliques[c]);
    varsVec ovars;
    for ( uint v=0 ; v<fvars.size() ; v++ )
      if ( fvars[v] != index ) {
	ovars.push_back(fvars[v]);
      }
    cardVec ocards = _model.getCardForVars(ovars);
    lbAssignedMeasure_ptr probPtr = NULL;
    if ( ovars.size() ) {
      probPtr = prob(ovars,false);
    }

    // go over the values of the variable
    lbSmallAssignment assign(fvars);
    do { 
      double total = 0.0;
      do {
	// Q(assignment to other variable) log( \psi(assignment to all) ) to marginal
	double qprob = 1.0;
	if ( probPtr != NULL )
	  qprob = probPtr->valueOfFull(assign);
	double lpotential = potRef.logValueOfFull(assign);
	total += lpotential*qprob;
      } while ( assign.advanceOne(ocards,ovars) );
      
      double curValue = assignMeas->logValueOfFull(assign);
      assignMeas->setLogValueOfFull(assign,total+curValue);
    } while ( assign.advanceOne(cards,vars) );

  } // over cliques

  meas->normalize();
  return meas;
}

lbAssignedMeasure_ptr lbMeanField::prob(varsVec const& vars)
{
  return prob(vars,true);
}

lbAssignedMeasure_ptr lbMeanField::prob(varsVec const& vars,
					cliqIndex cliq,
					bool computeProbs)
{ 
  return prob(vars); 
}

lbAssignedMeasure_ptr lbMeanField::prob(varsVec const& vars,bool calc)
{
  if ( calc ) 
    calcProbs();
  // go over variables and multiply measures
  cardVec cards = _model.getCardForVars(vars);
  lbMeasure_Sptr measPtr = _measDispatcher.getNewMeasure(cards,false);
  lbAssignedMeasure_ptr result = new lbAssignedMeasure(measPtr,vars);
  lbSmallAssignment assign(vars);

  do { 
    probType val = 0.0;
    for ( uint v=0 ; v<vars.size() ; v++ ) 
      val += _factors[vars[v]]->logValueOfFull(assign);
    result->setLogValueOfFull(assign,val);
  } while ( assign.advanceOne(cards,vars) );
  
  return result;
}

probType lbMeanField::evidenceLogProb()
{
  calcProbs();
  probType result = 0.0;
  for ( uint i=0 ; i<_factors.size() ; i++ )
    if ( _evidence.isAssigned(i) ) 
      result += _factors[i]->logValueOfFull(_evidence);
  return result;
}

void lbMeanField::printMarginals(int num,bool calc)
{
  if ( calc ) 
    calcProbs();
  cerr << endl;
  cerr << "Q Marginals are:\n";
  cerr << "---------------\n";
  if ( num > (int) _factors.size() )
    num = _factors.size();
  for ( int i=0 ; i<num ; i++ ) {
    cerr << "[" << i << "]";
    varsVec vars(1,i);
    lbSmallAssignment assign(vars);
    do {
      cerr << " " << _factors[i]->valueOfFull(assign);
    } while ( assign.advanceOne(_measures[i]->getCards(),vars) );
    cerr << endl;
  }
}


/*
 * ln Z_mf = sum_a sum_xa (prod_{i\in a} bi) ln psi_a -
 *           - sum_i sum_xi bi ln bi
 */
void lbMeanField::calculatePartition(lbAssignedMeasure_ptr* exactBeliefs)
{
  assert (exactBeliefs==NULL) ;

  //cerr << "[DEBUG] Calculating the partition function" << endl ;

  probType result = 0.0;

  if (getCalculatedPartition()) {
    return;
  }

  // sum_a sum_xa (prod_{i\in a} bi) ln psi_a
  for (cliqIndex cliq = 0; cliq < _model.getGraph().getNumOfCliques(); cliq++){
    //cout << "[DEBUG] calculating partition for clique " << cliq << endl ;
    varsVec cliqVars = _model.getGraph().getVarsVecForClique(cliq);
    lbAssignedMeasure const& meas = getModel().getAssignedMeasureForClique(cliq);
    //count is by default 1. In compact inference we multiply by the number of instances of each cliq.

    lbAssignment assign = lbAssignment();
    assign.zeroise(cliqVars);
    cardVec card = getModel().getCardForVars(cliqVars);

    // for mean field should return: ba = prod_i bi
    lbAssignedMeasure_ptr measBelief = prob(cliqVars, cliq);

    do {
      probType lbelief = measBelief->logValueOfFull(assign);
      probType belief = exp(lbelief);
      probType lprobab = meas.logValueOfFull(assign);

      //cout << "[DEBUG] b=" << belief << " logb=" << lbelief << " e^theta=" << lprobab << endl ;

      if (belief >= lbDefinitions::ZEPSILON) {
        result += belief*lprobab ;
      }
    } while (assign.advanceOne(card,cliqVars));
  }

  // - sum_i sum_xi bi ln bi
  for ( int v=0 ; v<_model.getGraph().getVars().getNumOfVars() ; v++ ) {
    varsVec singleVar ;
    singleVar.push_back((rVarIndex)v) ;
    lbAssignedMeasure_ptr varBeliefs = prob(singleVar);
    lbSmallAssignment assign(singleVar);
    cardVec cards = _model.getCardForVars(singleVar);
    
    do {
      probType lbelief = varBeliefs->logValueOfFull(assign) ;
      probType belief = exp(lbelief) ;
      result -= belief*lbelief ;
    } while ( assign.advanceOne(cards,singleVar) );
  } // for var

  setPartition(result);
  setCalculatedPartition(true);
}
