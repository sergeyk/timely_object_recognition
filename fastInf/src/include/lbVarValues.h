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

#ifndef _Loopy__var_values_list

#define _Loopy__var_values_list



#include <lbDefinitions.h>
#include <lbUtils.h>

const int DEF_VEC_VAL = -1;



namespace lbLib {


  /*!
    MAP IMPLEMENTATION OF A VEC WITH DEFINED FUNCT
    An implementation of variable to value map using STL map
    \version July 2009
    \author Gal Elidan
  */

  class varValueMap : public map<int,varValue> {

  public:

    inline varValueMap() {};

    inline varValueMap(int size) {};

    inline varValueMap(varsVec const& vec) {};    

    inline const varValue operator[](int i) const {
      const_iterator it = find(i);
      if ( it != end() )
	return (const varValue)(it->second);
      else
	return DEF_VEC_VAL;
    };

    inline varValue& operator[](int i) {
      return map<int,varValue>::operator[](i);
    };

    inline varValue& GetValue(int i) {
      return map<int,varValue>::operator[](i);
    };

    
    inline bool isDefined(int i) const {
      return( operator[](i) != DEF_VEC_VAL );
    };

    inline bool isEmpty() const {
      return empty() ;
    };

    // reset does not actually erase the vars
    inline void reset() {
      iterator it;
      for ( it=begin() ; it!=end() ; it++ ) 
	it->second = DEF_VEC_VAL;
    };

    
    // reset does not actually erase the vars
    inline void zeroise(varsVec const& vars) {
      for ( uint i=0 ; i<vars.size() ; i++ )
	operator[](vars[i]) = 0;
    };
  };
  
  /*!
    VECTOR IMPLEMENTATION OF A VEC THAT WE EXPECT TO BE SMALL
    An implementation of variable to value map using a vector
    \version July 2009
    \author Gal Elidan
  */


  class varValueSmallVec  {

  public:

    inline varValueSmallVec() { assert(false); };

    inline varValueSmallVec(int size) { assert(false); }

    inline varValueSmallVec(varValueSmallVec const& other) {
      _size = other._size;
      _vars = new rVarIndex[_size];
      _vals = new varValue[_size];

      for (int i = 0; i < _size; i++) {
	_vars[i] = other._vars[i];
	_vals[i] = other._vals[i];
      }      
    }

    inline ~varValueSmallVec() {
      if ( _vars ) {
	delete [] _vars;
	delete [] _vals;
      }
    }
    
    inline bool operator==(varValueSmallVec const& vec) { 
      if ( _size != vec._size )
	return false;

      for ( int i=0 ; i<_size ; i++ ) {
	if ( _vars[i] != vec._vars[i] ||
	     _vals[i] != vec._vals[i] )
	  return false;
      }

      return true;
    };

    inline varValueSmallVec(varsVec const& vec,int defVal = 0) : _vars(NULL),_vals(NULL) {
      _size = vec.size();
      _vars = new rVarIndex[_size];
      _vals = new varValue[_size];
      for ( int i=0 ; i<_size ; i++ ) {
	_vars[i] = vec[i];
	_vals[i] = defVal;
      }
    };    
    

    inline bool isDefined(int i) const {
      return( operator[](i) != DEF_VEC_VAL );
    };

    inline bool isEmpty() const {
      return _size==0;
    }
    

    inline varValue& GetValue(int i) {
      return operator[](i);
    };

    
    inline const varValue operator[](int i) const {
      for ( int j=0 ; j <_size ; j++ ) 
	if ( _vars[j] == i ) 
	  return _vals[j];
      return DEF_VEC_VAL;
    };

    inline varValue& operator[](int i) {
      int j;
      for (j=0 ; j <_size ; j++ ) 
	if ( _vars[j] == i )
	  break;
      return _vals[j];;
    };


    inline void reset() {
      for ( int j=0 ; j <_size ; j++ ) 
	_vals[j] = DEF_VEC_VAL;
    };
    

    // reset does not actually erase the vars
    inline void zeroise(varsVec const& vars) {
      for ( uint i=0 ; i<vars.size() ; i++ )
	operator[](vars[i]) = 0;
    };

    inline uint size() const { return _size; };

  private:
    rVarIndex* _vars;
    varValue* _vals;
    int _size;
  };

  

  /*!
    VECTOR IMPLEMENTATION OF A VEC WITH DEFINED FUNCT
    An implementation of variable to value map using a vector
    \version July 2009
    \author Gal Elidan
  */

  class varValueVec : public safeVec<varValue> {
  public:
    inline varValueVec() {};

