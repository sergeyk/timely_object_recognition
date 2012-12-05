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

#ifndef _Loopy__Assigned__Measure
#define _Loopy__Assigned__Measure

#include <lbMeasure.h>
#include <lbMeasureDispatcher.h>
#include <cofw_ptr.hpp>
#include <assert.h>

namespace lbLib {

  class 				lbAssignedMeasure;
  typedef lbAssignedMeasure* 		lbAssignedMeasure_ptr;
  typedef safeVec< lbAssignedMeasure_ptr > assignedMesVec  ;
  typedef assignedMesVec::iterator 	assignedMesVecIter;
  typedef assignedMesVec lbAssignedMeasurePtrVec;

  typedef cofw_ptr<lbAssignedMeasure> lbAssignedMeasureCofwPtr;
  typedef safeVec<lbAssignedMeasureCofwPtr> lbAssignedMeasureCofwPtrVec;

  /*!
     This Class holds the assigned measure for the graph

     This Object holds the probabilistic characters of a clique.
     and performs simple actions on it
    
     This class is essentially a bridge to the lbMeasure interface.
     That is, we don't actually want to implement any operators here.
     We just want to store the variables so that we can give them to
     the lbMeasure class when it's time to do operations on them.

     Part of the loopy belief library
     \version July 2009
     \author Ariel Jaimovich
  */
  class lbAssignedMeasure {
    
  public:
    
    inline lbAssignedMeasure(lbMeasure_Sptr measure,varsVec const& vars) ;
    
    inline explicit lbAssignedMeasure(lbAssignedMeasure const& oldMeasure) ;
    
    inline ~lbAssignedMeasure();

    inline lbAssignedMeasure_ptr duplicate() const;

    inline lbAssignedMeasure_ptr duplicateValues() const;

    inline lbAssignedMeasure_ptr clone() const {
      return this->duplicateValues();
    }

    inline lbMeasure const& getMeasure() const;
    
    inline void normalize();
    
    inline probType totalWeight() ;
    
    inline probType valueOfFull(lbBaseAssignment const& assign) const;

    inline probType logValueOfFull(lbBaseAssignment const& assign) const;

    inline void setValueOfFull(lbBaseAssignment const& assign,probType value) ;

    inline void setLogValueOfFull(lbBaseAssignment const& assign,probType value) ;
    
    inline void divideMeasureByAnother(lbAssignedMeasure const& secondMeasure);

    inline void divideFromReplace(lbAssignedMeasure const& secondMeasure);

    inline void multiplyMeasureByAnother(lbAssignedMeasure const& secondMeasure);

    inline lbAssignedMeasure_ptr combineMeasures(lbAssignedMeasure const& secondMeasure,
					  lbMeasureDispatcher const& disp,
					  CombineType ct) const;
    
    bool raiseToThePower(probType power);

    inline bool operator == (lbAssignedMeasure const& otherMeasure) ;
    
    static lbAssignedMeasure_ptr readOneMeasure(ifstream_ptr in, lbMeasureDispatcher const& disp, bool readLogValues = false);
    
    // todo.cleanup: Move the implementations to lbTableMeasure.
    inline probType getH() const;

    inline probType getKL(lbAssignedMeasure const& other) const;

    inline probType getJS(lbAssignedMeasure const& other) const;

    inline probType getL1(lbAssignedMeasure const& other) const;

    inline probType getL2(lbAssignedMeasure const& other) const;

    inline probType getMaxDiff(lbAssignedMeasure const& other) const;
    
    inline bool zeroesAreOnto(lbAssignedMeasure const& other, bool printNonmatching = true) const;

    inline void print(ostream& out) const;
    
    inline varsVec const& getVars() const;

    inline bool updateAssign(lbAssignment const& given,lbAssignedMeasure const& original) ;

    inline bool updateAssign(lbAssignment const& given) ;

    inline void replaceValues(probType from, probType to);

    inline bool isDifferent(lbAssignedMeasure const& other,const lbMessageCompareType type,probType epsilon) const;
    
    inline void cleanMeasure();
    
    inline void updateMeasureValues(lbMeasure const& other,probType smoothParam,
				    probType epsilon = -1);

    inline void replaceMeasure(lbMeasure_Sptr measure);

    inline void replaceVars(varsVec const& vars);

    inline void makeRandom();

    inline void makeZeroes();

    inline void makeUniform();

    inline void makeDirected();

