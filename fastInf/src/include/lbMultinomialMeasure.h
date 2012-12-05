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

#ifndef _Loopy__MULTINOMIAL_Measure
#define _Loopy__MULTINOMIAL_Measure

#include <lbMeasure.h>
#include <lbMathFunctions.h>

namespace lbLib {

  /**
     This Class holds the ABSTRACT MultinomialMeasure for the graph

     This Object holds the probabilistic characters of a clique.
     and performs simple actions on it
   
     Part of the loopy belief library
     @version November 2002
     @author Ariel Jaimovich
  */

  class lbMultinomialMeasure : public lbMeasure {
	
  public:

    inline lbMultinomialMeasure();

    inline lbMultinomialMeasure(cardVec const& card);

    inline explicit lbMultinomialMeasure(lbMultinomialMeasure const& oldMeasure) ;

    inline explicit lbMultinomialMeasure(lbMeasure const& oldMeasure) ;

    inline virtual ~lbMultinomialMeasure();

    inline virtual lbMeasure& operator=(lbMeasure const& oldMeasure);

    inline virtual lbMultinomialMeasure& operator=(lbMultinomialMeasure const& oldMeasure);
    
    virtual probType valueOfFull(lbBaseAssignment const& assign,
			       varsVec const& vars) const =0;
   
    virtual probType logValueOfFull(lbBaseAssignment const& assign,
				  varsVec const& vars) const=0 ;
    
    virtual void setValueOfFull(lbBaseAssignment const& assign,
				varsVec const& vars,
				probType value)=0 ;
    
    virtual void setLogValueOfFull(lbBaseAssignment const& assign,
				   varsVec const& vars,
				   probType value) = 0;

    virtual bool addMeasure(lbMeasure const& otherMeasure) = 0;

    virtual bool operator==(lbMeasure const& otherMeasure)=0 ;

    virtual bool marginalize(lbMeasure& newMeasure,
				    varsVec const& newVec,
				    varsVec const& oldVec,
				    bool multiply) const = 0;
    
    virtual bool marginalizeWithAssign(lbMeasure& newMeasure,
					      varsVec const& newVec,
					      varsVec const& oldVec,
					      lbBaseAssignment const& assign,
					      bool multiply) const = 0;
    
    virtual bool combineMeasures(lbMeasure& newtable,
					const lbMeasure& table,
					const varsVec& leftVec,
					const varsVec& rightVec,
					CombineType ct) const = 0;

    virtual bool raiseToThePower(probType power) = 0;
   
    virtual void readOneMeasure(ifstream_ptr in,
				paramPtrMap & sharedMap,
				paramPtrMap & idleMap,
				bool readLogValues = false)=0;

    
    virtual void print(ostream& out) const=0;

    
    virtual void printToFastInfFormat(ostream& out,bool normalizeValue = false,int prec = 5,
				      bool printLogValues = false) const=0;
      
    inline virtual probType operator() (lbBaseAssignment const& assign,
				      varsVec const& vars) const ;

    virtual bool updateAssign(lbAssignment const& givenAssign, varsVec const& vars,
			      lbMeasure const& original) ;

    virtual bool updateAssign(lbAssignment const& givenAssign, varsVec const& vars) ;
    
    inline virtual bool isDifferent(lbMeasure const& other,const lbMessageCompareType type,probType epsilon) const;

    virtual void normalize() =0;

    virtual void normalizeDirected() =0;
    
    virtual probType totalWeight()=0;

    virtual varValue randomValFromTable() = 0;
    
    virtual lbMeasure_Sptr duplicate() const =0;

    // for table that have complex inner representation,
    // duplicate values is used for inference purposes
    virtual lbMeasure_Sptr duplicateValues() const =0;

    virtual bool updateMeasureValues(lbMeasure const& other,probType smoothParam,
				     probType epsilon = -1) =0;

    virtual void makeZeroes() = 0;

    virtual void makeRandom() = 0;

