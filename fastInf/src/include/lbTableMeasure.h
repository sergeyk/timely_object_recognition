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

#ifndef _Loopy__lbTableMeasure
#define _Loopy__lbTableMeasure

#include <lbMultinomialMeasure.h>
#include <iomanip>
#include <map>
//#include <typeinfo>

#include <lbOptions.h>

namespace lbLib {

  /**
     This Class holds the table measure for the graph
   
     At any given point in time between method calls an instance of
     this class is NOT associated with any particular set of variables
     (though it does have cardinality information).  When methods that
     perform operations are called we pass in the variables that the
     measure is defined over to complete the operation.

     Part of the loopy belief library @version November 2002 @author
     Ariel Jaimovich
  */

  template<class VALUE>
  class lbTableMeasure : public lbMultinomialMeasure {
  public:

    typedef safeVec<VALUE> valueVec;
    typedef safeVec<valueVec> valueVecVec;

    // CONSTRUCTION / DESTRUCTION
    inline lbTableMeasure();

    inline lbTableMeasure(cardVec const& card,bool random = true);
    
    inline explicit lbTableMeasure(lbMeasure const& oldMeasure);

    inline explicit lbTableMeasure(lbMultinomialMeasure const& oldMeasure);

    template <class OTHER_VALUE>
    inline explicit lbTableMeasure(lbTableMeasure<OTHER_VALUE> const& oldMeasure)
	: lbMultinomialMeasure((lbMeasure &)oldMeasure) {
		initialize(oldMeasure);
	}


    inline explicit lbTableMeasure(lbTableMeasure<VALUE> const& oldMeasure);

    inline virtual lbMeasure& operator=(lbMeasure const& oldMeasure);

    inline virtual lbTableMeasure<VALUE>& operator=(lbTableMeasure const& oldMeasure);
   
    inline virtual lbMeasure_Sptr duplicate() const;

    // for table that have complex inner representation,
    // duplicate values is used for inference purposes
    virtual lbMeasure_Sptr duplicateValues() const { return duplicate(); };

    inline virtual ~lbTableMeasure();



    // COMPARISON

    inline bool operator==(lbMeasure const& otherMeasure);

    inline bool isDifferentMAX(lbMeasure const& otherMeasure, probType epsilon) const;

    inline bool isDifferentLogMAX(lbMeasure const& otherMeasure, probType epsilon) const;

    inline bool isDifferentAVG(lbMeasure const& otherMeasure, probType epsilon) const;

    inline bool isDifferentLogAVG(lbMeasure const& otherMeasure, probType epsilon) const;
    
    inline bool isDifferentKL(lbMeasure const& otherMeasure, probType epsilon) const;

    inline bool isEmpty() const;

    // TABLEWIDE METHODS

    inline virtual bool marginalize(lbMeasure& newMeasure,
				    varsVec const& newVec,
				    varsVec const& oldVec,
				    bool multiply = false) const;
    
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
 
    inline virtual probType totalWeight();

    inline virtual void normalize();

    inline virtual void makeZeroes();

    inline virtual void makeRandom();

    inline virtual void makeUniform();

    inline virtual void makeSparse(double sparsity);

    inline virtual void normalizeDirected();

    inline virtual bool raiseToThePower(probType power);

    inline virtual void takeLog() ;
    inline virtual void takeExp() ;

    inline virtual bool addMeasure(lbMeasure const& otherMeasure);      
    
    inline virtual void subtractMeasure(lbMeasure const& other);

    inline virtual bool updateMeasureValues(lbMeasure const& other,probType smoothParam,
					    probType epsilon = -1);



    inline virtual void addLogOfQuotient(lbMeasure const& nominator,lbMeasure const& denominator);

    inline virtual void subtractSumOfConditional(lbMeasure const & meas);
    
    inline virtual void divideMeasureByAnother(lbMeasure const& otherM);  

    inline virtual void divideFromReplace(lbMeasure const& otherM);
  
    inline virtual void multiplyMeasureByAnother(lbMeasure const& otherM);

    inline virtual void multiplyMeasureByNumber(probType num);

    inline virtual void multiplyByConditionalSumOfMeasure(lbMeasure const& measure);

    inline virtual void replaceValues(probType from, probType to);

    
    virtual int getParamNum() const;

    virtual int calcDeriv(probType* res,int index,
			  lbMeasure const& Estimated,
			  lbMeasure const& Empirical) const;

    inline int extractValuesAddToVector(probType* vec,int index,bool logValues) const;

    inline int extractLogParamsAddToVector(probType* vec,int index) const;

    inline probVector measure2Vec() const;

    inline int setLogParams(probType const* vec, int index, bool& changed);

    inline valueVec const& getValueVec(lbBaseAssignment const& assign,
					  varsVec const& vars) const;
    
    inline valueVec & getValueVec(lbBaseAssignment const& assign,
				     varsVec const& vars);

    inline void setParamVec(lbBaseAssignment const& assign,
			    varsVec const& vars,
			    valueVec const& prob);


    
    // ENTRYWIDE METHODS

    inline virtual probType valueOfFull(lbBaseAssignment const& assign,
					varsVec const& vars) const ;
   
    inline virtual probType logValueOfFull(lbBaseAssignment const& assign,
					   varsVec const& vars) const;
    
    inline virtual void setValueOfFull(lbBaseAssignment const& assign,
				       varsVec const& vars,
				       probType value);
    
    inline virtual void setLogValueOfFull(lbBaseAssignment const& assign,
					  varsVec const& vars,
					  probType value);
    


    // SHARED / IDLE PARAMETERS
    inline void setSharedParams(map<string , intVec> const& sharedIndices);

    inline virtual void enableSharedParams(lbSharedParamMode mode);

    inline virtual void enableIdleParams();

    inline void addSharedParams(assignmentPtrVec const & vec, varsVec const& vars);
    
    inline void addIdleParams(assignmentPtrVec const & vec, varsVec const& var);
    
    inline varValue randomValFromTable(); 

    inline lbAssignment_ptr getMAPassign (const varsVec & vars) const ;

    // SPARSITY

    virtual inline double getSparsityRatio() const;

    virtual inline void setSparsityThresh(double sparseLevel) const;

    // INPUT / OUTPUT

    inline void readOneMeasure(ifstream_ptr in,
			       paramPtrMap & sharedMap,
			       paramPtrMap & idleMap,
			       bool readLogValues = false);

    inline void print(ostream& out) const;
    
    inline void printToFastInfFormat(ostream& out,bool normalizeValues = false,int prec = 5,
				     bool printLogValues = false) const;
    
    static void printStats();

    virtual inline void printNZEs(int size) const;


    inline uint getPrimarySize() const { return _primarySize; }
    inline uint getSecondarySize() const { return _secondarySize; }; 
    virtual inline bool getSparse() const;

    VALUE getTotalWeight() const { return _totalWeight; };
    int getTotalSize() const { return _totalSize; };
    valueVecVec const* getValues() const { return _values; };
    int getNumOfSharedParams() const { return _numOfSharedParams; };
    intVecVec const& getSharedParams() const { return _sharedParams; };
    intVec const& getIdleParams() const { return _idleParams; };
    void setIdleParams(intVec const& vec);
    void setSharedParams(intVecVec const& cev);

  protected:

    template<class OTHER_VALUE>
    inline void initialize(lbTableMeasure<OTHER_VALUE> const& oldMeasure)
	{
  //   cerr << "Initializing table...\n";
  _totalWeight = oldMeasure.getTotalWeight();
  _totalSize = oldMeasure.getTotalSize();
  _primarySize = oldMeasure.getPrimarySize();
  _secondarySize = oldMeasure.getSecondarySize();
  _values = new valueVecVec(oldMeasure.getValues()->size());
  for ( uint i=0 ; i<oldMeasure.getValues()->size() ; i++ ) {
    (*_values)[i].resize((*oldMeasure.getValues())[i].size());
    for ( uint j=0 ; j<(*oldMeasure.getValues())[i].size() ; j++ )
      (*_values)[i][j] = (*oldMeasure.getValues())[i][j];
  }
  _numOfSharedParams = oldMeasure.getNumOfSharedParams();
  _sharedParams = oldMeasure.getSharedParams();
  _idleParams = oldMeasure.getIdleParams();
  setSparse(oldMeasure.getSparse());
}



    virtual inline void setSparse(bool sparse) const;

    virtual inline void dirtySparsity() const { _sparsityUpdated = false; }

    // HELPER METHODS

    inline pair <internalIndicesVec,internalIndicesVec> findMarginVars(varsVec const& newVec,
									varsVec const& oldVec) const;

    inline pair<cardVec, varsVec> buildCombineVecs(varsVec const& leftVars, 
						   varsVec const& rightVars,
						   cardVec const& leftCards,
						   cardVec const& rightCards) const;

    inline virtual void readCards(ifstream_ptr in);

    inline virtual void readParameters(ifstream_ptr in,
				       paramPtrMap & sharedMap,
				       paramPtrMap & idleMap,
				       bool readLogValues);

    inline virtual pair<cardVec,varsVec> buildSubSet(internalIndicesVec const& indices,
						     varsVec const& oldVec) const;

    inline intVec getTablePosByArrayIndex (int index ,
					   const intVec & cards) const ;