    inline varValueVec(int size) : safeVec<varValue>(size,DEF_VEC_VAL) {};

    inline varValueVec(varsVec const& vec) {
      int max = 0;
      for ( uint i=0 ; i<vec.size() ; i++ ) 
	if ( vec[i] > max )
	  max = vec[i];
      this->resize(max,DEF_VEC_VAL);
    };    
    
    inline bool isDefined(int i) const {
      return operator[](i) != DEF_VEC_VAL;
    };

    inline varValue& GetValue(int i) {
      return operator[](i);
    };

    inline void reset() {
      iterator it;
      for ( it=begin() ; it!=end() ; it++ ) 
	*it = DEF_VEC_VAL;
    };

    // reset does not actually erase the vars
    inline void zeroise(varsVec const& vars) {
      for ( uint i=0 ; i<vars.size() ; i++ )
	operator[](vars[i]) = 0;
    };
  };

  typedef varValueVec* varValueVec_ptr;

  /*!
  BASIC ASSIGNMENTS CLASS
  Basic interface we want from a class that holds evidence on some variables:
  \version July 2009
  \author Gal Elidan
  */

  class lbBaseAssignment {
  public:
    virtual ~lbBaseAssignment() {};

    virtual varValue getValueForVar(rVarIndex var) const = 0;

    virtual void setValueForVar(rVarIndex var,varValue value) = 0;

    virtual void UnsetValueForVar(rVarIndex var) = 0;

    virtual bool isAssigned(rVarIndex var) const = 0;

    virtual bool advanceOne(cardVec const& card,varsVec const& vars) = 0;
  };

  /*!
  BASIC ASSIGNMENTS CLASS
  The general idea is that we have different implementations of a map
  between variables and assignments and we use this template class
  to create various instanciations of them in lbAssignment for various needs
  \version July 2009
  \author Gal Elidan
  */
  template<class T> class lbVarValues : public lbBaseAssignment {
  public:
    inline lbVarValues() {};

    inline lbVarValues(uint size);

    inline lbVarValues(varsVec const& vars);

    inline lbVarValues(lbVarValues<T> const& otherList) ;

    inline lbVarValues& operator=(lbVarValues<T> const& otherList) ;

    inline bool operator==(lbVarValues<T> const& otherList);

    inline bool equals(lbVarValues<T> const& otherAssign, varsVec const& vars) const;

    inline bool matches(lbVarValues<T> const& otherAssign, varsVec const& vars) const;

    void print(ostream& out,int size) const;// = lbDefinitions::_numOfVars) const;

    inline virtual varValue getValueForVar(rVarIndex var) const;

    inline virtual void setValueForVar(rVarIndex var,varValue value);

    inline virtual void UnsetValueForVar(rVarIndex var);

    inline virtual bool isAssigned(rVarIndex var) const { return _valuesList.isDefined(var); };

    inline virtual bool advanceOne(cardVec const& card,varsVec const& vars);

    //    inline virtual varsVec getNotAssignedVars(varsVec const& origVars);
    
    inline uint getSize() const;

    inline void zeroise(varsVec const& vars) ;

    inline void zeroiseAll(int size) ;

    inline void reset();

    inline bool isEmpty() const { return _valuesList.isEmpty(); };

    bool readAssignmentFromFile(ifstream& in,int size);
    bool readAssignmentFromString(string assignStr,int size);

    inline bool areAssigned(varsVec const& vec) const {
      for (uint i = 0; i < vec.size(); i++)  {
	if (!isAssigned(vec[i]))
	  return false;
      }
      return true;
    };

    inline bool hasAnyAssigned(varsVec const& vec) const {
      for (uint i = 0; i < vec.size(); i++) {
        if (isAssigned(vec[i]))
          return true;
      }
      return false;
    }

  private:
    T _valuesList;
  };

  template<class T> inline uint lbVarValues<T>::getSize() const
  {
    return _valuesList.size();
  }

  template<class T> inline lbVarValues<T>::lbVarValues(uint size) :
    _valuesList(size)
  {
  }

  template<class T> inline lbVarValues<T>::lbVarValues(varsVec const& vars) :
    _valuesList(vars)
  {
  }
  
  template<class T> inline lbVarValues<T>::lbVarValues(lbVarValues<T> const& other) :
    _valuesList(other._valuesList)
  {
  };

  template<class T> inline varValue lbVarValues<T>::getValueForVar(rVarIndex var) const
  {
    return _valuesList[var];
  };

  template<class T> inline void lbVarValues<T>::setValueForVar(rVarIndex var,varValue value)
  {
    _valuesList[var]=value;
  };

