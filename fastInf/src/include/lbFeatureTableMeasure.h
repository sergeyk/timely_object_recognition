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

#ifndef _Loopy__lbFeatureTableMeasure_
#define _Loopy__lbFeatureTableMeasure_

#include <lbMultinomialMeasure.h>
#include <lbTableMeasure.h>
#include <iomanip>
#include <map>

namespace lbLib {

  /**
     This Class holds the table measure for the graph
   
     This class genaralizes the lbTableMeasure to be a set of
     parameters as combined with a set of features.  This allows us to
     adjust features and parameters independently to define the
     specific values stored in the lbTableMeasure.
     
     Part of the loopy belief library @version November 2002  @author
     Ariel Jaimovich
  */


  class lbFeatureTableMeasure : public lbMultinomialMeasure {
  public:

    // CONSTRUCTION / DESTRUCTION
    
    lbFeatureTableMeasure();
    
    lbFeatureTableMeasure(cardVec const& card);
    
    explicit lbFeatureTableMeasure(lbMeasure const& oldMeasure);

    explicit lbFeatureTableMeasure(lbMultinomialMeasure const& oldMeasure);

    explicit lbFeatureTableMeasure(lbFeatureTableMeasure const& oldMeasure);

    // create a feature table measure that is based on a table measure
    // all features are assigned a value of 1 and there are as many
    // parameters as there are cells
    explicit lbFeatureTableMeasure(lbTableMeasure<lbLogValue> const& oldMeasure);
    
    virtual lbMeasure_Sptr duplicate() const;
  
    virtual lbMeasure_Sptr duplicateValues() const;

    virtual void initialize(lbFeatureTableMeasure const& oldMeasure);

    virtual ~lbFeatureTableMeasure();


    // COMPARISON

    inline virtual bool operator==(lbMeasure const& otherMeasure);

    inline virtual bool isValid() const;

    // TABLEWIDE
    inline virtual lbFeatureTableMeasure * multiply(lbFeatureTableMeasure& newMeasure) const;

    inline virtual void makeUniform();


    // FEATURES AND PARAMETER ACCESSORS / MUTATORS

    inline virtual void setFeatures(lbTableMeasure<lbValue> const& featureValues);

    virtual void setLogParams(safeVec<lbParameter> const & params);

    virtual int setLogParams(probType const* vec, int index, bool& changed);

    virtual void setFeatureValuesOfFull(lbBaseAssignment const& assign,
					       varsVec const& vars,
					       safeVec<lbFeatureValue> const & featureValues);

    inline virtual void setParameterIndicesOfFull(lbBaseAssignment const& assign,
						  varsVec const& vars,
						  safeVec<int> const & parameterIndices);


    inline lbParameter const& getParameter(int index) const;

    inline safeVec<lbFeatureValue> const& getFeature(int index) const { return _featureValues[index]; };

    // COMPUTE DERIVATIVE

    inline virtual int computeParamGradientAddToVector(probType* vec, int index, lbTableMeasure<lbValue> const& suffstats) const;


    // INPUT / OUTPUT
    inline virtual void print(ostream& out) const;



