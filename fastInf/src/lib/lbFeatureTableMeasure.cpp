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

#include <lbFeatureTableMeasure.h>
#include <lbRandomProb.h>
#include <lbTableMeasure.h>

using namespace lbLib;

/**
   This Class holds the measure for the graph

   This Object holds the probabilistic characters of a clique.
   and performs simple actions on it
   
   Part of the loopy belief library
   @version November 2002
   @author Ariel Jaimovich
*/

void lbFeatureTableMeasure::setFeatureValuesOfFull(lbBaseAssignment const& assign,
						   varsVec const& vars,
						   safeVec<lbFeatureValue> const & featureValues) {
  int index = computeIndex(assign, vars);
  _featureValues[index] = featureValues;
}
  

lbFeatureTableMeasure::lbFeatureTableMeasure(lbTableMeasure<lbLogValue> const& oldMeasure) :
 lbMultinomialMeasure(oldMeasure.getCards())
  {
  _featureValues.clear();
  _paramIndices.clear();

  safeVec<lbFeatureValue> oneVec;
  lbFeatureValue one;
  one.setValue(1.0);
  oneVec.push_back(one);

  // go over cells
  probType* p = new probType[oldMeasure.getSize()];
  int i;
  for ( i=0 ; i<oldMeasure.getSize() ; i++ )
	  p[i] = 0.0;
  oldMeasure.extractValuesAddToVector(p,0,true);
  for ( i=0 ; i<oldMeasure.getSize() ; i++ ) {
    _featureValues.push_back(oneVec);
    safeVec<int> pi(1,i);
    _paramIndices.push_back(pi);
    lbParameter par;
    par.setValue(p[i]);
    _params.push_back(par);
  }
  delete[] p;
}


lbFeatureTableMeasure::lbFeatureTableMeasure() :
  lbMultinomialMeasure() {
  
}

lbFeatureTableMeasure::lbFeatureTableMeasure(cardVec const& card) :
  lbMultinomialMeasure(card) {
  _featureValues.resize(getSize());
  _paramIndices.resize(getSize());
}

lbFeatureTableMeasure::lbFeatureTableMeasure(lbMeasure const& oldMeasure) :
  lbMultinomialMeasure(oldMeasure) {
  initialize((lbFeatureTableMeasure const &) oldMeasure);
}

lbFeatureTableMeasure::lbFeatureTableMeasure(lbMultinomialMeasure const& oldMeasure) :
  lbMultinomialMeasure(oldMeasure) {
  initialize((lbFeatureTableMeasure const &) oldMeasure);
}

lbFeatureTableMeasure::lbFeatureTableMeasure(lbFeatureTableMeasure const& oldMeasure) :
  lbMultinomialMeasure(oldMeasure) {
  initialize((lbFeatureTableMeasure const &) oldMeasure);
}

lbMeasure_Sptr lbFeatureTableMeasure::duplicate() const {  
  lbMeasure_Sptr res(new lbFeatureTableMeasure(*this));
  return res;
}

lbMeasure_Sptr lbFeatureTableMeasure::duplicateValues() const {
  assert(isValid());
  lbMeasure_Sptr res(new lbTableMeasure<lbLogValue>(_card));
  
  varsVec psuedoVars;
  for (uint i = 0; i < _card.size(); i++) {
    psuedoVars.push_back(i);
  }

  lbSmallAssignment assign = lbSmallAssignment(psuedoVars);

  do {
    lbValue val = computeValue(assign, psuedoVars);
    res->setLogValueOfFull(assign, psuedoVars, val.getLogValue());
  } while(assign.advanceOne(_card, psuedoVars));  

  return res;
}

lbFeatureTableMeasure::~lbFeatureTableMeasure() {
  // Nothing to do.
}


void lbFeatureTableMeasure::initialize(lbFeatureTableMeasure const& oldMeasure) {
  _paramIndices = oldMeasure._paramIndices;
  _featureValues = oldMeasure._featureValues;
  _params = oldMeasure._params;
}

void lbFeatureTableMeasure::printToFastInfFormat(ostream& out,bool normalizeValues, int prec,
						 bool printLogValues) const {
  lbMeasure_Sptr sptr = duplicateValues();
  sptr->printToFastInfFormat(out, normalizeValues, prec, printLogValues);
}

int lbFeatureTableMeasure::calcDeriv(probType* res,int index,
				     lbMeasure const& Estimated,
				     lbMeasure const& Empirical) const
{
  if ( _featureValues[0].size() > 1 ) {
    cerr << "Derivative computation for non-scalar parameters (" << _featureValues[0].size() << ") is not implemented yet!" << endl;
    exit(1);
  }
  
  lbTableMeasure<lbValue> temp((lbTableMeasure<lbValue> const &)Estimated);

  if ( isDirected() ) {
    temp.normalizeDirected();
    temp.multiplyByConditionalSumOfMeasure((lbTableMeasure<lbValue> const&)Empirical);
  }
  temp.subtractMeasure((lbTableMeasure<lbValue> const&)Empirical);
  
  //
  uint p;
  for (p = 0; p < _params.size() ; p++ )
    res[index+p] = 0.0;
  
  // collect from all cells in the table (this already takes care of sharing)
  int ind = 0;
  vector<int> paramCount(_params.size(),0);
  for ( uint i = 0; i<temp.getPrimarySize() ; i++ )
    for ( uint j = 0; j<temp.getSecondarySize() ; j++,ind++ ) {
      int pind = _paramIndices[ind][0];
      if ( (*temp.getValues())[i][j].getValue() != 0.0 ) {
	res[pind+index] +=  (*temp.getValues())[i][j].getValue();
        paramCount[pind]++;
      }
    }
  
  return _params.size();
}

int lbFeatureTableMeasure::extractValuesAddToVector(probType* vec,int index,bool logValues) const  {
  lbMeasure_Sptr sptr = duplicateValues();
  int n = sptr->extractValuesAddToVector(vec,index,logValues);
  return n;
}

int lbFeatureTableMeasure::extractLogParamsAddToVector(probType* vec,int index) const {
  if ( _featureValues[0].size() > 1 ) {
    cerr << "Param extraction for non-scalar parameters is not implemented yet!" << endl;
    exit(1);
  }

  for ( uint p=0 ; p<_params.size() ; p++ ) {
    vec[index+p] += _params[p].getValue();
  }
  
  return _params.size();
}

probVector lbFeatureTableMeasure::measure2Vec() const  {
  if ( _featureValues[0].size() > 1 ) {
    cerr << "Param extraction for non-scalar parameters is not implemented yet!" << endl;
    exit(1);
  }

  probVector res (_params.size()) ;
  for ( uint p=0 ; p<_params.size() ; p++ ) {
    res[p] = _params[p].getValue();
  }
  
  return res ;
}

void lbFeatureTableMeasure::setLogParams(safeVec<lbParameter> const & params) {
  _params.resize(params.size());
  for ( uint p=0 ; p<params.size() ; p++ ) {
    probType val = params[p].getValue();
    _params[p].setValue(val);
  }
}

int lbFeatureTableMeasure::setLogParams(probType const* vec, int index, bool& changed) {
  changed = false;
  for (uint i = 0; i < _params.size(); i++) {
    probType newVal = (vec[index+i]);
    probType diff = ABS(_params[i].getValue() - newVal);
    
    if (diff > (lbDefinitions::PARAM_EPSILON)) {
      _params[i].setValue(newVal);	  
      changed = true;
    }
  }
  
  return _params.size();
}
  
 