    inline void makeUndirected();
        
    inline varValue randomValFromTable();
    
    inline bool isDirected() const;


    // todo.cleanup: Can we get rid of some of these?

    inline bool marginalize(lbAssignedMeasure& 	newMeasure,
			    varsVec const& newVec) const;

    inline bool marginalize(lbAssignedMeasure& 	newMeasure,
			    varsVec const& newVec,
			    lbBaseAssignment const& assign) const;
    
    inline bool marginalizeAndMultiply(lbAssignedMeasure& newMeasure) const;
    
    inline bool marginalizeAndMultiply(lbAssignedMeasure& newMeasure,
				       varsVec const& newVec) const;
    
    inline lbAssignedMeasure_ptr marginalize(varsVec const& newVec,
					     lbMeasureDispatcher const& disp)  const;

    inline lbAssignedMeasure_ptr marginalize(varsVec const& newVec,
					     lbBaseAssignment const& assign,
					     lbMeasureDispatcher const& disp)  const;

    inline lbAssignment_ptr getMAPassign() const ;
    
  private:
    varsVec 		_vars;
    lbMeasure_Sptr 	_measure;
  };
  

  inline lbAssignedMeasure::lbAssignedMeasure(lbMeasure_Sptr measure,varsVec const& vars) :
    _vars(vars),
    _measure(measure){
      if (_measure->getCards().size()!=0) 
	assert(vars.size()==_measure->getCards().size());
  }
  
  inline void lbAssignedMeasure::replaceMeasure(lbMeasure_Sptr measure) {
    _measure = measure;
    if (_measure->getCards().size()!=0) 
      assert(_vars.size()==_measure->getCards().size());
  }

  inline void lbAssignedMeasure::replaceVars(varsVec const& vars) {
    _vars = vars;
  }
  
  inline lbAssignedMeasure::lbAssignedMeasure(lbAssignedMeasure const& oldMeasure) :
    _vars(oldMeasure._vars),
    _measure(oldMeasure._measure){
  }
 
  inline lbAssignedMeasure::~lbAssignedMeasure() {
    // Nothing to do
  }

  inline lbAssignedMeasure_ptr lbAssignedMeasure::duplicate() const{
    lbAssignedMeasure_ptr res = new lbAssignedMeasure(_measure->duplicate(),_vars);
    return res;
  }

  inline lbAssignedMeasure_ptr lbAssignedMeasure::duplicateValues() const{
    lbAssignedMeasure_ptr res = new lbAssignedMeasure(_measure->duplicateValues(),_vars);
    return res;
  }

  inline void lbAssignedMeasure::normalize() {
    _measure->normalize();
  }

  inline probType lbAssignedMeasure::totalWeight() {
    return _measure->totalWeight();
  }

  //full instantiation
  inline probType lbAssignedMeasure::valueOfFull(lbBaseAssignment const& assign) const {  
    return _measure->valueOfFull(assign,_vars);
  }

  inline probType lbAssignedMeasure::logValueOfFull(lbBaseAssignment const& assign) const {
    return _measure->logValueOfFull(assign,_vars);
  }
 
  inline void lbAssignedMeasure::setValueOfFull(lbBaseAssignment const& assign,probType value) {
    _measure->setValueOfFull(assign,_vars,value);
  }

  inline void lbAssignedMeasure::setLogValueOfFull(lbBaseAssignment const& assign,probType value) {
    _measure->setLogValueOfFull(assign,_vars,value);
  }
 

  inline void lbAssignedMeasure::divideMeasureByAnother(lbAssignedMeasure const& secondMeasure) {
    assert(secondMeasure.getVars() == getVars());
    _measure->divideMeasureByAnother(*secondMeasure._measure);
  }

  inline void lbAssignedMeasure::divideFromReplace(lbAssignedMeasure const& secondMeasure) {
    assert(secondMeasure.getVars() == getVars());
    _measure->divideFromReplace(*secondMeasure._measure);
  }


//multiply this one by another measure.
  inline lbAssignedMeasure_ptr lbAssignedMeasure::combineMeasures(lbAssignedMeasure const& secondMeasure,
								  lbMeasureDispatcher const& disp,
								  CombineType ct) const {
    varsVec const& leftVars = this->_vars;
    varsVec const& rightVars = secondMeasure._vars;
    varsVec fullVars = vecUnion(leftVars, rightVars);
    
    lbMeasure_Sptr mesPtr = disp.getNewMeasure();
    lbAssignedMeasure_ptr result(new lbAssignedMeasure(mesPtr,fullVars));
    
    _measure->combineMeasures(*result->_measure, secondMeasure.getMeasure(),
			      leftVars, rightVars, ct);
    
    
    return result;
  }