    // NOT REACHED
    inline virtual bool isDifferentMAX(lbMeasure const& otherMeasure, probType epsilon) const;  
    inline virtual bool isDifferentLogMAX(lbMeasure const& otherMeasure, probType epsilon) const;
    inline virtual bool isDifferentAVG(lbMeasure const& otherMeasure, probType epsilon) const;
    inline virtual bool isDifferentLogAVG(lbMeasure const& otherMeasure, probType epsilon) const;
    inline virtual bool isDifferentKL(lbMeasure const& otherMeasure, probType epsilon) const;
    inline virtual bool isEmpty() const;
    inline virtual bool marginalize(lbMeasure & newMeasure,
				    varsVec const& newVec,
				    varsVec const& oldVec,
				    bool multiply) const;
    inline virtual bool marginalizeWithAssign(lbMeasure& newMeasure,
					      varsVec const& newVec,
					      varsVec const& oldVec,
					      lbBaseAssignment const& assign,
					      bool multiply) const;
    inline virtual bool combineMeasures(lbMeasure& newtable,
					const lbMeasure& table,
					const varsVec& leftVec,
					const varsVec& rightVec,
					CombineType ct) const;
    inline virtual valueVector const& getValueVec(lbBaseAssignment const& assign,
						  varsVec const& vars) const;
    inline virtual valueVector & getValueVec(lbBaseAssignment const& assign,
					     varsVec const& vars);
    inline virtual void setParamVec(lbBaseAssignment const& assign,
				    varsVec const& vars,
				    valueVector const& params);
    inline virtual varValue randomValFromTable();
    inline virtual lbAssignment_ptr getMAPassign (const varsVec & vars) const ;
    inline virtual bool addMeasure(lbMeasure const& other);
    inline virtual void subtractMeasure(lbMeasure const& other);
    inline virtual void normalizeDirected();
    inline virtual void makeZeroes();
    inline virtual void makeRandom();
    inline virtual void makeSparse(double sparsity);
    inline virtual probType valueOfFull(lbBaseAssignment const& assign,
					varsVec const& vars) const;
    inline virtual probType logValueOfFull(lbBaseAssignment const& assign,
					   varsVec const& vars) const;
    inline virtual void setValueOfFull(lbBaseAssignment const& assign,
				       varsVec const& vars,
				       probType value);
    inline virtual void setLogValueOfFull(lbBaseAssignment const& assign,
					  varsVec const& vars,
					  probType value);
    inline virtual void subtractSumOfConditional(lbMeasure const & meas);
    inline virtual void divideMeasureByAnother(lbMeasure const& otherM);
    inline virtual void divideFromReplace(lbMeasure const& otherM);
    inline virtual void multiplyMeasureByAnother(lbMeasure const& otherM);
    inline virtual void multiplyMeasureByNumber(probType num);
    inline virtual void multiplyByConditionalSumOfMeasure(lbMeasure const& measure);
    inline virtual void replaceValues(probType from, probType to);
    inline virtual void normalize();

    inline virtual int getParamNum() const { return _params.size(); }
    // NOTE THAT FOR FEATURE TABLES THE ESTIMATED AND EMPIRICAL
    // MATRIX HAVE TO INCLUDE THE FEATURE VALUES
    virtual int calcDeriv(probType* res,int index,
			  lbMeasure const& Estimated,
			  lbMeasure const& Empirical) const;
    virtual int extractValuesAddToVector(probType* vec,int index,bool logValues) const;
    virtual int extractLogParamsAddToVector(probType* vec,int index) const;
    virtual probVector measure2Vec() const;

    inline virtual probType totalWeight();
    inline virtual void addSharedParams(assignmentPtrVec const & vec,varsVec const& vars);
    inline virtual void addIdleParams(assignmentPtrVec const & vec, varsVec const& vars);
    inline virtual bool updateMeasureValues(lbMeasure const& other, probType smooth,
					    probType epsilon = -1);
    
    inline virtual void addLogOfQuotient(lbMeasure const& numerator,lbMeasure const& denominator);
    inline virtual void setSharedParams(map<string , intVec> const& sharedIndices);
    inline virtual void enableSharedParams(lbSharedParamMode mode);
    inline virtual void enableIdleParams();
    inline virtual bool raiseToThePower(probType power);
    inline virtual void setSparsityThresh(double sparseLevel) const;
    inline virtual void readOneMeasure(ifstream_ptr in,
				       paramPtrMap & sharedMap,
				       paramPtrMap & idleMap,
				       bool readLogValues = false);
    virtual void printToFastInfFormat(ostream& out,bool normalizeValue = false,int prec = 5,
				      bool printLogValues = false) const;

  protected:
    inline virtual pair<cardVec,varsVec> buildSubSet(internalIndicesVec const& indices,
						     varsVec const& oldVec) const;
    inline virtual void buildProbVec(bool random);
    inline virtual void buildProbVec(int len, bool random);

    inline virtual lbParameter computeValue(lbBaseAssignment const& assign, varsVec const & vars) const;

    inline virtual lbParameter computeValue(uint index) const;

    inline virtual int computeIndex(lbBaseAssignment const& assign, varsVec const & vars) const;

    // The implementation makes use of the *this lbTable measure to
    // store the actual values represented by combining the parameters
    // and features.

    
    // Each assignment has a vector of parameter indices
    safeVec<safeVec<int> > _paramIndices;

    // Each assignment has a vector of features
    safeVec<safeVec<lbFeatureValue> > _featureValues;

    // The parameters themselves
    safeVec<lbParameter> _params;

    // The value of an entry Xij is exp(paramsXij dot  featuresXij)

    // These are to getr
    valueVector _unused;
  };

  ////////////////////////
  // COMPARISON METHODS //
  ////////////////////////
  inline bool lbFeatureTableMeasure::operator==(lbMeasure const& otherMeasure) {
    lbFeatureTableMeasure const& otherFeatureTable = (lbFeatureTableMeasure const &) otherMeasure;
    return (_paramIndices == otherFeatureTable._paramIndices &&
	    _featureValues == otherFeatureTable._featureValues &&
	    _params == otherFeatureTable._params);
  }