    inline virtual void buildProbVec(bool random = true);
    
    inline virtual void buildProbVec(int len,bool random = true) ;
        

    // These are protected helpers to get parameters without requiring
    // total weight or sparsity maintanance (it assumes the methods
    // calling them handles these sorts of things).
    inline virtual VALUE & getValue(int i,int j) const;

    inline VALUE & getValue(lbBaseAssignment const& assign,
			      varsVec const& vars);
    

    inline VALUE const& getValue(lbBaseAssignment const& assign,
				   varsVec const& vars) const;


    
    // Standard marginalizations
    inline virtual bool marginalizeWithoutAssign(lbTableMeasure& newTable,
						 cardVec const& margCard,
						 varsVec const& margVars,
						 varsVec const& newVec,
						 varsVec const& oldVec,
						 varsVec const& fullVec,
						 bool multiply) const;

    inline virtual bool marginalizeWithAssign(lbTableMeasure& newTable,
					      cardVec const& margCard,
					      varsVec const& margVars,
					      varsVec const& newVec,
					      varsVec const& oldVec,
					      varsVec const& fullVec,
					      lbBaseAssignment const& assign,
					      bool multiply) const ;

    // Sparse marginalizations (without assignment)
    inline virtual bool margiplyOntoSparse(lbTableMeasure& newTable,
					      cardVec const& margCard,
					      varsVec const& margVars,
					      varsVec const& newVec,
					      varsVec const& oldVec,
					      varsVec const& fullVec) const;

    inline virtual bool marginalizeFromSparse(lbTableMeasure& newTable,
					      cardVec const& extraCard,
					      varsVec const& extraVars,
					      varsVec const& newVec,
					      varsVec const& oldVec,
					      varsVec const& fullVec,
					      bool multiply) const;
    
    virtual inline void updateSparsity() const;

    virtual inline pair<varsVec, cardVec> varsCardsVecMinus(varsVec const& v1,
							    cardVec const& c1,
							    varsVec const& v2) const;
    virtual inline pair<varsVec, cardVec> varsCardsVecPlus(varsVec const& v1,
							   cardVec const& c1,
							   varsVec const& v2,
							   cardVec const& c2) const;

    virtual inline void fillFullAssignment(lbSmallAssignment & assign,
					   varsVec const& movingVars,
					   varsVec const& fixedVars,
					   varsVec const& psuedoVars,
					   lbSmallAssignment const& nonzeroassign) const {      
      assign.zeroise(movingVars);
      for (uint v = 0; v < fixedVars.size(); v++) {
	assign.setValueForVar(fixedVars[v], nonzeroassign.getValueForVar(psuedoVars[v]));
      }
    }
    
  protected:

  private:
    
    uint _primarySize;
    uint _secondarySize;
    int _totalSize;
    valueVecVec * _values;
    intVecVec _sharedParams;
    intVec _idleParams;
    int _numOfSharedParams;

    // For sparse represenation
    mutable bool _sparsityUpdated;
    mutable bool _sparse;
    mutable vector<lbSmallAssignment> _nonZeroEntries;
    mutable varsVec _psuedoVars;    // 0 ... n to assist maintaining default entries.


    static int _MARG_COUNT;
    static int _MULT_COUNT;
    static int _DIV_COUNT;
    static int _NORM_COUNT;
    static int _MARG_OP_COUNT;
    static int _MULT_OP_COUNT;
    static int _DIV_OP_COUNT;
    static int _NORM_OP_COUNT;

    VALUE _totalWeight;
  };

  typedef shared_ptr < lbTableMeasure<lbValue> > lbTableMeasure_Sptr;
  typedef shared_ptr < lbTableMeasure<lbLogValue> > lbLogTableMeasure_Sptr;
template <class VALUE>
int lbTableMeasure<VALUE>::_MARG_COUNT = 0;

template <class VALUE>
int lbTableMeasure<VALUE>::_MULT_COUNT = 0;

template <class VALUE>
int lbTableMeasure<VALUE>::_DIV_COUNT = 0;

template <class VALUE>
int lbTableMeasure<VALUE>::_NORM_COUNT = 0;

template <class VALUE>
int lbTableMeasure<VALUE>::_MARG_OP_COUNT = 0;

template <class VALUE>
int lbTableMeasure<VALUE>::_MULT_OP_COUNT = 0;

template <class VALUE>
int lbTableMeasure<VALUE>::_DIV_OP_COUNT = 0;

template <class VALUE>
int lbTableMeasure<VALUE>::_NORM_OP_COUNT = 0;

template <class VALUE>
inline lbTableMeasure<VALUE>::lbTableMeasure() :
  lbMultinomialMeasure() {
  _values = NULL;
  _totalSize = 0;
  _primarySize = 0;
  _secondarySize = 0;
  _values = new valueVecVec();
  _numOfSharedParams = 0;
  _sharedParams = intVecVec();
  _idleParams = intVec();
  _sparse = false;
  _sparsityUpdated = true;
  _totalWeight.setValue(0);
}

template <class VALUE>
inline lbTableMeasure<VALUE>::lbTableMeasure(cardVec const& card, bool random) :
  lbMultinomialMeasure(card) {

  _sparse = false;
  _sparsityUpdated = true;
  _values = NULL;
  _numOfSharedParams = 0;
  _sharedParams = intVecVec();
  _idleParams = intVec();
  _totalWeight.setValue(0);
  buildProbVec(random);
}

// If you want to convert between table measures with different VALUE
// types then you must explicitly cast it down to the correct type
// before implicitly casting to the other table type is even possible.
// If you don't the typeid asserts will fire.

template <class VALUE>
inline lbTableMeasure<VALUE>::lbTableMeasure(lbMeasure const& oldMeasure) :
  lbMultinomialMeasure(oldMeasure) {
  //  assert (typeid(oldMeasure) == typeid(*this));
  initialize((lbTableMeasure const&) oldMeasure);
}

template <class VALUE>
inline lbTableMeasure<VALUE>::lbTableMeasure(lbMultinomialMeasure const& oldMeasure) :
  lbMultinomialMeasure(oldMeasure){
  // assert (typeid(oldMeasure) == typeid(*this));
  initialize((lbTableMeasure const&) oldMeasure);
}

template <class VALUE>
inline lbTableMeasure<VALUE>::lbTableMeasure(lbTableMeasure<VALUE> const& oldMeasure) :
  lbMultinomialMeasure((lbMeasure &)oldMeasure) {
  initialize(oldMeasure);
}

template <class VALUE>
inline lbMeasure& lbTableMeasure<VALUE>::operator=(lbMeasure const& oldMeasure)
  {
    // cerr << "In M operator=\n";
    return operator=( (lbTableMeasure const &)oldMeasure );
  }

template <class VALUE>
inline lbTableMeasure<VALUE>& lbTableMeasure<VALUE>::operator=(lbTableMeasure const& oldMeasure)
{
  //  cerr << "In TM operator=\n";
  lbMultinomialMeasure::operator=(oldMeasure);
  initialize((lbTableMeasure const&) oldMeasure);
  return *this;
}

template <class VALUE>
inline lbMeasure_Sptr lbTableMeasure<VALUE>::duplicate() const {  
  lbMeasure_Sptr res(new lbTableMeasure(*this));
  return res;
}

template <class VALUE>
inline lbTableMeasure<VALUE>::~lbTableMeasure() {
  if (_values)
    delete _values;
}

  ////////////////////////
  // COMPARISON METHODS //
  ////////////////////////
  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::operator==(lbMeasure const& otherMeasure) {
    lbTableMeasure const& otherTable = (lbTableMeasure const&) otherMeasure;
    
    if (!(_type == otherTable._type)) {
      return false;
    }
    
    if (!(_card == otherTable._card)) {
      return false;
    }
    
    // Note we're not doing a parameter by parameter comparison.
    if(!(_values == otherTable._values)) {
      return false;
    }
    
    return true;
  }
  
  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::isDifferentMAX(lbMeasure const& otherMeasure, probType epsilon) const {
    lbTableMeasure & other = (lbTableMeasure&) otherMeasure;
    
    assert(_primarySize == other._primarySize);
    assert(_secondarySize == other._secondarySize);

    VALUE eps;
    eps.setValue(epsilon);
    
    for (uint i = 0; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++) {
	if (absDiff(getValue(i,j), other.getValue(i,j)) > eps) {
	  return true;
	}
      }
    }
    