  inline bool lbAssignedMeasure::operator == (lbAssignedMeasure const& otherMeasure) {
    return ((_measure) == ((otherMeasure._measure)));
  }

  inline lbAssignedMeasure_ptr lbAssignedMeasure::marginalize(varsVec const& newVec,
							      lbMeasureDispatcher const& disp) const {

    if (newVec == _vars) {
      // Important that these be EXACTLY equal
      return (new lbAssignedMeasure(_measure->duplicate(), _vars));
    }
    else {	    
      lbMeasure_Sptr mesPtr = disp.getNewMeasure();
      lbAssignedMeasure_ptr result(new lbAssignedMeasure(mesPtr,newVec));
      marginalize(*result,newVec);
      return result;
    }
  }

  inline lbAssignedMeasure_ptr lbAssignedMeasure::marginalize(varsVec const& newVec,
							      lbBaseAssignment const& assign,
							      lbMeasureDispatcher const& disp)  const {
    lbMeasure_Sptr mesPtr = disp.getNewMeasure();
    lbAssignedMeasure_ptr result(new lbAssignedMeasure(mesPtr,newVec));
    marginalize(*result,newVec,assign);
    return result;
  }
 
  inline varsVec const& lbAssignedMeasure::getVars() const{
    return _vars;
  }

  inline void lbAssignedMeasure::print(ostream& out) const {
    _measure->print(out);
    out<<"VARS:";
    printVector(_vars,out);
  }
 
  inline probType lbAssignedMeasure::getH() const {
    lbSmallAssignment assign(_vars);
    probType H = 0.0;

    do {
      probType probP;
      probP = _measure->valueOfFull(assign, _vars);

      if (probP != 0.0) {
	H += probP*log2(1/probP);
      }

    } while(assign.advanceOne(_measure->getCards(), _vars));

    return H;
  }

  inline probType lbAssignedMeasure::getKL(lbAssignedMeasure const& other) const {
    lbSmallAssignment assign(_vars);
    probType kl =0.0;

    do {      
      probType probP, probQ;
      probP = _measure->valueOfFull(assign, _vars);
      probQ = other.getMeasure().valueOfFull(assign, other.getVars());

      if (probP != 0.0) {
	kl += probP*(log2(probP)-log2(probQ));
      }

    } while(assign.advanceOne(_measure->getCards(), _vars));

    return kl;
  }

  inline probType lbAssignedMeasure::getJS(lbAssignedMeasure const& other) const {
    lbSmallAssignment assign(_vars);

    lbAssignedMeasure_ptr avg = other.duplicate();
    avg->updateMeasureValues(getMeasure(), .5, lbDefinitions::EPSILON);
    probType js = .5*(getKL(*avg) + other.getKL(*avg));

    delete avg;
    return js;
  }

  inline probType lbAssignedMeasure::getL1(lbAssignedMeasure const& other) const {
    lbSmallAssignment assign(_vars);
    probType L1 =0.0;

    do {
      probType probP, probQ;
      probP = _measure->valueOfFull(assign, _vars);
      probQ = other.getMeasure().valueOfFull(assign, other.getVars());
      L1 += abs(probP-probQ);

    } while(assign.advanceOne(_measure->getCards(), _vars));

    return L1;
  }

  inline probType lbAssignedMeasure::getL2(lbAssignedMeasure const& other) const {
    lbSmallAssignment assign(_vars);
    probType L2 = 0.0;

    do {
      probType probP, probQ;
      probP = _measure->valueOfFull(assign, _vars);
      probQ = other.getMeasure().valueOfFull(assign, other.getVars());
      L2 += (probP-probQ)*(probP-probQ);
    } while(assign.advanceOne(_measure->getCards(), _vars));

    return L2;
  }

  inline probType lbAssignedMeasure::getMaxDiff(lbAssignedMeasure const& other) const {
    lbSmallAssignment assign(_vars);
    probType maxDiff = 0.0;

    do {
      probType probP, probQ;
      probP = _measure->valueOfFull(assign, _vars);
      probQ = other.getMeasure().valueOfFull(assign, other.getVars());
      probType diff = abs(probP-probQ);

      if (diff > maxDiff) {
	maxDiff = diff;
      }

    } while(assign.advanceOne(_measure->getCards(), _vars));

    return maxDiff;
  }