  inline bool lbFeatureTableMeasure::isValid() const {
    bool valid = true;

    if (_featureValues.size() != _paramIndices.size()) {
      valid = false;
      cerr << "Entry count differs according to param indices (";
      cerr << _paramIndices.size() << ") and feature value vectors (";
      cerr << _featureValues.size() << ").\n";
    }
    else {
      for (uint i = 0; i < _featureValues.size(); i++) {
	if (_featureValues[i].size() != _paramIndices[i].size()) {
	  valid = false;
	  cerr << "Feature value vector " << i << " differs in size of corresponding param indices vector.\n";
	}
      }
    }

    for (uint i = 0; i < _paramIndices.size(); i++) {
      for (uint j = 0; j < _paramIndices[i].size(); j++) {
	if (_paramIndices[i][j] < 0 || _paramIndices[i][j] >= (int) _params.size()) {
	  valid = false;
	  cerr << "Param index " << j << " of entry number " << i << "is out of range: " << _paramIndices[i][j] << "\n";
	}
      }
    }

    return valid;
  }

  inline void lbFeatureTableMeasure::setFeatures(lbTableMeasure<lbValue> const& featureValues) {
    varsVec psuedoVars;
    for (uint i = 0; i < _card.size(); i++) {
      psuedoVars.push_back(i);
    }

    lbSmallAssignment assign(psuedoVars);
    _featureValues.clear();

    do {
      lbFeatureValue fv;
      fv.setLogValue(featureValues.logValueOfFull(assign,psuedoVars));
      safeVec<lbFeatureValue> fvvec(1);
      fvvec[0] = fv;
      _featureValues.push_back(fvvec);
    } while(assign.advanceOne(_card, psuedoVars));  
  }

  inline lbParameter lbFeatureTableMeasure::computeValue(lbBaseAssignment const& assign, varsVec const & vars) const {
    return computeValue(computeIndex(assign, vars));
  }

  inline lbParameter lbFeatureTableMeasure::computeValue(uint index) const {
    safeVec<lbFeatureValue> const& fvs = _featureValues[index];
    safeVec<int> const& pinds = _paramIndices[index];
    
    assert(pinds.size() == fvs.size());
    lbParameter exponent;
    exponent.setValue(0);
    
    for (uint k = 0; k < pinds.size(); k++) {
      int pindex = pinds[k];
      assert(pindex >= 0 && pindex < (int) _params.size());
      exponent += _params[pindex]*fvs[k];
    }

    lbValue val;
    if ( isinf(exponent.getValue())) { //exponent.getValue() = +-inf
      if (exponent.getValue() < 0) //exponent.getValue() = -inf
	val.setValue(0.0);
      else //exponent.getValue() = +inf
	val.setValue(HUGE_VAL);
    }
    else
      val.setValue(exp(exponent.getValue())); // Numerical issues?
    return val;
  }

  inline int lbFeatureTableMeasure::computeIndex(lbBaseAssignment const& assign, varsVec const & vars) const {
    //    lbIndexConverter convi(_card,vars,_cardSize);
    //probIndex partindex = convi.assignToIndPartial(assign);
    probIndex partindex = assignToIndPartial(assign,vars);
    rVarIndex v = vars[_cardSize-1];
    int tmp = assign.getValueForVar(v);
    uint index = partindex*(_card.back()) + tmp;
    assert(index >= 0 && index < _featureValues.size());
    assert(index >= 0 && index < _paramIndices.size());
    return (int) index;
  }

  inline lbParameter const& lbFeatureTableMeasure::getParameter(int index) const
    {
      return _params[index];
    }


  inline void lbFeatureTableMeasure::setParameterIndicesOfFull(lbBaseAssignment const& assign,
							       varsVec const& vars,							       
							       safeVec<int> const & parameterIndices) {
    int index = computeIndex(assign, vars);
    _paramIndices[index] = parameterIndices;
  }
  
