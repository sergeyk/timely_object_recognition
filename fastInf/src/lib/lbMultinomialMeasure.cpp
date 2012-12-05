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

#include <lbMultinomialMeasure.h>

using namespace lbLib ;

/**
   This Class holds the measure for the graph

   This Object holds the probabilistic characters of a clique.
   and performs simple actions on it
   
   Part of the loopy belief library
   @version November 2002
   @author Ariel Jaimovich
*/

  //this method returns true if something has changed
  inline bool lbMultinomialMeasure::updateAssign(lbAssignment const& givenAssign,
                                                        varsVec const& vars,
                                                        lbMeasure const& original) {
    //lbTableMeasure const& originalTable = static_cast<lbTableMeasure const&>(original);
    
      
    lbAssignment_ptr movingAssign(new lbAssignment());
    bool changed = false;
    movingAssign->zeroise(vars);
    do {    
      probType dref = (*this)(*movingAssign,vars);
      
      //zeroise all assignment that don't agree with the evidence
      if (!(givenAssign.matches(*movingAssign,vars))) {
	if (dref != 0.0) {
	  setValueOfFull(*movingAssign,vars,0.0);
	  changed = true;
	}
      }
      else {
	//make sure it returns to the same value as before
	probType origValue = original(*movingAssign,vars);
	if (origValue != dref) {
          setValueOfFull(*movingAssign,vars,origValue);
	  changed = true;
	}
      }
    
    } while (movingAssign->advanceOne(_card,vars));
    delete movingAssign;

    if (changed) {
      _totalWeightIsUpdated = false;
    }
    return changed;
  }

  inline bool lbMultinomialMeasure::updateAssign(lbAssignment const& givenAssign,
                                                 varsVec const& vars) {
    makeUniform();
    lbAssignment_ptr movingAssign(new lbAssignment());
    bool changed = false;
    movingAssign->zeroise(vars);
    do {
      probType dref = (*this)(*movingAssign,vars);
      //zeroise all assignment that don't agree with the evidence
      if (!(givenAssign.matches(*movingAssign,vars))) {
	if (dref != 0.0) {
	  setValueOfFull(*movingAssign,vars,0.0);;
	  changed = true;
	}
      }
    } while (movingAssign->advanceOne(_card,vars));
    delete movingAssign;

    if (changed) {
      _totalWeightIsUpdated = false;
    }
    normalize();
    //cerr<<"updated assign"<<endl;
    //print(cerr);
    return true;
  }