  inline bool lbAssignedMeasure::zeroesAreOnto(lbAssignedMeasure const& other, bool printNonmatching) const {
    lbSmallAssignment assign(_vars);
    bool match = true;

    if (printNonmatching) {
      cerr << "Testing assigned measure with vars: ";
      printVector(_vars, cerr);
    }

    do {
      probType probP, probQ;
      probP = _measure->valueOfFull(assign, _vars);
      probQ = other.getMeasure().valueOfFull(assign, other.getVars());
      
      if (probP == 0 && probQ != 0) {
	match = false;      
	
	if (printNonmatching) {
	  cerr << "Nonmatching Table Assignement: ( ";
	  for (uint i = 0; i < _vars.size(); i++) {
	    cerr << assign.getValueForVar(_vars[i]) << " ";
	  }
	  cerr << ")" << endl;
	}
      }
      else {
	cerr << ".";
      }

    } while(assign.advanceOne(_measure->getCards(), _vars));

    if (printNonmatching) {
      cerr << endl;
    }

    return match;
  }

  inline bool lbAssignedMeasure::updateAssign(lbAssignment const& given,
					      lbAssignedMeasure const& original) {
    if (_vars.size()>0)
      return _measure->updateAssign(given,_vars,*original._measure);
    return false;
    
  }
  
  inline bool lbAssignedMeasure::updateAssign(lbAssignment const& given){
    if (_vars.size()>0)
      return _measure->updateAssign(given,_vars);
    return false;
    
  }

  inline void lbAssignedMeasure::replaceValues(probType from, probType to) {
    _measure->replaceValues(from, to);
  }
 
  inline bool lbAssignedMeasure::isDifferent(lbAssignedMeasure const& other,
					     const lbMessageCompareType type,probType epsilon) const {
    assert(other._vars == _vars);
    return _measure->isDifferent(*other._measure,type,epsilon);
  }

  inline void lbAssignedMeasure::cleanMeasure(){
    NOT_REACHED;
  }
 
  inline void lbAssignedMeasure::updateMeasureValues(lbMeasure const& other,probType smoothParam,
						     probType epsilon) {
    _measure->updateMeasureValues(other,smoothParam,epsilon);
  }
 
  inline lbMeasure const& lbAssignedMeasure::getMeasure() const {
    return *_measure;
  }
 
  inline void lbAssignedMeasure::makeUniform() {
    _measure->makeUniform();
  }


  inline bool lbAssignedMeasure::marginalize(lbAssignedMeasure&	newMeasure,
					     varsVec const& newVec) const {
    bool res = _measure->marginalize((*newMeasure._measure), newVec, _vars);
    return res;
  }

  inline bool lbAssignedMeasure::marginalize(lbAssignedMeasure&	newMeasure,
					     varsVec const& newVec,
					     lbBaseAssignment const& assign) const {
    bool res = _measure->marginalizeWithAssign((*newMeasure._measure),newVec,_vars,assign);
    return res;
  }

  inline bool lbAssignedMeasure::marginalizeAndMultiply(lbAssignedMeasure& newMeasure,
							varsVec const& newVec) const {
    bool res = _measure->marginalize((*newMeasure._measure),newVec, _vars, true);
    return res;
  }

  inline void lbAssignedMeasure::makeRandom() {
    _measure->makeRandom();
  }

  inline void lbAssignedMeasure::makeZeroes() {
    _measure->makeZeroes();
  }

  inline varValue lbAssignedMeasure::randomValFromTable() {
    return _measure->randomValFromTable();
  }

  inline bool lbAssignedMeasure::isDirected() const{
    return _measure->isDirected();
  };

  inline void lbAssignedMeasure::makeDirected(){
    _measure->makeDirected();
  }

  inline void lbAssignedMeasure::makeUndirected() {
    _measure->makeUndirected();
  }

  inline lbAssignment_ptr lbAssignedMeasure::getMAPassign() const {
    return _measure->getMAPassign(_vars) ;
  }

  inline void lbAssignedMeasure::multiplyMeasureByAnother(lbAssignedMeasure const & secondMeasure) {
    assert(secondMeasure.getVars() == getVars());
    _measure->multiplyMeasureByAnother(secondMeasure.getMeasure());
  }

};

#endif

