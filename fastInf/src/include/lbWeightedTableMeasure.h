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

#ifndef _Loopy__lbWeightedTableMeasure_
#define _Loopy__lbWeightedTableMeasure_

#include <lbFeatureTableMeasure.h>

namespace lbLib {

  /**
     A table where all entries are multiplied by a weighted,
     that is also the only free parameter for learning purposes
  **/

  class lbWeightedTableMeasure : public lbFeatureTableMeasure {
  public:

    // CONSTRUCTION / DESTRUCTION
    lbWeightedTableMeasure();
    
    lbWeightedTableMeasure(cardVec const& card);
    
    explicit lbWeightedTableMeasure(lbMeasure const& oldMeasure);
    
    explicit lbWeightedTableMeasure(lbMultinomialMeasure const& oldMeasure);
        
    explicit lbWeightedTableMeasure(lbFeatureTableMeasure const& oldMeasure);

    explicit lbWeightedTableMeasure(lbWeightedTableMeasure const& oldMeasure);
    
    explicit lbWeightedTableMeasure(lbTableMeasure<lbLogValue> const& oldMeasure);

    virtual void makeUniform();

    virtual ~lbWeightedTableMeasure();

    void setFeatureValueOfFull(lbBaseAssignment const& assign,
			       varsVec const& vars,
			       probType value);
    
    void setLogFeatureValueOfFull(lbBaseAssignment const& assign,
				  varsVec const& vars,
				  probType value);
    void setWeight(double w);

    double getWeight() const;

  protected:
  private:
  };

  inline void lbWeightedTableMeasure::makeUniform() {
    varsVec psuedoVars;
    for (uint i = 0; i < _card.size(); i++) {
      psuedoVars.push_back(i);
    }

    lbSmallAssignment assign = lbSmallAssignment(psuedoVars);
    
    do {
      setParameterIndicesOfFull(assign, psuedoVars, safeVec<int>(1,0));
    } while(assign.advanceOne(getCards(), psuedoVars));

    lbSmallAssignment assign2 = lbSmallAssignment(psuedoVars);

    do {
      setFeatureValueOfFull(assign2, psuedoVars, 1);
    } while(assign2.advanceOne(getCards(), psuedoVars));


    setWeight(0);
  }

  inline void lbWeightedTableMeasure::setWeight(double w) {
    _params.clear();
    lbParameter weight;
    weight.setValue(w);
    _params.push_back(weight);
uint i;
    for (i = 0; i < _paramIndices.size(); i++) {
      safeVec<int> vec(1,0);
      _paramIndices[i] = vec;
    }

    for (i = 0; i < _featureValues.size(); i++) {
      assert(_featureValues[i].size() == 1);
    }
  }
  
  inline double lbWeightedTableMeasure::getWeight() const {
    assert (_params.size() == 1);
    return _params[0].getValue();
  }
};
#endif

      


