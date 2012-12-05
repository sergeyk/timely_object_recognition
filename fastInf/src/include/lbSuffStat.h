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

#ifndef _Loopy__Suff_Stat
#define _Loopy__Suff_Stat

#include <lbInferenceObject.h>
#include <lbAssignedMeasure.h>
#include <lbModelListener.h>
#include <lbTableMeasure.h>

namespace lbLib {

  class lbSuffStat : public lbModelListener {
  public:
    
    lbSuffStat(lbInferenceObject& infObj,string const& evidenceFileName,set<measIndex> const& measSet);

    lbSuffStat(lbInferenceObject& infObj,set<measIndex> const& measSet);

    virtual ~lbSuffStat();      

    inline const lbInferenceObject & getInfObj() const ; 
    
    void setEvidence (string evidenceFileName);
    
    inline virtual lbTableMeasure<lbValue> const& getEmpiricalExpectation(measIndex meas,bool recalc = true) const;

    inline virtual lbTableMeasure<lbValue> const& getEstimatedExpectation(measIndex meas,bool recalc = true) const;
    virtual lbMeasure_Sptr getEstimatedSquaredExpectation(measIndex meas) const;
    virtual lbMeasure_Sptr getEstimatedExpectationSquared(measIndex meas) const;

    inline virtual bool factorsUpdated(measIndicesVec const& vec);

    /*
      estimation of ll by the free energy given the model parameters  
      computed via diffs in the partition function (with and
      without evidence).
      with full evidence, _llikelihood and _expllikelihood will be simialr
     */
    virtual probType getLogLikelihood() const ;
    virtual probType getLog2Likelihood() const {
      return (getLogLikelihood() / log(2.0)) ;
    }

    /*!
      multiplication of param vector by empirical counts 
      normalized by the partition function
      with full evidence, _llikelihood and _expllikelihood will be simialr
     */
    virtual probType getExpectedLogLikelihood() const;
    virtual probType getExpectedLog2Likelihood() const {
      return getExpectedLogLikelihood()/log(2.0) ;
    }

    inline void setMeasureOfInterest(set<measIndex> const& measSet);

    inline void activateEMMode();

    inline virtual int getNumOfEvidence() const;
    
    inline void setEMMode(bool set);
    inline bool getEMMode() const;

    inline void print(ostream & out) const;
    inline void printEstimatedCounts(ostream & out) const;
    inline void printExpectedCounts(ostream & out) const;

    inline int getMeasVecIndex(measIndex meas) const;
    inline measIndex getMeasRealIndex(int index) const;
    inline int measureNum() const;

    inline virtual void resetCounts(bool resetEmpirical = true);

  protected:
    virtual void calcEstimatedCounts() const;

    virtual void calcLogLikelihood() const;

    virtual void calcExpectedLogLikelihood() const;

    virtual void calcEmpiricalCounts() const;

    virtual void resetEstimatedSuffStat() const;

    virtual void resetEmpiricalSuffStat() const;

  protected:

    lbInferenceObject& _infObj;
    fullAssignmentPtrVec _evidence;

    set<measIndex> _measSet;
    
    mutable probType _lLikelihood;
    mutable probType _explLikelihood;
    mutable probType* _empirCountsVec;
    mutable measurePtrVec _empirCounts;
    mutable measurePtrVec _estimatedCounts;

    mutable bool _empiricalComputed;
    mutable bool _estimatedComputed;
    mutable bool _llComputed;
    mutable bool _expllComputed;
    mutable bool _countsInitialized;

    bool _EMMode;
    
  private:

    void Init();

    void readEvidenceFromFile(string evidenceFileName);
    
    void initCounts();

  private:

  };

    
  inline lbTableMeasure<lbValue> const& lbSuffStat::getEmpiricalExpectation(measIndex meas,bool recalc) const {
    if ( recalc ) 
      calcEmpiricalCounts();
    return (lbTableMeasure<lbValue> const&) *(_empirCounts[getMeasVecIndex(meas)]);
  }

  inline lbTableMeasure<lbValue> const& lbSuffStat::getEstimatedExpectation(measIndex meas,bool recalc) const {
    if ( recalc ) 
      calcEstimatedCounts();
    return (lbTableMeasure<lbValue> const&) *(_estimatedCounts[getMeasVecIndex(meas)]);
  }

  inline bool lbSuffStat::factorsUpdated(measIndicesVec const& vec) {
    resetCounts(!_EMMode);
    return true;
  }

  inline void lbSuffStat::resetCounts(bool resetEmpirical) {
    if ( resetEmpirical ) {
      _empiricalComputed = false;
    }
    _llComputed = false;
    _expllComputed = false;
    _estimatedComputed = false;
    _infObj.reset();
  }

  inline void lbSuffStat::setMeasureOfInterest(set<measIndex> const& measSet) {
    _measSet=measSet;
  }

  inline void lbSuffStat::activateEMMode() {
    _EMMode=true;
  }

  inline const lbInferenceObject & lbSuffStat::getInfObj() const {
    return _infObj;
  }

  inline int lbSuffStat::getNumOfEvidence() const{
    return _evidence.size();
  }
  
  inline void lbSuffStat::setEMMode(bool set) {
    _EMMode=set;
  }

  inline bool lbSuffStat::getEMMode() const {
    return _EMMode;
  }

  inline void lbSuffStat::print(ostream & out) const {
    cerr << "Estimated Counts: " << endl;
    printEstimatedCounts(cerr);
    cerr << "Expected Counts: " << endl;
    printExpectedCounts(cerr);
  }
  
  inline void lbSuffStat::printEstimatedCounts(ostream & out) const {
    calcEstimatedCounts();
    for (uint i = 0; i < _estimatedCounts.size(); i++) {
      _estimatedCounts[i]->print(cerr);
      cerr << endl;
    }
  }
  
  inline void lbSuffStat::printExpectedCounts(ostream & out) const {
    calcEmpiricalCounts();
    for (uint i = 0; i < _empirCounts.size(); i++) {
      _empirCounts[i]->print(cerr);
      cerr << endl;
    }
  }

  inline int lbSuffStat::getMeasVecIndex(measIndex meas) const {
    if ( _measSet.size() == 0 )
      return meas;

    int i=0;
    for ( set<measIndex>::const_iterator it=_measSet.begin() ; it!=_measSet.end() ; it++,i++ )
      if ( *it == meas )
	return i;

    return -1;
  }

  inline measIndex lbSuffStat::getMeasRealIndex(int index) const {
    if ( _measSet.size() == 0 )
      return index;

    int i=0;
    for ( set<measIndex>::const_iterator it=_measSet.begin() ; it!=_measSet.end() ; it++,i++ )
      if ( index == i )
	return *it;

    return -1;
  }

  inline int lbSuffStat::measureNum() const {
    if ( _measSet.size() == 0 )
      return _infObj.getModel().getNumOfMeasures();
    else
      return _measSet.size();
  }
  
}

#endif //_Loopy__Suff_Stat