  inline lbFeatureTableMeasure * lbFeatureTableMeasure::multiply(lbFeatureTableMeasure& otherTable) const {
    lbFeatureTableMeasure * resultTable = new lbFeatureTableMeasure(*this);

    assert(resultTable->_featureValues.size() == otherTable._featureValues.size());
    assert(resultTable->_paramIndices.size() == otherTable._paramIndices.size());
    assert(resultTable->_featureValues.size() == resultTable->_paramIndices.size());

    uint size = resultTable->_featureValues.size();
    uint i;
    for (i = 0; i < size; i++) {
      safeVec<lbFeatureValue> & fvs = resultTable->_featureValues[i];
      safeVec<int> & pinds = resultTable->_paramIndices[i];
      safeVec<lbFeatureValue> otherfvs = otherTable._featureValues[i];
      safeVec<int> otherpinds = otherTable._paramIndices[i];
	uint j; 
      for (j= 0; j < otherfvs.size(); j++) {
	fvs.push_back(otherfvs[j]);
      }

      for (j = 0; j < otherpinds.size(); j++) {
	pinds.push_back(otherpinds[j] + resultTable->_params.size());
      }
    }

    safeVec<lbParameter> params = otherTable._params;
    for (i = 0; i < params.size(); i++) {
      resultTable->_params.push_back(params[i]);
    }

    return resultTable;
  }

  inline int lbFeatureTableMeasure::computeParamGradientAddToVector(probType* vec, int index, lbTableMeasure<lbValue> const& suffstats) const {
    NOT_IMPLEMENTED_YET;
    return 0;
  }

  inline void lbFeatureTableMeasure::print(ostream &out) const {
    lbMeasure_Sptr mes = duplicateValues();
    mes->print(out);

    out << endl << "Features:" << endl;
uint i;
    for (i = 0; i < _featureValues.size(); i++) {
      for (uint j = 0; j < _featureValues[i].size(); j++) {
	cerr << _featureValues[i][j] << " ";
      }
    }
    out << endl << endl << "Parameter Indices: " << endl; 

    for (i = 0; i < _paramIndices.size(); i++) {
      for (uint j = 0; j < _paramIndices[i].size(); j++) {
	cerr << _paramIndices[i][j] << " ";
      }
    }

    out << endl << endl << "Parameters: " << endl;
    printVector(_params, out);
  }


  inline bool lbFeatureTableMeasure::isDifferentMAX(lbMeasure const& otherMeasure, probType epsilon) const {
    NOT_REACHED;
    return false;
  }

  inline bool lbFeatureTableMeasure::isDifferentLogMAX(lbMeasure const& otherMeasure, probType epsilon) const {
    NOT_REACHED;
    return false;
  }
  
  inline bool lbFeatureTableMeasure::isDifferentAVG(lbMeasure const& otherMeasure, probType epsilon) const {
    NOT_REACHED;
    return false;
  }

  inline bool lbFeatureTableMeasure::isDifferentLogAVG(lbMeasure const& otherMeasure, probType epsilon) const {
    NOT_REACHED;
    return false;
  }

  inline bool lbFeatureTableMeasure::isDifferentKL(lbMeasure const& otherMeasure, probType epsilon) const {
    NOT_REACHED;
    return false;
  }
  

  inline bool lbFeatureTableMeasure::isEmpty() const {
    NOT_REACHED;
    return false;
  }

  

  ///////////////////////
  // TABLEWIDE METHODS //
  ///////////////////////

  inline bool lbFeatureTableMeasure::marginalize(lbMeasure & newMeasure,
					  varsVec const& newVec,
					  varsVec const& oldVec,
					  bool multiply) const {
    NOT_REACHED;
    return false;
  }
  
  inline bool lbFeatureTableMeasure::marginalizeWithAssign(lbMeasure& newMeasure,
						    varsVec const& newVec,
						    varsVec const& oldVec,
						    lbBaseAssignment const& assign,
						    bool multiply) const {
    NOT_REACHED;
    return false;
  }


  inline bool lbFeatureTableMeasure::combineMeasures(lbMeasure& newtable,
					      const lbMeasure& table,
					      const varsVec& leftVec,
					      const varsVec& rightVec,
					      CombineType ct) const {
    NOT_REACHED;
    return false;
  }
  
  inline valueVector const& lbFeatureTableMeasure::getValueVec(lbBaseAssignment const& assign,
							varsVec const& vars) const {
    NOT_REACHED;
    return _unused;
  }
 
  inline valueVector & lbFeatureTableMeasure::getValueVec(lbBaseAssignment const& assign,
						   varsVec const& vars) {
    NOT_REACHED;
    return _unused;
  }

 
  inline void lbFeatureTableMeasure::setParamVec(lbBaseAssignment const& assign,
					  varsVec const& vars,
					  valueVector const& params) {
    NOT_REACHED;
  }

  inline varValue lbFeatureTableMeasure::randomValFromTable() {
    NOT_REACHED;
    return 0;
  }

  inline lbAssignment_ptr lbFeatureTableMeasure::getMAPassign (const varsVec & vars) const {
    NOT_REACHED;
    return 0;
  }

