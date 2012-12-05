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

#include <lbWeightedTableMeasure.h>
#include <lbRandomProb.h>
#include <math.h>

using namespace lbLib;

/**
   This Class holds the measure for the graph

   This Object holds the probabilistic characters of a clique.
   and performs simple actions on it
   
   Part of the loopy belief library
   @version November 2002
   @author Ariel Jaimovich
*/

lbWeightedTableMeasure::lbWeightedTableMeasure(lbTableMeasure<lbLogValue> const& oldMeasure) :
 lbFeatureTableMeasure(oldMeasure.getCards())
  {
  _featureValues.clear();
  _paramIndices.clear();
  safeVec<int> pi;
  pi.push_back(0); // point to single parameter
  setWeight(1.0);

  // go over cells
  probType* p = new probType[oldMeasure.getSize()];
  int i;
  for ( i=0 ; i<oldMeasure.getSize() ; i++ )
    p[i] = 0.0;
  oldMeasure.extractValuesAddToVector(p,0,true);
  for ( i=0 ; i<oldMeasure.getSize() ; i++ ) {
    lbFeatureValue feat;
    feat.setValue(p[i]);
    safeVec<lbFeatureValue> fvec;
    fvec.push_back(feat);
    _featureValues.push_back(fvec);
    _paramIndices.push_back(pi);
  }
  delete[] p;
}

lbWeightedTableMeasure::lbWeightedTableMeasure() :
  lbFeatureTableMeasure() {
  
  makeUniform();
  setWeight(1.0);
}

lbWeightedTableMeasure::lbWeightedTableMeasure(cardVec const& card) :
  lbFeatureTableMeasure(card) {
  makeUniform();
  setWeight(1.0);
}

lbWeightedTableMeasure::lbWeightedTableMeasure(lbMeasure const& oldMeasure) :
  lbFeatureTableMeasure(oldMeasure) {
  
}

lbWeightedTableMeasure::lbWeightedTableMeasure(lbMultinomialMeasure const& oldMeasure) :
  lbFeatureTableMeasure(oldMeasure) {
  
}
lbWeightedTableMeasure::lbWeightedTableMeasure(lbFeatureTableMeasure const& oldMeasure) :
  lbFeatureTableMeasure(oldMeasure) {
  
}

lbWeightedTableMeasure::lbWeightedTableMeasure(lbWeightedTableMeasure const& oldMeasure) :
  lbFeatureTableMeasure(oldMeasure) {
  
}

lbWeightedTableMeasure::~lbWeightedTableMeasure() {
}

void lbWeightedTableMeasure::setFeatureValueOfFull(lbBaseAssignment const& assign,
							  varsVec const& vars,
							  probType value) {
  int index = computeIndex(assign, vars);
  _featureValues[index].clear();
  lbParameter param;
    param.setValue(value);
    _featureValues[index].push_back(param);
}
  
void lbWeightedTableMeasure::setLogFeatureValueOfFull(lbBaseAssignment const& assign,
						      varsVec const& vars,
						      probType logvalue) {
  int index = computeIndex(assign, vars);
  _featureValues[index].clear();
  lbParameter param;
  param.setLogValue(logvalue);
  _featureValues[index].push_back(param);
}
  