  template<class T> inline void lbVarValues<T>::UnsetValueForVar(rVarIndex var)
  {
    _valuesList[var]=DEF_VEC_VAL;
  };

  template<class T> inline bool lbVarValues<T>::advanceOne(cardVec const& card,varsVec const& vars){
    int i = vars.size()-1;
    for ( ; i>=0 ; i--) {
      varValue& val = _valuesList.GetValue(vars[i]);
      if (val<(card[i]-1)) {
	val++;
	return true;
      }
      //assert( vars[i] < lbDefinitions::_numOfVars);
      val=0;
    }
    return false;
  }

  template<class T> inline lbVarValues<T>& lbVarValues<T>::operator=(lbVarValues<T> const& other) {
    _valuesList = other._valuesList;
    return *this;
  };

  template<class T> inline bool lbVarValues<T>::operator==(lbVarValues<T> const& other) {
    return _valuesList == other._valuesList;
  };

  template<class T> inline void lbVarValues<T>::zeroise(varsVec const& vars) {
    _valuesList.zeroise(vars);
  }

  template<class T> inline void lbVarValues<T>::zeroiseAll(int size) {
    for (int i=0; i<size; i++ )
      _valuesList[i] = 0;
  }

  
  template<class T> inline void lbVarValues<T>::reset() {
    _valuesList.reset();
  }
  /*
    template<class T> inline void lbVarValues<T>::getNotAssignedVars(varsVec const& origVars) {
    varsVec movingVars = varsVec();
    for (rVarIndex var=0;(uint)var<origVars.size();var++){
    if (_valuesList.isAssigned(origVars[var]))
    assign.setValueForVar(origVars[var],_evidence->getValueForVar(origVars[var]));
    else 
    movingVars.push_back(origVars[var]);
    }
    }
  */
  //this method is not simetric!!!
  //it checks for the vars of the other assignment, but returns false only if
  //they ARE assigned in our assignment AND the value of the assignment
  //is different.
  template<class T> inline bool lbVarValues<T>::matches(lbVarValues<T> const& otherAssign,varsVec const& otherVars) const {
    for ( uint i=0 ; i<otherVars.size() ; i++ ) {
      if ( isAssigned(otherVars[i]) ) {
	if ( !otherAssign.isAssigned(otherVars[i]) || getValueForVar(otherVars[i]) != otherAssign.getValueForVar(otherVars[i]) )
	  return false;
      }
    }
    //all important vars match
    return true;
  }

  template<class T> inline bool lbVarValues<T>::equals(lbVarValues<T> const& otherAssign,varsVec const& otherVars) const {
    for (uint i = 0; i < otherVars.size(); i++) {
      if (isAssigned(otherVars[i]) && !otherAssign.isAssigned(otherVars[i])) {
	return false;
      }
      else if (!isAssigned(otherVars[i]) && otherAssign.isAssigned(otherVars[i])) {
	return false;
      }
      else if (getValueForVar(otherVars[i]) != otherAssign.getValueForVar(otherVars[i])) {
	return false;
      }
    }

    return true;
  }

  template<class T> void lbVarValues<T>::print(ostream& out,int size) const
  {
    out<<"( ";
    for ( int i=0 ; i<size ; i++ ) {
      if ( isAssigned(i) ) 
	out<< getValueForVar(i) << " ";
      else
	out<<"? ";
    }
    out<<")"<<endl;
  }

  template<class T> bool lbVarValues<T>::readAssignmentFromFile(ifstream& in,int size){
    char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
    assert(!in.eof());
    in.getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
    string assignStr = string(buffer.get());
    return readAssignmentFromString(assignStr,size);
  }

  template<class T> bool lbVarValues<T>::readAssignmentFromString(string assignStr, int size){
    string notKnownStr="?";

    if (isVerbose(V_ASSIGNMENTS)) {
      cerr << "Reading Assign: " << assignStr << endl;
    }

    vector<string> tokens = lbUtils::tokenize(assignStr, " \n");

    if (tokens.size() == 0 || tokens[0] != "(" || tokens.back() != ")") {
      return false;
    }

    for (uint index = 1; index < tokens.size() - 1; index++) {
      if (tokens[index] == notKnownStr) {
  UnsetValueForVar(index-1);
      }
      else {
  setValueForVar(index-1 ,atoi(tokens[index].c_str()));
      }
    }
    
    if (isVerbose(V_ASSIGNMENTS)) {
      cerr << "Read Assign: ";
      print(cerr,size);
    }

    return true;
  }
  
};

#endif