  inline bool lbFeatureTableMeasure::addMeasure(lbMeasure const& other)  {
    NOT_REACHED;
    return true;
  }

  inline void lbFeatureTableMeasure::subtractMeasure(lbMeasure const& other)  {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::normalizeDirected()  {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::makeZeroes()  {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::makeRandom()  {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::makeUniform()  {
	uint i;
	  for (i = 0; i < _params.size(); i++) {
      _params[i].setValue(1);
    }

    for (i = 0; i < _featureValues.size(); i++) {
      for (uint j = 0; j < _featureValues[i].size(); j++) {
	_featureValues[i][j].setValue(1);
      }
    }
  }

  inline void lbFeatureTableMeasure::makeSparse(double sparsity) {
    NOT_REACHED;
  }

  // ENTRYWIDE METHODS

  inline probType lbFeatureTableMeasure::valueOfFull(lbBaseAssignment const& assign,
						     varsVec const& vars) const {
    return computeValue(assign,vars).getValue();
  }

  inline probType lbFeatureTableMeasure::logValueOfFull(lbBaseAssignment const& assign,
						 varsVec const& vars) const {
    return computeValue(assign,vars).getLogValue();
  }

  inline void lbFeatureTableMeasure::setValueOfFull(lbBaseAssignment const& assign,
					     varsVec const& vars,
					     probType value)  {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::setLogValueOfFull(lbBaseAssignment const& assign,
						varsVec const& vars,
						probType value)  {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::subtractSumOfConditional(lbMeasure const & meas) {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::divideMeasureByAnother(lbMeasure const& otherM) {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::divideFromReplace(lbMeasure const& otherM)  {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::multiplyMeasureByAnother(lbMeasure const& otherM) {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::multiplyMeasureByNumber(probType num) {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::multiplyByConditionalSumOfMeasure(lbMeasure const& measure) {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::replaceValues(probType from, probType to) {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::normalize() {
    NOT_REACHED;
  }

  inline probType lbFeatureTableMeasure::totalWeight() {
    double lweight = -HUGE_VAL;
    varsVec psuedoVars;
    for (uint i = 0; i < _card.size(); i++) psuedoVars.push_back(i);
    lbSmallAssignment assign(psuedoVars);
    do {
      lbValue val = computeValue(assign, psuedoVars);
      lweight = lbAddLog(lweight,val.getLogValue());
    } while(assign.advanceOne(_card, psuedoVars));  
    return exp(lweight);
  }

  inline void lbFeatureTableMeasure::addSharedParams(assignmentPtrVec const & vec,varsVec const& vars)  {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::addIdleParams(assignmentPtrVec const & vec, varsVec const& vars) {
    NOT_REACHED;
  }

  inline bool lbFeatureTableMeasure::updateMeasureValues(lbMeasure const& other, probType smooth,
							 probType epsilon)  {
    NOT_REACHED;
    return false;
  }

  inline void lbFeatureTableMeasure::addLogOfQuotient(lbMeasure const& numerator,lbMeasure const& denominator) {
    NOT_REACHED;
  }


  // SHARED / IDLE PARAMETERS

  inline void lbFeatureTableMeasure::setSharedParams(map<string , intVec> const& sharedIndices) {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::enableSharedParams(lbSharedParamMode mode) {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::enableIdleParams()  {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::buildProbVec(bool random) {
    NOT_REACHED;
  }

  inline void lbFeatureTableMeasure::buildProbVec(int len, bool random)  {
    NEEDS_CLEANUP; // is it?
  }

  inline bool lbFeatureTableMeasure::raiseToThePower(probType power) {
    NOT_REACHED;
	return false;
  }

  //////////////////////
  // SPARSITY METHODS //
  //////////////////////

  inline void lbFeatureTableMeasure::setSparsityThresh(double sparseLevel) const {
    NOT_REACHED;
  }

  
  //////////////////////////
  // INPUT/OUTPUT METHODS //
  //////////////////////////
  inline void lbFeatureTableMeasure::readOneMeasure(ifstream_ptr in,
						    paramPtrMap & sharedMap,
						    paramPtrMap & idleMap,
						    bool readLogValues) {
    NOT_REACHED;
  }
 
  ////////////////////
  // HELPER METHODS //
  ////////////////////
 
  inline pair<cardVec,varsVec> lbFeatureTableMeasure::buildSubSet(internalIndicesVec const& indices,
							   varsVec const& oldVec) const {
    NOT_REACHED;
		pair<cardVec,varsVec> tmp;
		return tmp;
  }

}; 
#endif

      