    virtual void makeUniform() = 0;

    virtual void makeSparse(double sparsity) = 0;

    virtual int getParamNum() const = 0;

    virtual int calcDeriv(probType* res,int index,
			  lbMeasure const& Estimated,
			  lbMeasure const& Empirical) const = 0;

    virtual int extractValuesAddToVector(probType* vec,int index,bool logValues) const =0;
    
    virtual int extractLogParamsAddToVector(probType* vec,int index) const =0;
    
    virtual probVector measure2Vec() const =0;

    virtual int setLogParams(probType const* vec,int index,bool& changed)=0;
    

    virtual void subtractMeasure(lbMeasure const& other)=0;
    
    virtual void addLogOfQuotient(lbMeasure const& nominator,lbMeasure const& denominator) = 0;

    virtual void enableSharedParams(lbSharedParamMode mode)=0 ;
    
    virtual void addSharedParams(assignmentPtrVec const & vec,varsVec const& vars) =0;
    
    virtual void addIdleParams(assignmentPtrVec const & vec,varsVec const& var)=0;
    
    virtual void enableIdleParams()=0;

    virtual void subtractSumOfConditional(lbMeasure const & meas)=0;

    virtual void divideMeasureByAnother(lbMeasure const& otherM)=0;

    virtual void divideFromReplace(lbMeasure const& otherM)=0;

    virtual void multiplyMeasureByNumber(probType num) = 0;

    virtual void multiplyByConditionalSumOfMeasure(lbMeasure const& measure)=0;

    virtual void replaceValues(probType from, probType to) = 0;


    // sparsity

    virtual void setSparsityThresh(double sparseLevel) const = 0;

  protected:
    
    virtual void buildProbVec(int len,bool random = true)=0 ;

    virtual void buildProbVec(bool random = true) =0;

    virtual pair<cardVec,varsVec> buildSubSet(internalIndicesVec const& indices,
					      varsVec const& oldVec) const=0;
  };

  inline lbMultinomialMeasure::lbMultinomialMeasure() :
    lbMeasure() { }

  inline lbMultinomialMeasure::lbMultinomialMeasure(cardVec const& card) :
    lbMeasure(card) { }

  inline lbMultinomialMeasure::lbMultinomialMeasure(lbMultinomialMeasure const& oldMeasure):
    lbMeasure(oldMeasure) { } 

  inline lbMultinomialMeasure::lbMultinomialMeasure(lbMeasure const& oldMeasure):
    lbMeasure(oldMeasure) { } 

  inline lbMeasure& lbMultinomialMeasure::operator=(lbMeasure const& oldMeasure)
    {
      return operator=( (lbMultinomialMeasure const &)oldMeasure );
    }

  inline lbMultinomialMeasure& lbMultinomialMeasure::operator=(lbMultinomialMeasure const& oldMeasure)
    {
      lbMeasure::operator=(oldMeasure);
      return *this;
    }

  inline lbMultinomialMeasure::~lbMultinomialMeasure() {
  }

  inline probType lbMultinomialMeasure::operator() (lbBaseAssignment const& assign,
						  varsVec const& vars) const {
    return valueOfFull(assign,vars);
  }

  inline bool lbMultinomialMeasure::isDifferent(lbMeasure const& otherMeasure,
						const lbMessageCompareType type,
						probType epsilon) const{ 
    switch (type){
      case C_KL:
	return isDifferentKL(otherMeasure,epsilon);
	break;
      case C_MAX:
	return isDifferentMAX(otherMeasure,epsilon);
	break;
      case C_MAX_LOG:
	return isDifferentLogMAX(otherMeasure,epsilon);
        break;
      case C_AVG:
	return isDifferentAVG(otherMeasure,epsilon);
	break;
      case C_AVG_LOG:
        return isDifferentLogAVG(otherMeasure,epsilon);
        break;
      default:
	assert(false);
    }
	return false;
  }  

}; 
#endif

      
