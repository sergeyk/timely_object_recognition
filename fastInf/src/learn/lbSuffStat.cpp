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

#include "lbSuffStat.h"
#include <lbTableMeasure.h>


using namespace lbLib ;

lbSuffStat::lbSuffStat(lbInferenceObject& infObj,string const& evidenceFileName,
                       set<measIndex> const& measSet) 
  : lbModelListener(infObj.getModel()),
    _infObj(infObj),
    _measSet(measSet)
{
  readEvidenceFromFile(evidenceFileName);
  Init();
}

lbSuffStat::lbSuffStat(lbInferenceObject& infObj,set<measIndex> const& measSet) 
  : lbModelListener(infObj.getModel()),
    _infObj(infObj),
    _measSet(measSet)
{
  Init();
}

void lbSuffStat::Init()
{
  initCounts();
  _lLikelihood=0;
  _empiricalComputed = false;
  _estimatedComputed = false;
  _llComputed = false;
  _expllComputed = false;
  _countsInitialized = false;	
  calcEmpiricalCounts();
}

void lbSuffStat::readEvidenceFromFile(string evidenceFileName) {

  ifstream_ptr in;
  try {
    in = ifstream_ptr(new ifstream(evidenceFileName.c_str()));
  }
  catch (...){
    cerr<<"error while reading file: "<<evidenceFileName<<endl;
  }
  _evidence = fullAssignmentPtrVec();
  bool isEvidenceFull = true;
  while (!in->eof()) {
    lbFullAssignment_ptr assign(new lbFullAssignment());
    if (assign->readAssignmentFromFile(*in,_infObj.getModel().getGraph().getNumOfVars())) {
      _evidence.push_back(assign);
      //check whether this is partial assignment
      if (!isEvidenceFull) //unless some evidence is already partial and checked this flag
	if (!(assign->areAssigned(_infObj.getGraph().getVars().getVarsVec())))
	  isEvidenceFull=false;
    }
    else {
      cerr<<"Error reading assignment from file "<<evidenceFileName<<endl;
      delete assign;
    }
  }
  //after reading all evidence, if all are full, no need to compute partition function or each evidence
  if (isEvidenceFull) {
    _EMMode = false;
  }
  else {
    _EMMode = true;
  }

  cerr<<"num of evidences "<<_evidence.size()<<endl;
  in->close();
  _empiricalComputed = false;
  _estimatedComputed = false;
  _llComputed = false;
  _expllComputed = false;
  _countsInitialized = false;
}

void lbSuffStat::initCounts(){
  //make all measures for stroing counts
  lbModel const& model = _infObj.getModel();
  int len = measureNum() ;
  _empirCounts = measurePtrVec(len);
  _estimatedCounts = measurePtrVec(len);
  for (int i=0;i<len;i++) {
    measIndex meas = getMeasRealIndex(i) ;
    _empirCounts[i] = lbMeasure_Sptr(new lbTableMeasure<lbValue>(model.getMeasure(meas).getCards(),false));
    _empirCounts[i]->makeZeroes();
    _estimatedCounts[i] = lbMeasure_Sptr(new lbTableMeasure<lbValue>(model.getMeasure(meas).getCards(),false));
  }

  _empirCountsVec=NULL;
  int lenEmpir = _model.getSize(false,_measSet);
  _empirCountsVec = new probType[lenEmpir];
}

lbSuffStat::~lbSuffStat() {
  for (uint i=0;i<_evidence.size();i++)
    if(_evidence[i])
      delete _evidence[i];
  if (_empirCountsVec)
    delete[] _empirCountsVec;
}


void lbSuffStat::calcEstimatedCounts() const {

  if ( _estimatedComputed ) 
    return;

  resetEstimatedSuffStat();
  lbModel& model = _infObj.getModel();

  lbGraphStruct const& graph = model.getGraph();
  //cerr<<"Collecting estim counts"<<endl;
  for (cliqIndex cliq=0;cliq<_graph.getNumOfCliques();cliq++){
    varsVec vars = graph.getVarsVecForClique(cliq);
    //find the measure of this clique
    measIndex meas = model.getMeasureIndexForClique(cliq);
    //get estimated count
    if (_measSet.size() == 0 || _measSet.find(meas)!=_measSet.end()) {
      lbAssignedMeasure_ptr assignedMeasPtr = _infObj.prob(vars,cliq);
      lbTableMeasure<lbValue> tab((lbTableMeasure<lbLogValue> const&)assignedMeasPtr->getMeasure());
      delete assignedMeasPtr;
      tab.multiplyMeasureByNumber(getNumOfEvidence());
      _estimatedCounts[getMeasVecIndex(meas)]->addMeasure(tab);
    }
  }
  
  _estimatedComputed = true;
}

