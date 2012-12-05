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


#include <lbInferenceObject.h>

using namespace lbLib ;
  

/**
   This Class holds the Global Inference Object

   General Idea:
   this class is the "mother" type of all inference objects
   (i.e loopy blief).
   
   Part of the loopy belief library
   @version November 2002
   @author Ariel Jaimovich
*/

lbAssignment_ptr lbInferenceObject::getMAPassign() {
  uint numOfVars = getModel().getGraph().getNumOfVars() ;
  lbAssignment_ptr evidence = new lbAssignment (numOfVars) ; // init with all ?'s
  for (rVarIndex var=0 ; (uint)var<numOfVars ; ++var) {
    varsVec singleVarVec ;
    singleVarVec.push_back (var) ;
    lbAssignedMeasure_ptr varProb = prob (singleVarVec) ;
    lbAssignment_ptr varMAPassign = varProb->getMAPassign() ;
    evidence->setValueForVar (var , varMAPassign->getValueForVar(var)) ; // set MAP value
    delete varMAPassign ;
    delete varProb ;
    changeEvidence (*(evidence)) ; // update inference object
  }
  return evidence ;
}

probType lbInferenceObject::partitionFunction() {
  if (!_built) {
    reset();
  }

  calculatePartition();
  return _currentPartition;
}

probType lbInferenceObject::initialPartitionFunction() {
  //cerr<<"Calculating partition for model :"<<endl;
  //getModel().printModel(cerr);
  if (!_built) {
    reset();
  }

  //cerr<<"Calculating partition: "<<_calculatedInitialPartition<<endl;

  if (!_calculatedInitialPartition) {
    lbAssignment oldEvid = _evidence;
    changeEvidence(lbAssignment());
    calculatePartition();
    _initialPartition = _currentPartition;
    setCalculatedInitPartition(true);
    changeEvidence(oldEvid);
    //    cerr<<"Calculated new part. function : "<<_initialPartition<<endl;
  }
  //  cerr<<"returning part. function : "<<_initialPartition<<endl;
  return _initialPartition;
}

probType lbInferenceObject::entropyFunction() {
  if (!_built) {
    reset();
  }

  calculatePartition();
  return _currentEntropy;
}