    return false;
  }

  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::isDifferentLogMAX(lbMeasure const& otherMeasure, probType epsilon) const {
    lbTableMeasure & other = (lbTableMeasure&) otherMeasure;
    
    assert(_primarySize == other._primarySize);
    assert(_secondarySize == other._secondarySize);
    
    for (uint i = 0; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++) {
	probType thisLogVal = getValue(i,j).getLogValue();
	probType otherLogVal = other.getValue(i,j).getLogValue();

	probType diff = HUGE_VAL;
	if (thisLogVal == otherLogVal) //handles case of thisLogVal = otherLogVal = -HUGE_VAL
	  diff = 0;
	else
	  diff = ABS(thisLogVal - otherLogVal);
	
	if ( diff  > epsilon) {
	  return true;
	}
      }
    }
    
    return false;
  }
  
  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::isDifferentAVG(lbMeasure const& otherMeasure, probType epsilon) const {
    lbTableMeasure & other = (lbTableMeasure&) otherMeasure;
    
    assert(_primarySize == other._primarySize);
    assert(_secondarySize == other._secondarySize);
    
    int counter = 0;

    VALUE sum;
    sum.setValue(0.0);
    
    for (uint i = 0; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++) {
	VALUE diff = absDiff(getValue(i,j), other.getValue(i,j));
	sum += diff;
	
	counter++;
      }
    }

    VALUE counterVal;
    counterVal.setValue(counter);
    
    VALUE avg = sum / counterVal;

    VALUE eps;
    eps.setValue(epsilon);
    return (avg > eps);
  }

  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::isDifferentLogAVG(lbMeasure const& otherMeasure, probType epsilon) const {
    lbTableMeasure & other = (lbTableMeasure&) otherMeasure;
    
    assert(_primarySize == other._primarySize);
    assert(_secondarySize == other._secondarySize);
    
    int counter = 0;
    probType sum = 0.0;
    
    for (uint i = 0; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++) {
	probType d1 = getValue(i,j).getLogValue();
	probType d2 = other.getValue(i,j).getLogValue();
	
	if (d1 != d2) {
	  probType diff = ABS(d1 - d2);
	  sum += diff;
	}
	
	counter++;
      }
    }
    
    probType avg = sum/counter;
    return (avg > epsilon);
  }
  
  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::isDifferentKL(lbMeasure const& otherMeasure, probType epsilon) const {
    lbTableMeasure & other = (lbTableMeasure&) otherMeasure;
    
    assert(_primarySize == other._primarySize);
    assert(_secondarySize == other._secondarySize);
    
    probType kl = 0.0;
    for (uint i = 0; i < _primarySize; i++)
      for (uint j = 0; j < _secondarySize; j++) {
	probType logp = getValue(i,j).getLog2Value();
	probType logq = other.getValue(i,j).getLog2Value();
	probType p = getValue(i,j).getValue();
	
	if (p != 0.0) {
	  kl += p*(logp - logq);
	}
      }
    
    return (ABS(kl) > epsilon);
  }
  
  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::isEmpty() const {
    return (_card.size() == 0);
  }

  

  ///////////////////////
  // TABLEWIDE METHODS //
  ///////////////////////


  // This measure's variables are instantiated as oldVec for this
  // operation.  Fills in the newMeasure with this measure
  // marginalized according to newVec.  If newMeasure comes in with
  // anything its contents are overwritten.

  template <class VALUE>
    inline bool lbTableMeasure<VALUE>::marginalize(lbMeasure & newMeasure, // this is where the result is places
						   varsVec const& newVec,  // vars of target measure
						   varsVec const& oldVec,  // vars of current measure
						   bool multiply) const {
    
    pair<varsVec, cardVec> full, marginalized, remaining, extra;

    lbTableMeasure & newTable = (lbTableMeasure &) newMeasure;

    if (newTable.isEmpty()) {
      assert(vecSubset(newVec, oldVec));
      full.first = oldVec;
      full.second = _card;
    }
    else {
      full = varsCardsVecPlus(oldVec, _card, newVec, newTable._card);
    }
    
    marginalized = varsCardsVecMinus(oldVec, _card, newVec);
    remaining = varsCardsVecMinus(oldVec, _card, marginalized.first);
    extra = varsCardsVecMinus(full.first, full.second, oldVec);
   
    if (newTable.isEmpty()) {
      newTable.buildCardVec(remaining.second);
      newTable.buildProbVec(true /* random */);
    }

    if (multiply) {
      _MULT_COUNT++;
    }
    else {
      _MARG_COUNT++;
    }

    if (getSparse()) {
      return marginalizeFromSparse(newTable,
				   extra.second,
				   extra.first,
				   newVec,
				   oldVec,
				   full.first,
				   multiply);
    }
    else if (newTable.getSparse() && multiply) {
      return margiplyOntoSparse(newTable,
				marginalized.second,
				marginalized.first,
				newVec,
				oldVec,
				full.first);
    }
    else {
      return marginalizeWithoutAssign(newTable,
				      marginalized.second,
				      marginalized.first,
				      newVec, 
				      oldVec, 
				      full.first, 
				      multiply);
    }
  }

  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::marginalizeWithAssign(lbMeasure& newMeasure,
						    varsVec const& newVec,
						    varsVec const& oldVec,
						    lbBaseAssignment const& assign,
						    bool multiply) const {
    pair<internalIndicesVec, internalIndicesVec> indices = findMarginVars(newVec, oldVec);
    internalIndicesVec & marginalized = indices.first;
    internalIndicesVec & remaining = indices.second;
    
    lbTableMeasure & newTable = (lbTableMeasure &) newMeasure; 
    newTable.buildCardVec(_card, remaining);
    newTable.buildProbVec(false /* random */);

    pair<cardVec, varsVec> tmpPair = buildSubSet(marginalized, oldVec);
    cardVec& margCard = tmpPair.first;
    varsVec& margVars = tmpPair.second;

    varsVec fullVec(newVec);
    for (uint i = 0; i < margVars.size(); i++) {
      fullVec.push_back(margVars[i]);
    }

    if (multiply) {
      _MULT_COUNT++;
    }
    else {
      _MARG_COUNT++;
    }

    return marginalizeWithAssign(newTable,
				 margCard, margVars,
				 newVec, oldVec, fullVec, assign,
				 multiply);
  }

  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::combineMeasures(lbMeasure& newtable,
					      const lbMeasure& table,
					      const varsVec& leftVec,
					      const varsVec& rightVec,
					      CombineType ct) const {
    lbTableMeasure & result = (lbTableMeasure &) newtable;
    const lbTableMeasure & left = (lbTableMeasure &) *this;
    const lbTableMeasure & right = (lbTableMeasure &) table;
    

    pair<cardVec, varsVec> vecpair = buildCombineVecs(leftVec, 
						      rightVec, 
						      getCards(), 
						      table.getCards());
    cardVec const& fullCards = vecpair.first;
    varsVec const& fullVec = vecpair.second;

    result.buildCardVec(fullCards);
    result.buildProbVec(false /* random */);

    lbSmallAssignment assign = lbSmallAssignment(fullVec);

    if (ct == COMBINE_MULTIPLY) {
      _MULT_COUNT++;
    }
    else {
      _DIV_COUNT++;
    }

    do {      
      VALUE paramLeft, paramRight;
      paramLeft = left.getValue(assign, leftVec);
      paramRight = right.getValue(assign, rightVec);

      if (ct == COMBINE_MULTIPLY) {
	_MULT_OP_COUNT++;
	result.getValue(assign, fullVec) = paramLeft * paramRight;
      }
      else if (ct == COMBINE_DIVIDE) {
	_DIV_OP_COUNT++;
	VALUE zero;
	zero.setValue(0);

	if (paramRight == zero) {
	  result.getValue(assign, fullVec) = zero;
	}
	else {
	  result.getValue(assign, fullVec) = paramLeft / paramRight;
	}
      }
      else {
	return false;
	NOT_REACHED;
      }
    } while(assign.advanceOne(result._card, fullVec));
    
    result._totalWeightIsUpdated = false;
    result.dirtySparsity();

    return true;
  }
 


  // help for multiply

  template <class VALUE>
  inline pair<cardVec, varsVec> lbTableMeasure<VALUE>::buildCombineVecs(varsVec const& leftVars, 
								 varsVec const& rightVars,
								 cardVec const& leftCards,
								 cardVec const& rightCards) const {
    /* Start with all of left's vars */
    varsVec fullVarsVec = leftVars;
    cardVec fullCardVec = leftCards;
    
    for (uint i = 0; i < rightVars.size(); i++) {
      bool found = false;
      for (uint j = 0; j < leftVars.size(); j++) {
	if (leftVars[j] == rightVars[i]) {
	  found = true;
	  assert(leftCards[j] == rightCards[i]);
	  break;
	}
      }
      
      /* Add the vars that are not duplicates */
      if (!found) {
	fullVarsVec.push_back(rightVars[i]);
	fullCardVec.push_back(rightCards[i]);
      }
    }
    
    return pair<cardVec, varsVec>(fullCardVec, fullVarsVec);
  }
 

  template <class VALUE>
  inline int lbTableMeasure<VALUE>::getParamNum() const {
    return _totalSize;
  }

  template <class VALUE>
  inline int lbTableMeasure<VALUE>::calcDeriv(probType* res,int index,
					      lbMeasure const& Estimated,
					      lbMeasure const& Empirical) const {

    lbTableMeasure<lbValue> temp((lbTableMeasure<lbValue> const&)Estimated);
    
    if ( isDirected() ) {
      temp.normalizeDirected();
      temp.multiplyByConditionalSumOfMeasure((lbTableMeasure<lbValue> const&)Empirical);
    }
    
    temp.subtractMeasure((lbTableMeasure<lbValue> const&)Empirical);
    temp.setIdleParams(_idleParams);
    temp.setSharedParams(_sharedParams);
    temp.enableSharedParams(SPM_SUM);
    temp.enableIdleParams();
    for ( int p=0 ; p<temp.getSize() ; p++ )
      res[index+p] = 0.0;
    int addOn = temp.extractValuesAddToVector(res,index,false); 

    return addOn;
  }


  template <class VALUE>
  inline typename lbTableMeasure<VALUE>::valueVec const& lbTableMeasure<VALUE>::getValueVec(lbBaseAssignment const& assign,
										   varsVec const& vars) const {
    //lbIndexConverter convi(_card, vars, _cardSize);
    //probIndex index = convi.assignToIndPartial(assign);
    probIndex index = assignToIndPartial(assign, vars);
    return (*_values)[index];
  }
 
  template <class VALUE>
  inline typename lbTableMeasure<VALUE>::valueVec & lbTableMeasure<VALUE>::getValueVec(lbBaseAssignment const& assign,
									      varsVec const& vars) {
    //lbIndexConverter convi(_card,vars,_cardSize);
    //probIndex index = convi.assignToIndPartial(assign);
    probIndex index = assignToIndPartial(assign, vars);
    _totalWeightIsUpdated = false;
    dirtySparsity();
    return (*_values)[index];
  }

 
  template <class VALUE>
  inline void lbTableMeasure<VALUE>::setParamVec(lbBaseAssignment const& assign,
						 varsVec const& vars,
						 valueVec const& params) {
    getValueVec(assign,vars) = params;
    dirtySparsity();
    _totalWeightIsUpdated = false;
  }


  template <class VALUE>
  inline int lbTableMeasure<VALUE>::setLogParams(probType const* vec, int index, bool& changed){
    changed = false;

    probType DEFAULT_LOG_VAL = log(LB_MIN_DOUBLE);

    for (uint i = 0; i < _primarySize; i++) {
      for(uint j = 0; j < _secondarySize; j++) {
	probType newVal = (vec[index]);
	probType diff = ABS((getValue(i,j).getLogValue() - newVal));

	if (diff > (lbDefinitions::PARAM_EPSILON)) {
	  if (newVal > DEFAULT_LOG_VAL) {
	    getValue(i,j).setLogValue(newVal);	  
	    changed = true;
	  }
	  else {
	    if ((getValue(i,j).getLogValue() != DEFAULT_LOG_VAL)){
	      cerr << "Warning, need to assign " << exp(newVal);
	      cerr << " assigning  " << exp(DEFAULT_LOG_VAL) << endl;
	      getValue(i,j).setLogValue(DEFAULT_LOG_VAL);
	      changed=true;
	    }
	  }
	}
	index++;
      }
    }

    if (changed) {
      _totalWeightIsUpdated = false;
      dirtySparsity();
    }

    if (_isDirected) {
      normalizeDirected();
    }

    return _totalSize;
  }

  // What is this for?
  template <class VALUE>
  inline varValue lbTableMeasure<VALUE>::randomValFromTable() {
    assert(_card.size() == 1);

    probType total = totalWeight();
    probType cummulative = 0.0;
    probType randD = _lbRandomProbGenerator.RandomDouble(1.0);

    for (varValue ind = 0; ind < _card[0]; ind++) {
      cummulative += getValue(0,ind).getValue();
      if ((cummulative/total) > randD){
	return (ind);
      }
    } 

    assert(false);

    return 0;
  }

  template <class VALUE>
  inline lbAssignment_ptr lbTableMeasure<VALUE>::getMAPassign (const varsVec & vars) const {
    lbAssignment_ptr res = new lbAssignment(vars) ;
    VALUE max ;
    uint maxI=0, maxJ=0 ;
    for (uint i = 0; i < getPrimarySize(); i++) {
      for(uint j = 0; j < getSecondarySize(); j++) {
        if (max < getValue(i,j)) {
          max = getValue(i,j) ;
          maxI = i ;
          maxJ = j ;
        }
      }
    }
    //cerr << "[DEBUG] Max is: " << max << ", position is: (" << maxI << "," << maxJ << ")" << endl ;

    int flatInd = getSecondarySize() * maxI + maxJ ;

    intVec intCardVec ;
    for (uint i=0 ; i<_cardSize ; ++i) {
      intCardVec.push_back (_card[i]) ;
    }

    intVec intAssignment = getTablePosByArrayIndex (flatInd , intCardVec) ;
    for (uint i=0 ; i<intAssignment.size() ; ++i) {
      res->setValueForVar (vars[i] , intAssignment[i]) ;
    }

    return res ;
  }

  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::addMeasure(lbMeasure const& other)  {
    lbTableMeasure const& otherTable = (lbTableMeasure const&) other; 
  
    _totalWeight.setValue(0);
    for (uint i = 0; i < _primarySize; i++){
      for (uint j = 0; j < _secondarySize; j++) {
	getValue(i,j) += otherTable.getValue(i, j);
	_totalWeight += getValue(i, j);
      }
    }
  
    _totalWeightIsUpdated = true;
    dirtySparsity();

    return true;
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::normalizeDirected()  {
    _totalWeight.setValue(0);
    _NORM_COUNT++;
    for (uint i = 0; i < _primarySize; i++) {
      VALUE vecSum;
      vecSum.setValue(0);
      for (uint j = 0; j < _secondarySize; j++) {
	_NORM_OP_COUNT++;
	vecSum += getValue(i,j);
      }
      
      VALUE zero;
      zero.setValue(0);

      if (vecSum != zero) {
	for (uint j = 0; j < _secondarySize; j++) {
	  getValue(i,j) /= vecSum;
	  _totalWeight += getValue(i,j);
	}
      }   
    }
  
    _totalWeightIsUpdated = true;
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::makeZeroes()  {
    _totalWeight.setValue(0);

    for (uint i = 0; i < _primarySize; i++) {
      for(uint j = 0; j < _secondarySize; j++) {
	getValue(i,j).setValue(0);
      }
    }

    _totalWeightIsUpdated = true;
    dirtySparsity();
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::makeRandom()  {
    _totalWeight.setValue(0);

    for (uint i = 0; i < _primarySize; i++) {
      for(uint j = 0; j < _secondarySize; j++) {
	probType rndNum = _lbRandomProbGenerator.RandomProb();
	rndNum /= (_totalSize);

	getValue(i,j).setValue(rndNum);
	_totalWeight += getValue(i,j);
      }
    }

    _totalWeightIsUpdated = true;
    dirtySparsity();
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::makeUniform()  {
    _totalWeight.setValue(0);

    for (uint i = 0; i < _primarySize; i++) {
      for(uint j = 0; j < _secondarySize; j++) {
	getValue(i,j).setValue(1.0);
	_totalWeight += getValue(i,j);
      }
    }

    _totalWeightIsUpdated = true;
    dirtySparsity();
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::makeSparse(double sparsity) {

    for (uint i = 0; i < (uint) (_totalSize*sparsity); i++) {
      int pindex = (int) (_lbRandomProbGenerator.RandomDouble(1.0)*_primarySize);
      int sindex = (int) (_lbRandomProbGenerator.RandomDouble(1.0)*_secondarySize);
      
      getValue(pindex,sindex).setValue(0);
    }

    // Set something to be non-zero
    if (sparsity > 0) {
      int pindex = (int) (_lbRandomProbGenerator.RandomDouble(1.0)*_primarySize);
      int sindex = (int) (_lbRandomProbGenerator.RandomDouble(1.0)*_secondarySize);
      getValue(pindex,sindex).setValue(.5);
    }

    _sparse = true;
    _totalWeightIsUpdated = false;
    dirtySparsity();
  }

  // ENTRYWIDE METHODS

  template <class VALUE>
  inline probType lbTableMeasure<VALUE>::valueOfFull(lbBaseAssignment const& assign,
					      varsVec const& vars) const {
    // The assignment can be any size it's the variables that we're
    // assuming match one to one with the cardinality of the measure.
    // We just need the variables to be somewhere within the assignment
    int primaryIndex = assign.getValueForVar(vars[_cardSize-1]);
    return getValueVec(assign,vars)[primaryIndex].getValue();
  }

  template <class VALUE>
  inline probType lbTableMeasure<VALUE>::logValueOfFull(lbBaseAssignment const& assign,
						 varsVec const& vars) const {
    // The assignment can be any size it's the variables that we're
    // assuming match one to one with the cardinality of the measure.
    // We just need the variables to be somewhere within the assignment
    int primaryIndex = assign.getValueForVar(vars[_cardSize-1]);
    return getValueVec(assign,vars)[primaryIndex].getLogValue();
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::setValueOfFull(lbBaseAssignment const& assign,
					     varsVec const& vars,
					     probType value)  {
    _totalWeightIsUpdated = false;
    dirtySparsity();
    getValue(assign, vars).setValue(value);
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::setLogValueOfFull(lbBaseAssignment const& assign,
						varsVec const& vars,
						probType value)  {
    _totalWeightIsUpdated = false;
    dirtySparsity();
    getValue(assign, vars).setLogValue(value);
  }


  // Slightly different between lbTableMeasure and lbTableMeasureNoLog
  template <class VALUE>
  inline void lbTableMeasure<VALUE>::subtractSumOfConditional(lbMeasure const & meas)  {

    for (uint i = 0; i < _primarySize; i++) {
      VALUE sum;
      sum.setValue(0);
uint j;
      for (j = 0; j < _secondarySize; j++) {
	sum += getValue(i,j);
      }

      VALUE size;
      size.setValue(_secondarySize);

      sum /= size;

      for (j = 0; j < _secondarySize; j++){
	getValue(i,j) /= sum;
      }
    }

    _totalWeightIsUpdated = true;
    dirtySparsity();
    _totalWeight.setValue(0);
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::divideMeasureByAnother(lbMeasure const& otherM) {

    lbTableMeasure const& otherTable = (lbTableMeasure const&)otherM;
    _totalWeight.setValue(0);

    _DIV_COUNT++;

    for (uint i = 0; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++) {

	VALUE zero;
	zero.setValue(0);
	
	VALUE otherParam = otherTable.getValue(i,j);
	VALUE & thisParam = getValue(i,j);

	_DIV_OP_COUNT++;

	/*
	if (otherParam == zero) {
	  getValue(i,j) = zero;
	}
	else {
	  getValue(i,j) /= otherParam;
	}
	*/
	
	// If 0/0 we leave it alone... otherwise
	if (otherParam != zero || thisParam != zero) {
	  thisParam /= otherParam;
	}

	_totalWeight += getValue(i,j);
      }
    }

    _totalWeightIsUpdated = true;
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::divideFromReplace(lbMeasure const& otherM) {

    lbTableMeasure const& otherTable = (lbTableMeasure const&)otherM;
    _totalWeight.setValue(0);

    _DIV_COUNT++;

    VALUE zero;
    zero.setValue(0);
        
    for (uint i = 0; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++) {
	VALUE otherParam = otherTable.getValue(i,j);
	VALUE & thisParam = getValue(i,j);

	_DIV_OP_COUNT++;

	// If 0/0 we leave it alone... otherwise
	if (otherParam != zero || thisParam != zero) {
	  thisParam = otherParam/thisParam;
	}
	if (otherParam == zero && thisParam != zero) {
	  NOT_REACHED;
	}

	_totalWeight += getValue(i,j);
      }
    }

    _totalWeightIsUpdated = true;
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::multiplyMeasureByAnother(lbMeasure const& otherM) {

    lbTableMeasure const& otherTable = (lbTableMeasure const&)otherM;
    _totalWeight.setValue(0);

    _MULT_COUNT++;

    for (uint i = 0; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++) {

	VALUE zero;
	zero.setValue(0);
	
	VALUE otherParam = otherTable.getValue(i,j);

	_MULT_OP_COUNT++;
	getValue(i,j) *= otherParam;
	_totalWeight += getValue(i,j);
      }
    }

    _totalWeightIsUpdated = true;
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::multiplyMeasureByNumber(probType num)  {
  
    _totalWeight.setValue(0);
    VALUE value;
    value.setValue(num);

    for (uint i = 0; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++) {
	getValue(i,j) *= value;
	_totalWeight += getValue(i,j);
      }
    }

    _totalWeightIsUpdated = true;
    dirtySparsity();
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::multiplyByConditionalSumOfMeasure(lbMeasure const& measure)  {
    lbTableMeasure const& otherTable = (lbTableMeasure const&) measure;
    _totalWeight.setValue(0);

    for (uint i = 0; i < _primarySize; i++) {
      VALUE sum;
      sum.setValue(0);

      uint j;

      for (j = 0; j < _secondarySize; j++) {
	sum += otherTable.getValue(i,j);
      }

      for (j = 0; j < _secondarySize; j++) {
	getValue(i,j) *= sum;
	_totalWeight += getValue(i,j);
      }
    }

    _totalWeightIsUpdated = true;
    dirtySparsity();
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::replaceValues(probType from, probType to) {

    VALUE fromVal;
    fromVal.setValue(from);

    VALUE toVal;
    toVal.setValue(to);

    _totalWeight.setValue(0);
    for (uint i = 0; i < _primarySize; ++i) {
      for (uint j = 0; j < _secondarySize; ++j) {
        VALUE& val = getValue(i,j);

        if (val == fromVal)
          val = toVal;

        _totalWeight += val;
      }
    }
  
    _totalWeightIsUpdated = true;
    dirtySparsity();
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::normalize()  {

    totalWeight();
    VALUE one;
    one.setLogValue(0);

    VALUE zero;
    zero.setValue(0);

    if (_totalWeight != zero && _totalWeight != one) {
      updateSparsity();

      _NORM_COUNT++;
      
      if (_sparse) {
	for (uint i = 0; i < _nonZeroEntries.size(); i++) {
	  _NORM_OP_COUNT++;
	  getValue(_nonZeroEntries[i], _psuedoVars) /= _totalWeight;
	}
      }
      else {
	for (uint i = 0; i < _primarySize; i++) {
	  for (uint j = 0; j < _secondarySize; j++) {
	    _NORM_OP_COUNT++;
	    getValue(i,j) /= _totalWeight;
	  }
	}
      }
    }
    
    _totalWeight.setLogValue(0);
    _totalWeightIsUpdated = true;
  }


  template <class VALUE>
    inline int lbTableMeasure<VALUE>::extractValuesAddToVector(probType* vec,int index,bool logValues) const {
    for (uint i = 0; i < _primarySize; i++) {
      for(uint j = 0; j < _secondarySize; j++) {
	if ( logValues ) 
	  vec[index] += getValue(i,j).getLogValue();
	else
	  vec[index] += getValue(i,j).getValue();
	index++;
      }
    }
    return _totalSize;
  }
  
  template <class VALUE>
    inline int lbTableMeasure<VALUE>::extractLogParamsAddToVector(probType* vec,int index) const {

    for (uint i = 0; i < _primarySize; i++) {
      for(uint j = 0; j < _secondarySize; j++) {
	vec[index] += getValue(i,j).getLogValue();
	index++;
      }
    }
    return _totalSize;
  }

  template <class VALUE>
  inline probVector lbTableMeasure<VALUE>::measure2Vec() const {
    probVector res (_totalSize) ;
    int index = 0 ;
    for (uint i = 0; i < _primarySize; i++) {
      for(uint j = 0; j < _secondarySize; j++) {
        res[index++] = getValue(i,j).getValue() ;
      }
    }
    return res ;
  }

  template <class VALUE>
  inline probType lbTableMeasure<VALUE>::totalWeight()  {
    if (!_totalWeightIsUpdated) {
      _totalWeight.setValue(0);
      updateSparsity();

      if (_sparse) {
	for (uint i = 0; i < _nonZeroEntries.size(); i++) {
	   _totalWeight += getValue(_nonZeroEntries[i], _psuedoVars);
	}
      }
      else {
	for (uint i = 0; i < _primarySize; i++) {
	  for (uint j = 0; j < _secondarySize; j++) {
	    _totalWeight += getValue(i,j);
	  }
	}
      }

      _totalWeightIsUpdated = true;
    }

    VALUE zero;
    zero.setValue(0);

    if (_totalWeight == zero) {
      if (!(lbOptions::isVerbose(V_IGNORE_TOTAL_WEIGHT_0)) && _totalWeight == zero) {
        cerr<<"Warning :  Total Weight is 0"<<endl;
      }
    }

    return _totalWeight.getValue();
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::addSharedParams(assignmentPtrVec const & vec,varsVec const& vars)  {
    uint size = vec.size();
    _sharedParams.push_back(intVec(size));
    //lbIndexConverter convi(_card,vars,_cardSize);

    for (uint i=0;i<size;i++) {
      _sharedParams[_numOfSharedParams][i] = assignToInd(*(vec[i]),vars);
    }

    _numOfSharedParams++;
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::addIdleParams(assignmentPtrVec const & vec, varsVec const& vars)  {
    uint size = vec.size();
    //lbIndexConverter convi(_card,vars,_cardSize);
    for (uint i=0;i<size;i++) {
      _idleParams.push_back(assignToInd(*(vec[i]),vars));
    }
  }

  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::updateMeasureValues(lbMeasure const& other, probType smooth,
							 probType epsilon)  {
    // cerr<<"Inside updateing measurs values (table measure)"<<endl;
    bool result = false;
    if (smooth == 1.0) {
      cerr << "Warning: smoothing has no effect" << endl;
    }
    else {
      lbTableMeasure const& updatedTable = (lbTableMeasure const&) other;
      _totalWeight.setValue(0);
      VALUE smoothParam;
      smoothParam.setValue(smooth);

      VALUE one;
      one.setValue(1);

      for (uint i = 0; i < _primarySize; i++) {
	for(uint j = 0; j < _secondarySize; j++) {


	  VALUE thisParam = getValue(i,j);
	  VALUE thatParam = updatedTable.getValue(i,j);
	  VALUE tmp;
	
	  if (smooth == 0.0) { //for efficiency
	    tmp = thatParam;
	  }
	  else if (LOG_SMOOTH) {
	    // SHOULD I SMOOTH IN ln SPACE?
	    tmp.setLogValue(thisParam.getLogValue()*smooth + thatParam.getLogValue()*(1 - smooth));
	  }
	  else {
	    assert(smooth >= 0 && smooth < 1.0);
	    VALUE smoothParam;
	    smoothParam.setValue(smooth);
	    tmp = thisParam*smoothParam + thatParam*(one - smoothParam);
	  }
	  
	  if (epsilon < 0 ||
	      ABS(thisParam.getLogValue()-tmp.getLogValue()) > epsilon) {
	    getValue(i,j) = tmp;
	    result = true;
	  }

	  _totalWeight += getValue(i,j);
	}
      }

      _totalWeightIsUpdated = true;
      dirtySparsity();
    }

    return result;
  }

  
  template <class VALUE>
  inline void lbTableMeasure<VALUE>::addLogOfQuotient(lbMeasure const& numerator,lbMeasure const& denominator)  {
    lbTableMeasure const& numerTable = (lbTableMeasure const&) numerator;
    lbTableMeasure const& denominTable = (lbTableMeasure const&) denominator;

    for (uint i = 0 ; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++){
	VALUE numer = numerTable.getValue(i,j);
	VALUE denomin = denominTable.getValue(i,j);

	VALUE quotient;
	quotient.setValue(0);
      
	VALUE zero;
	zero.setValue(0);

	VALUE eps;
	eps.setValue(lbDefinitions::EPSILON);

	if (denomin != zero) {
	  quotient = numer/denomin;
	}
	else {
	  quotient = numer/(denomin + eps);
	}

	if (quotient != zero) {
	  getValue(i,j) *= quotient;
	}
	else {
	  getValue(i,j) *= eps;
	}
      }
    }

    _totalWeightIsUpdated = false;
    dirtySparsity();
    normalize();
  }


  // SHARED / IDLE PARAMETERS

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::setSharedParams(map<string , intVec> const& sharedIndices) {
    map<string, intVec>::const_iterator iter;
    _sharedParams.clear();

    for (iter = sharedIndices.begin();
	 iter != sharedIndices.end();
	 iter++) {
      intVec temp = iter->second;
      uint sharedSize = temp.size();

      if (sharedSize > 0) {
	_sharedParams.push_back(intVec(sharedSize));
      
	for (uint i = 0; i < sharedSize; i++) {
	  _sharedParams[_numOfSharedParams][i] = temp[i];
	}
	_numOfSharedParams++;
      }
    }
 
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::enableSharedParams(lbSharedParamMode mode)  {
    // Go over all shared parameters
    for (int sharedInd=0;sharedInd<_numOfSharedParams;sharedInd++) {
      // For each one calculate the average
      VALUE val;
      val.setValue(0.0);

      uint numOfShared = _sharedParams[sharedInd].size();
	  uint ind;
      for (ind = 0; ind < numOfShared; ind++) {
	int temp = _sharedParams[sharedInd][ind];
	int firstInd = temp/_card[_cardSize-1];
	int secInd = temp%_card[_cardSize-1];
	val += getValue(firstInd,secInd);
      }

      if(mode == SPM_AVERAGE) {
	VALUE numShared;
	numShared.setValue(numOfShared);
	val = val/numShared;
      }

      // Replace all values with average.
      for (ind = 0; ind < numOfShared; ind++) {
	int temp = _sharedParams[sharedInd][ind];
	int firstInd = temp/_card[_cardSize-1];
	int secInd = temp%_card[_cardSize-1];
	getValue(firstInd,secInd) = val;
      }
    }

    dirtySparsity();
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::enableIdleParams()  {
    for (uint ind = 0; ind < _idleParams.size(); ind++) {
      int temp = _idleParams[ind];
      int firstInd = temp/_card[_cardSize-1];
      int secInd = temp%_card[_cardSize-1];
      getValue(firstInd,secInd).setValue(0);
    }

    dirtySparsity();
  }

  // All that need be initialized is the card vec.
  template <class VALUE>
  inline void lbTableMeasure<VALUE>::buildProbVec(bool random)  {
    if (_values) {
      delete _values;
    }
  
    if (_cardSize == 0) {
      _primarySize = 0;
      _secondarySize = 0;
      _values = new valueVecVec(0);
    }
    else {
      _totalSize = 1;
      for (uint i = 0; i < _card.size(); i++) {
	_totalSize *= _card[i];
      }

      _primarySize = (_totalSize/_card[_cardSize-1]);
      _secondarySize = _card[_cardSize-1];
      _values = new valueVecVec(_primarySize);
    }

    _totalWeight.setValue(0);

    for(uint i = 0; i < _primarySize; i++) {
      (*_values)[i] = valueVec(_secondarySize);
    }
    _vectorsAreInitialized = true;

    if (random) {
      makeRandom();
    }
    else {
      makeUniform();
    }
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::buildProbVec(int len, bool random)  {
    NEEDS_CLEANUP; // is it?
  }

  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::raiseToThePower(probType power)  {
    _totalWeight.setValue(0);

    for (uint i = 0; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++) {
	getValue(i,j) = getValue(i,j).raiseToThePower(power);
	_totalWeight += getValue(i,j);
      }
    }
    _totalWeightIsUpdated = true;
    return true;
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::takeLog()  {

    _totalWeight.setValue(0);
	
    for (unsigned int i = 0; i < _primarySize; i++) {
      for (unsigned int j = 0; j < _secondarySize; j++) {
	getValue(i,j) = getValue(i,j).takeLog() ;
	_totalWeight += getValue(i,j);
      }
    }
    _totalWeightIsUpdated = true;
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::takeExp()  {

    _totalWeight.setValue(0);
	
    for (unsigned int i = 0; i < _primarySize; i++) {
      for (unsigned int j = 0; j < _secondarySize; j++) {
	getValue(i,j) = getValue(i,j).takeExp() ;
	_totalWeight += getValue(i,j);
      }
    }
    _totalWeightIsUpdated = true;
  }


  //////////////////////
  // SPARSITY METHODS //
  //////////////////////

  template <class VALUE>
  inline double lbTableMeasure<VALUE>::getSparsityRatio() const {
    int totalZeroes = 0;

    VALUE zero;
    zero.setValue(0);

    for (uint i = 0; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++) {
	if (getValue(i,j) == zero) {
	  totalZeroes++;
	}
      }
    }

    return (double) totalZeroes / (_primarySize * _secondarySize);
  }

  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::getSparse() const {
    return _sparse;
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::setSparse(bool sparse) const {
    _sparse = sparse;
    _sparsityUpdated = true;

    _nonZeroEntries.clear();
    _psuedoVars.clear();

    if (sparse) {
      VALUE zero;
      zero.setValue(0);
            
      for (uint i = 0; i < _card.size(); i++) {
	_psuedoVars.push_back(i);
      }
      
      lbSmallAssignment assign = lbSmallAssignment(_psuedoVars);
      
      do {
	if (getValue(assign, _psuedoVars) != zero) {
	  _nonZeroEntries.push_back(lbSmallAssignment(assign));
	}
      } while(assign.advanceOne(_card, _psuedoVars));  
    }
  }

  
  //////////////////////////
  // INPUT/OUTPUT METHODS //
  //////////////////////////
  template <class VALUE>
  inline void lbTableMeasure<VALUE>::readOneMeasure(ifstream_ptr in,
						    paramPtrMap & sharedMap,
						    paramPtrMap & idleMap,
						    bool readLogValues)  {
    // Set up cardinality vector
    char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
    (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE, lbDefinitions::DELIM);
    string str = string(buffer.get());
    
    istringstream iss(str);
   
    iss >> _cardSize;
    if (_cardSize<=0) {
      cerr<<"Error, while reading measure from file card size <= 0: "<<_cardSize<<endl;
      exit(1);
    } 
    _card = cardVec(_cardSize);

    readCards(in);
    buildProbVec(false /* Random */);
    readParameters(in, sharedMap, idleMap, readLogValues);
  }
 
  template <class VALUE>
  inline void lbTableMeasure<VALUE>::print(ostream& out) const {
    out << "Printing Measure " << "\nCARDS: ";
   
    printVector(_card, out);
   
    out << "PROBS: \n";
    for (uint i = 0; i < _primarySize; i++) {
      out << i << ": ";
      for (uint j = 0; j < _secondarySize; j++) {
	out << getValue(i,j) << " ";
      }
      out<<endl;
    }
  }
 
  template <class VALUE>
    inline void lbTableMeasure<VALUE>::printToFastInfFormat(ostream& out,bool normalizeValues,int prec,
							    bool printLogValues) const {
    double total = 1.0;
    if (printLogValues) {
      total = 0.0;
    }
    
    if ( normalizeValues ) {
      if (printLogValues) {
	total = getTotalWeight().getLogValue();
      }
      else {
	total = getTotalWeight().getValue();
      }
    }
    out << _card.size() << "\t";
    for (uint k = 0; k < _card.size(); k++)
      out << _card[k] << " ";
   
    out << "\t";    for (uint i = 0; i < _primarySize; i++) {
      for (uint j = 0; j < _secondarySize; j++) {
	if (printLogValues) {
	  out << setprecision(prec) << getValue(i,j).getLogValue() - total << " ";
	}
	else {
	  out << setprecision(prec) << getValue(i,j).getValue()/total << " ";
	}
      }
    }
    out << endl;
  }
 
  template <class VALUE>
  inline void lbTableMeasure<VALUE>::printNZEs(int size) const { 
    if (!_sparsityUpdated) {
      setSparse(_sparse);
    }
    
    cerr << "Num non-zero assignments: " << _nonZeroEntries.size() << endl;
    for (uint i = 0; i < _nonZeroEntries.size(); i++) {
      _nonZeroEntries[i].print(cerr,size);
    }
    cerr << endl << endl << endl;
  }

 
 
  ////////////////////
  // HELPER METHODS //
  ////////////////////
 
  template <class VALUE>
  inline pair<cardVec,varsVec> lbTableMeasure<VALUE>::buildSubSet(internalIndicesVec const& indices,
							   varsVec const& oldVec) const {
    uint size = indices.size();
    varsVec tmpVars = varsVec(size);
    cardVec tmpCard = cardVec(size);
    for (uint i = 0; i < size; i++){
      tmpCard[i]= _card[indices[i]];
      tmpVars[i]= oldVec[indices[i]];
    }
    return pair<cardVec,varsVec>(tmpCard,tmpVars);
  }
 

  // Find the ones that are in the old vec but not in the new one
  // (they are marginalized) and find the indices of the remaining
  // vars.
  template <class VALUE>
  inline pair<internalIndicesVec, internalIndicesVec> lbTableMeasure<VALUE>::findMarginVars(varsVec const& newVec,
										     varsVec const& oldVec) const {
    internalIndicesVec marginalized = internalIndicesVec();
    internalIndicesVec remaining = internalIndicesVec();
   
    uint newSize = newVec.size();

    for (uint ind = 0; ind < oldVec.size(); ind++) {
      bool found	= false;

      for (uint j = 0; j < newSize; j++) {
	if (newVec[j] == oldVec[ind]) {
	  found = true;  
	}
      }

      if (!found) {
	marginalized.push_back(ind);
      }
      else{
	remaining.push_back(ind);
      }
    }

    pair <internalIndicesVec,internalIndicesVec> result(marginalized,remaining);

    return result;
  }
 

  template <class VALUE>
  inline  void lbTableMeasure<VALUE>::readCards(ifstream_ptr in) {
    char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
    (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE,lbDefinitions::DELIM);
    string str = string(buffer.get());
    istringstream iss(str);
    _totalSize = 1;
    for (uint i = 0 ; i < _cardSize; i++) {
      int tmp;
      iss >> tmp;
      _card[i] = tmp;
      _totalSize *= _card[i];
    }
    _secondarySize = _card[_cardSize-1];
    _primarySize = _totalSize/_secondarySize;
    updateStrideVector();
  }
 
  template <class VALUE>
  inline void lbTableMeasure<VALUE>::readParameters(ifstream_ptr in,
						    paramPtrMap & sharedMap,
						    paramPtrMap & idleMap,
						    bool readLogValues) {
    stringbuf lineBuf;
    in->get(lineBuf);
    in->get(); //get '\n'
    string str = lineBuf.str();
    istringstream iss(str);
   
    map<string , intVec> sharedIndices = map<string , intVec>();
   
    _totalWeight.setValue(0);
    for (uint i = 0; i < _values->size(); i++) {
      for (int j = 0; j < _card[_cardSize-1]; j++) {
	string probS;
	probType prob;
	iss >> probS;
	char c = probS[0];
       
	if (c == 'i') {
	  lbParam* pp = idleMap[probS];
	  prob = pp->getVal();
	  int idleIndex = (_card[_cardSize-1]*i) + j;
	  _idleParams.push_back(idleIndex);
	}
	else if(c == 's'){
	  lbParam * pp = sharedMap[probS];
	  prob = pp->getVal();
	  int sharedIndex = (_card[_cardSize-1]*i) + j;
	  sharedIndices[pp->getName()].push_back(sharedIndex);
	}
	else {
	  prob = atof(probS.c_str());
	}

        /*
        // TEMP:
        probType randNoise = _lbRandomProbGenerator.SampleNormal (prob*0.01) ;
        //cerr << "[DEBUG] prob=" << prob << " noise=" << randNoise << endl ;
        prob += randNoise ;
        if (prob < 0) {
          prob = lbDefinitions::EPSILON ;
        }
        // END TEMP
        */
	if (readLogValues) {
	  getValue(i,j).setLogValue(prob);
	}
	else {
	  getValue(i,j).setValue(prob);
	}
	
	_totalWeight += getValue(i,j);
      }
    }
   
    setSharedParams(sharedIndices);
    _vectorsAreInitialized = true;
    _totalWeightIsUpdated = true;
    dirtySparsity();
  }
 
  template <class VALUE>
  inline intVec lbTableMeasure<VALUE>::getTablePosByArrayIndex (int index ,
								const intVec & cards) const
  {
    assert (cards.size() > 0) ;

    uint num_cards = cards.size() ;
    int table_size = 1 ;
    for (uint i = 0 ; i < num_cards ; ++i) {
      table_size *= cards[i] ;
    }

    assert (index < table_size) ;


    intVec table_positions (num_cards) ;
    int residual = index ;

    for (uint i = 0 ; i < num_cards ; ++i) {
      int reverse_pos = num_cards-(i+1) ;
      table_positions[reverse_pos] = residual % cards[reverse_pos] ;
      residual /= cards[reverse_pos] ;
    }

    return table_positions ;
  }

  template <class VALUE>
  inline VALUE & lbTableMeasure<VALUE>::getValue(int i, int j) const {
    assert((uint) i < _primarySize && (uint) j < _secondarySize);
    assert(i >= 0 && j >= 0);
    return (*_values)[i][j];
  }

  template <class VALUE>
  inline VALUE & lbTableMeasure<VALUE>::getValue(lbBaseAssignment const& assign,
	 					varsVec const& vars) {
    //    lbIndexConverter convi(_card,vars,_cardSize);
    //probIndex index = convi.assignToIndPartial(assign);
    probIndex index = assignToIndPartial(assign, vars);
    rVarIndex v = vars[_cardSize-1];
    int tmp = assign.getValueForVar(v);
    return (*_values)[index][tmp];
  }

  template <class VALUE>
  inline VALUE const& lbTableMeasure<VALUE>::getValue(lbBaseAssignment const& assign,
						     varsVec const& vars) const {
    //lbIndexConverter convi(_card,vars,_cardSize);
    //probIndex index = convi.assignToIndPartial(assign);
    probIndex index = assignToIndPartial(assign, vars);
    rVarIndex v = vars[_cardSize-1];
    int tmp = assign.getValueForVar(v);
    return (*_values)[index][tmp];
  }
  
  template <class VALUE>
  inline void lbTableMeasure<VALUE>::printStats() {
    cerr << endl;
    cerr << "Marginalizations: count(" << _MARG_COUNT << ") ops(" << _MARG_OP_COUNT << ")" << endl;
    cerr << "Multiplication  : count(" << _MULT_COUNT << ") ops(" << _MULT_OP_COUNT << ")" << endl;
    cerr << "Divisions       : count(" << _DIV_COUNT << ") ops(" << _DIV_OP_COUNT << ")" << endl;
    //    cerr << "Normalizations  : count(" << _NORM_COUNT << ") ops(" << _NORM_OP_COUNT << ")" << endl;
    //    cerr << "TOTAL           : count(" << (_MARG_COUNT + _MULT_COUNT + _DIV_COUNT + _NORM_COUNT) << ") ops(";
    cerr << "TOTAL           : count(" << (_MARG_COUNT + _MULT_COUNT + _DIV_COUNT) << ") ops(";
    //    cerr << (_MARG_OP_COUNT + _MULT_OP_COUNT + _DIV_OP_COUNT + _NORM_OP_COUNT) << ")" << endl;
    cerr << (_MARG_OP_COUNT + _MULT_OP_COUNT + _DIV_OP_COUNT) << ")" << endl;
    cerr << endl;
    
    _MARG_COUNT = _MULT_COUNT = _DIV_COUNT = _NORM_COUNT = 0;
    _MARG_OP_COUNT = _MULT_OP_COUNT = _DIV_OP_COUNT = _NORM_OP_COUNT = 0;
  }
  
  template <class VALUE>
  inline void lbTableMeasure<VALUE>::updateSparsity() const {
    if (!_sparsityUpdated) {
      setSparse(_sparse);
    }
  }


  // Sparse marginalizations (without assignment)
  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::margiplyOntoSparse(lbTableMeasure& newTable,
						 cardVec const& tmpCard,
						 varsVec const& tmpVars,
						 varsVec const& newVec,
						 varsVec const& oldVec,
						 varsVec const& fullVec) const {
    assert(newTable.getSparse());
    
    newTable.updateSparsity();
    updateSparsity();

    if (newTable._nonZeroEntries.empty()) {
      return true;
    }

    lbSmallAssignment assign = lbSmallAssignment(fullVec);

    assert(newVec.size() == newTable._psuedoVars.size());

    for (uint i = 0; i < newTable._nonZeroEntries.size(); i++) {
      fillFullAssignment(assign,
			 tmpVars,
			 newVec,
			 newTable._psuedoVars,
			 newTable._nonZeroEntries[i]);

      /*      
      assign.zeroise(tmpVars);
      for (uint v = 0; v < newVec.size(); v++) {
	assign.setValueForVar(newVec[v], newTable._nonZeroEntries[i].getValueForVar(newTable._psuedoVars[v]));
      }
      */

      //for all assignments on remaining vars
      VALUE prob;
      prob.setValue(0);

      do {
	VALUE tmp = getValue(assign, oldVec);
	_MULT_OP_COUNT++;

	if (MAX_PRODUCT) {
	  if (tmp > prob) {
	    prob = tmp;
	  }
	}
	else {
	  prob += tmp;
	}
      } while(assign.advanceOne(tmpCard, tmpVars));

      newTable.getValue(assign, newVec) *= prob;
    }

    newTable._totalWeightIsUpdated = false;
    return true;
  }
  
  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::marginalizeFromSparse(lbTableMeasure& newTable,
							   cardVec const& extraCard,
							   varsVec const& extraVars,
							   varsVec const& newVec,
							   varsVec const& oldVec,
							   varsVec const& fullVec,
							   bool multiply) const {
    assert(getSparse());
    
    newTable.updateSparsity();
    updateSparsity();

    lbMeasure_Sptr dummyTable;
    dummyTable = newTable.duplicate();

    bool wasSparse = newTable.getSparse();
    newTable.setSparse(false);
    newTable.makeZeroes();
    
    if (_nonZeroEntries.empty()) {
      return true;
    }

    lbSmallAssignment assign = lbSmallAssignment(fullVec);

    assert(oldVec.size() == _psuedoVars.size());
    for (uint i = 0; i < _nonZeroEntries.size(); i++) {
      fillFullAssignment(assign,
			 extraVars,
			 oldVec,
			 _psuedoVars,
			 _nonZeroEntries[i]);
      /*
      assign.zeroise(extraVars);
      for (uint v = 0; v < oldVec.size(); v++) {
	assign.setValueForVar(oldVec[v], _nonZeroEntries[i].getValueForVar(_psuedoVars[v]));
      }
      */

      do {
	VALUE prob = newTable.getValue(assign, newVec);
	VALUE tmp = getValue(assign, oldVec);
	
	if (multiply) {
	  _MULT_OP_COUNT++;
	}
	else {
	  _MARG_OP_COUNT++;
	}

	if (MAX_PRODUCT) {
	  if (tmp > prob) {
	    newTable.getValue(assign, newVec) = tmp;
	  }
	}
	else {
	  newTable.getValue(assign, newVec) += tmp;
	}
      } while(assign.advanceOne(extraCard, extraVars));

    }

    if (multiply) {
      // Multiply by the original
      newTable.multiplyMeasureByAnother(*dummyTable);
    }

    newTable.setSparse(wasSparse);
    newTable._totalWeightIsUpdated = false;
    
    return true;
  }

  template <class VALUE>
  inline bool lbTableMeasure<VALUE>::marginalizeWithoutAssign(lbTableMeasure& table,
						       cardVec const& margCard,
						       varsVec const& margVars,
						       varsVec const& newVec,
						       varsVec const& oldVec,
						       varsVec const& fullVec,
						       bool multiply) const {
    lbTableMeasure& newTable = (lbTableMeasure&) table;
    // IMPORTANT: assignment is created with the correct variables for synch to work between measures
    lbSmallAssignment assign = lbSmallAssignment(fullVec);
    
    // go over all assignment of target measure
    do {
      //for all assignments on remaining vars
      VALUE prob;
      prob.setValue(0);

      // sum over all values of variables that are marginalized out
      do {
	VALUE tmp = getValue(assign, oldVec);

	if (multiply) {
	  _MULT_OP_COUNT++;
	}
	else {
	  _MARG_OP_COUNT++;
	}

	if (MAX_PRODUCT) {
	  if (tmp > prob) {
	    prob = tmp;
	  }
	}
	else {
	  prob += tmp;
	}
      } while(assign.advanceOne(margCard, margVars));

      // read to put marginalized value into proper "cell" and multiply if needed
      if (multiply) {
	newTable.getValue(assign, newVec) *= prob;
      }
      else {
	newTable.getValue(assign, newVec) = prob;
      }
    } while(assign.advanceOne(newTable._card, newVec));

    newTable._totalWeightIsUpdated = false;
    return true;
  }

  template <class VALUE>
  bool lbTableMeasure<VALUE>::marginalizeWithAssign(lbTableMeasure& table,
					     cardVec const& margCard,
					     varsVec const& margVars,
					     varsVec const& newVec,
					     varsVec const& oldVec,
					     varsVec const& fullVec,
					     lbBaseAssignment const& matchAssign,
					     bool multiply) const {
    lbTableMeasure & newTable = (lbTableMeasure&) table;
    lbSmallAssignment assign = lbSmallAssignment(fullVec);
  
    // Now define moving vars only from those that are not assigned.
    varsVec movingVars = varsVec();
    cardVec movingCard = cardVec();



    for (uint v = 0; v < margVars.size(); v++) {
      if (!matchAssign.isAssigned(margVars[v])) {
	movingCard.push_back(margCard[v]);
	movingVars.push_back(margVars[v]);
      }
      else {
	assign.setValueForVar(margVars[v], matchAssign.getValueForVar(margVars[v]));
      }
    }

    do {
      // For all assignments on remaining vars
      VALUE param;
      param.setValue(0);

      do { 
	VALUE tmp = getValue(assign,oldVec);

	if (multiply) {
	  _MULT_OP_COUNT++;
	}
	else {
	  _MARG_OP_COUNT++;
	}

	if (MAX_PRODUCT) {
	  if (tmp > param)
	    param = tmp;
	}
	else {
	  param += tmp;
	}
      } while( assign.advanceOne(movingCard,movingVars));

      if (multiply) {
	newTable.getValue(assign,newVec) *= param;
      } 
      else {
	newTable.getValue(assign,newVec) = param;
      }

      if ( isnan(newTable.getValue(assign,newVec).getValue()) )
          newTable.getValue(assign,newVec).setValue(0.0);

    } while(assign.advanceOne(newTable._card,newVec));

    newTable._totalWeightIsUpdated = false;
    return true;
  }

  template <class VALUE>
  inline pair<varsVec, cardVec> lbTableMeasure<VALUE>::varsCardsVecMinus(varsVec const& v1,
								  cardVec const& c1,
								  varsVec const& v2) const {
    assert(v1.size() == c1.size());

    internalIndicesVec intind = internalIndicesVec();

    varsVec ansVars;
    cardVec ansCard;

	uint i;
    for (i = 0; i < v1.size(); i++) {
      bool found = false;

      for (uint j = 0; j < v2.size(); j++) {
	if (v1[i] == v2[j]) {
	  found = true;  
	}
      }

      if (!found) {
	intind.push_back(i);
      }
    }

    for (i = 0; i < intind.size(); i++) {
      ansVars.push_back(v1[intind[i]]);
      ansCard.push_back(c1[intind[i]]);
    }

    pair <varsVec, cardVec> result(ansVars, ansCard);
    return result;
  }

  // produces a union list of variables for current and target measure
  template <class VALUE>
    inline pair<varsVec, cardVec> lbTableMeasure<VALUE>::varsCardsVecPlus(varsVec const& v1, // current measure
									  cardVec const& c1,
									  varsVec const& v2, // target measure
									  cardVec const& c2) const {
    assert(v1.size() == c1.size());
    assert(v2.size() == c2.size());

    internalIndicesVec intind = internalIndicesVec();

    varsVec ansVars;
    cardVec ansCard;

    uint i;
    // go over variables in THIS measure in order
    for ( i = 0; i < v1.size(); i++) {
      bool found = false;

      // see if variable is in target measure
      for (uint j = 0; j < v2.size(); j++) {
	if (v1[i] == v2[j]) {
	  found = true;  
	}
      }

      // if variable is in target measure, record it
      if (!found) {
	intind.push_back(i);
      }
    }

    ansVars = v2; // take all variables in target
    ansCard = c2;

    // and also the ones in current measure and not in the other
    for (i = 0; i < intind.size(); i++) {
      ansVars.push_back(v1[intind[i]]);
      ansCard.push_back(c1[intind[i]]);
    }

    pair <varsVec, cardVec> result(ansVars, ansCard);
    return result; 
  }

  template <class VALUE>
    inline void lbTableMeasure<VALUE>::setSparsityThresh(double sparseLevel) const
    {
      if ( getSparse() > sparseLevel )
	setSparse(true);
      else
	setSparse(false);
    }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::subtractMeasure(lbMeasure const& other)  {
    
    lbTableMeasure const& otherTable = (lbTableMeasure const&) other; 
    
    _totalWeight.setValue(0);
    for (uint i = 0; i < _primarySize; i++){
      for (uint j = 0; j < _secondarySize; j++) {
	getValue(i,j) -= otherTable.getValue(i, j);
	_totalWeight += getValue(i,j);
      }
    }
    
    _totalWeightIsUpdated = true;
    dirtySparsity();
  }

  template <class VALUE>
  inline void lbTableMeasure<VALUE>::setIdleParams(intVec const& params) {
    _idleParams = intVec(params);
  };
 
  template <class VALUE>
  inline void lbTableMeasure<VALUE>::setSharedParams(intVecVec const& params) {
    _sharedParams = intVecVec(params);
    _numOfSharedParams = params.size();
  };
}; 

#endif

      