void lbSuffStat::calcEmpiricalCounts() const {

  if ( _empiricalComputed && _countsInitialized )
    return; 

  _lLikelihood=0;
  resetEmpiricalSuffStat();
  lbModel& model = _infObj.getModel();
  lbGraphStruct const& graph = model.getGraph();
  
  for (uint evidIndex =0;evidIndex<_evidence.size();evidIndex++) {
    if (isVerbose(V_TEMPORARY)) {
      cerr << "Changing evidence" << endl;
    }

    _infObj.changeEvidence(*(_evidence[evidIndex]));
    for (cliqIndex cliq=0;cliq<_graph.getNumOfCliques();cliq++){
      varsVec vars = graph.getVarsVecForClique(cliq);
      measIndex meas = model.getMeasureIndexForClique(cliq);
      if (_measSet.size() == 0 || _measSet.find(meas)!=_measSet.end()) {
        lbAssignedMeasure_ptr assignedMeasPtr = _infObj.prob(vars,cliq);
        lbTableMeasure<lbValue> tab((lbTableMeasure<lbLogValue> const&)assignedMeasPtr->getMeasure());
        delete assignedMeasPtr;
        _empirCounts[getMeasVecIndex(meas)]->addMeasure(tab);
      }
    }
    _lLikelihood +=_infObj.evidenceLogProb();
  }

  //cerr << "*** LL in SScalcEmpirical is " << _lLikelihood << endl;

  _infObj.resetEvidence();
  
  int len=_model.getSize(false,_measSet);
  for (int i=0;i<len;i++)
    _empirCountsVec[i] = 0;
  int index =0;
  for (measIndex meas=0;meas<measureNum();meas++) {
    int tmp = _empirCounts[getMeasVecIndex(meas)]->extractValuesAddToVector(_empirCountsVec,index,false);
    index+=tmp;
  }

  _countsInitialized = true;
  _empiricalComputed = true;
  _llComputed = true;
}

void lbSuffStat::resetEstimatedSuffStat() const
{
  for (measIndex meas=0;meas<measureNum();meas++)
    _estimatedCounts[meas]->makeZeroes();
}

void lbSuffStat::resetEmpiricalSuffStat() const
{
  for (measIndex meas=0;meas<measureNum();meas++)
    _empirCounts[meas]->makeZeroes();
}

void lbSuffStat::setEvidence (string evidenceFileName){
  for (uint i=0;i<_evidence.size();i++)
    if (_evidence[i])
      delete _evidence[i];
  readEvidenceFromFile(evidenceFileName);
}

probType lbSuffStat::getLogLikelihood() const { 

  if ( _llComputed )
    return _lLikelihood;

  if (_EMMode) {
      calcLogLikelihood();
      return _lLikelihood;
  }
  else {
    //if evidence is full, no need to assign each evidence from the beginning
    return getExpectedLogLikelihood();
  }
}

probType lbSuffStat::getExpectedLogLikelihood() const { 
  
  calcEmpiricalCounts();
  // in case empirical counts are updated, but ll needs to be recomputed
  // i.e when model is updated and there is full evidence
  calcExpectedLogLikelihood();
  /// cerr << "+++ Expected LL is " << _explLikelihood << endl;
  
  return _explLikelihood;
}


void lbSuffStat::calcLogLikelihood() const
{
  if ( _llComputed )
    return ;

 _lLikelihood=0;
  for (unsigned int evidIndex =0;evidIndex<_evidence.size();evidIndex++) {
    _infObj.changeEvidence(*(_evidence[evidIndex]));
    _lLikelihood +=_infObj.evidenceLogProb();
  }
  _infObj.resetEvidence();
  _llComputed = true;
}

void lbSuffStat::calcExpectedLogLikelihood() const {

  if ( _expllComputed )
    return;

  calcEmpiricalCounts();
  int len;
  probType* modelParamsVec = _model.getLogParamVector(len,false,_measSet);

  double like=0;
  for (int i=0;i<len;i++)
    if ( _empirCountsVec[i] >= 0.0 && ~isnan(modelParamsVec[i]) )
    like+=_empirCountsVec[i]*modelParamsVec[i];
  
  _explLikelihood = like - getNumOfEvidence()*_infObj.initialPartitionFunction();
  delete[] modelParamsVec;

  //  cerr << "*** Exp LL in SScalcExpected is " << _explLikelihood << endl;

  _expllComputed = true;
}


lbMeasure_Sptr lbSuffStat::getEstimatedSquaredExpectation(measIndex meas) const {
  calcEstimatedCounts();
  lbMeasure_Sptr stab = getEstimatedExpectation(meas).duplicate();
  return stab;
}

lbMeasure_Sptr lbSuffStat::getEstimatedExpectationSquared(measIndex meas) const {
  calcEstimatedCounts();
  lbMeasure_Sptr tab = getEstimatedExpectation(meas).duplicate();
  tab->multiplyMeasureByNumber(1.0/getNumOfEvidence());
  tab->raiseToThePower(2.0);
  tab->multiplyMeasureByNumber(getNumOfEvidence());
  return tab;
}
