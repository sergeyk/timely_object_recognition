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

#ifndef _LB_VALUE__
#define _LB_VALUE__

#include <lbDefinitions.h>
#include <lbMathFunctions.h>
#include <float.h>

namespace lbLib {

  class lbLogValue;

  class lbValue {
  public:
    lbValue();

    ~lbValue() {};

    inline lbValue(lbLogValue const& value);
    inline lbValue const& operator=(lbLogValue const& value);

    inline bool operator<(lbValue const& value) const;
    inline bool operator>(lbValue const& value) const;
    inline bool operator<=(lbValue const& value) const;
    inline bool operator>=(lbValue const& value) const;
    inline bool operator==(lbValue const& value) const;
    inline bool operator!=(lbValue const& value) const;

    inline lbValue operator+(lbValue const& value) const;
    inline lbValue operator-(lbValue const& value) const;
    inline lbValue operator/(lbValue const& value) const;
    inline lbValue operator*(lbValue const& value) const;
    inline lbValue raiseToThePower(probType prob) const;
    inline lbValue takeLog() const ;
    inline lbValue takeExp() const ;

    /* NOTE: If one lbValue is 0 and the other is not,
       returns HUGE_VAL [and NOT the absolute value of the other lbValue]. */
    friend inline lbValue absDiff(lbValue const& value1,
				  lbValue const& value2);

    inline lbValue const& operator+=(lbValue const& value);
    inline lbValue const& operator-=(lbValue const& value);
    inline lbValue const& operator/=(lbValue const& value);
    inline lbValue const& operator*=(lbValue const& value);

    
    inline void setLogValue(probType prob);
    inline void setValue(probType prob);
    inline probType getLogValue() const;
    inline probType getLog2Value() const;
    inline probType getValue() const;
    
    inline void print(ostream & out) const;

    inline bool getLogRepresentation() const { return false; }
    
  private:
    probType _val;
  };

  inline bool lbValue::operator<(lbValue const& value) const {
    return _val < value._val;
  }

  inline bool lbValue::operator>(lbValue const& value) const {
    return _val > value._val;
  }

  inline bool lbValue::operator<=(lbValue const& value) const {
    return _val <= value._val;
  }

  inline bool lbValue::operator>=(lbValue const& value) const {
    return _val >= value._val;
  }

  inline bool lbValue::operator==(lbValue const& value) const {
    return _val == value._val;
  }

  inline bool lbValue::operator!=(lbValue const& value) const {
    return !operator==(value);
  }

  inline lbValue lbValue::operator+(lbValue const& value) const {
    lbValue p;
    p.setValue(_val + value._val);
      
    return p;
  }

  inline lbValue lbValue::operator-(lbValue const& value) const {
    lbValue p;
    p.setValue(_val - value._val);
     
    return p;
  }

  inline lbValue lbValue::operator/(lbValue const& value) const {
    lbValue p;
  
    if (value._val == 0) {
      cerr << "ERROR(2): Dividing lbValue by 0" << endl;
      NOT_REACHED;
    }
    
    p.setValue(_val / value._val);
  
    return p;
  }

  inline lbValue lbValue::operator*(lbValue const& value) const {
    lbValue p;
    p.setValue(_val * value._val);  
    return p;
  }

  inline lbValue lbValue::raiseToThePower(probType prob) const {
    lbValue p;
    p.setValue(pow(_val,prob));
    return p;
  }

  inline lbValue lbValue::takeLog() const {
    lbValue p;
    p.setValue(log(_val));
    return p;
  }

  inline lbValue lbValue::takeExp() const {
    lbValue p;
    p.setValue(exp(_val));
    return p;
  }

  inline lbValue absDiff(lbValue const& value1, lbValue const& value2) {
    lbValue d;
    d.setValue( ABS(value1._val - value2._val) );
    
    /* Depracated:
       NOTE: If one lbValue is 0 and the other is not,
       returns HUGE_VAL [and NOT the absolute value of the other lbValue]. 
    if ((value1._val == 0 && value2._val != 0) ||
	(value2._val == 0 && value1._val != 0)) {
      d.setValue(HUGE_VAL);
    }
    */
    return d;
  }

  inline lbValue const& lbValue::operator+=(lbValue const& value) {
    _val += value._val;
    return *this;
  }

  inline lbValue const& lbValue::operator-=(lbValue const& value) {
    _val -= value._val;
    return *this;
  }

  inline lbValue const& lbValue::operator/=(lbValue const& value) {
    if (value._val == 0) {
      cerr << "ERROR(4): Dividing lbValue by 0" << endl;
      NOT_REACHED;
    }
    
    _val /= value._val;
  
    return *this;
  }

  inline lbValue const& lbValue::operator*=(lbValue const& value) {
    _val *= value._val;
    return *this;
  }

  inline void lbValue::setLogValue(probType lp) {
    
    probType v;
    if ( isinf(lp) ) { //lp = +-inf
      if (lp < 0) //lp = -inf
	v = 0.0;
      else //lp = +inf
	v = HUGE_VAL;
    }
    else
      v = exp(lp);
    
    if (v > LDBL_MAX) {
      cerr << "WARNING: Got value " << v << " setting " << LDBL_MAX << endl;
      _val = LDBL_MAX;
    }
    else {
      _val = v;
    }
  }

  inline void lbValue::setValue(probType prob) {
    _val = prob;
  }
  
  inline probType lbValue::getLogValue() const {
    if (_val == 0) {
      return -HUGE_VAL;
    }
    else {
      return log(_val);
    }
  }

  inline probType lbValue::getLog2Value() const {
    if (_val == 0) {
      return -HUGE_VAL;
    }
    else {
      return log2(_val);
    }
  }

  inline probType lbValue::getValue() const {
    return _val;
  }

  inline void lbValue::print(ostream & out) const {
    out << "Value(" << getValue() << ") ";
    out << "Log-Value(" << getLogValue() << ") ";
    cerr << "Standard representation." << endl;
  }

  inline ostream& operator<<(ostream & out, lbValue const& value) {
    out << value.getValue();
    return out;
  }




  ////////////////////////
  // LOG REPRESENTATION //
  ////////////////////////

  class lbLogValue {
  public:
    lbLogValue();

    ~lbLogValue() {}

    inline lbLogValue(lbValue const& value);
    inline lbLogValue const& operator=(lbValue const& value);

    inline bool operator<(lbLogValue const& value) const;
    inline bool operator>(lbLogValue const& value) const;
    inline bool operator<=(lbLogValue const& value) const;
    inline bool operator>=(lbLogValue const& value) const;
    inline bool operator==(lbLogValue const& value) const;
    inline bool operator!=(lbLogValue const& value) const;

    inline lbLogValue operator+(lbLogValue const& value) const;
    inline lbLogValue operator-(lbLogValue const& value) const;
    inline lbLogValue operator/(lbLogValue const& value) const;
    inline lbLogValue operator*(lbLogValue const& value) const;
    inline lbLogValue raiseToThePower(probType prob) const;
    inline lbLogValue takeLog() const ;
    inline lbLogValue takeExp() const ;

    /* NOTE: If one lbLogValue is -HUGE_VAL and the other is not,
       returns HUGE_VAL [and NOT the other lbLogValue]. */
    friend inline lbLogValue absDiff(lbLogValue const& value1,
				     lbLogValue const& value2);

    inline lbLogValue const& operator+=(lbLogValue const& value);
    inline lbLogValue const& operator-=(lbLogValue const& value);
    inline lbLogValue const& operator/=(lbLogValue const& value);
    inline lbLogValue const& operator*=(lbLogValue const& value);

    inline void setLogValue(probType prob);
    inline void setValue(probType prob);
    inline probType getLogValue() const;
    inline probType getLog2Value() const;
    inline probType getValue() const;
   
    inline void print(ostream & out) const;

    inline bool getLogRepresentation() const { return true; }

  private:
    probType _val;
  };


  inline bool lbLogValue::operator<(lbLogValue const& value) const {
    return _val < value._val;
  }

  inline bool lbLogValue::operator>(lbLogValue const& value) const {
    return _val > value._val;
  }

  inline bool lbLogValue::operator<=(lbLogValue const& value) const {
    return _val <= value._val;
  }

  inline bool lbLogValue::operator>=(lbLogValue const& value) const {
    return _val >= value._val;
  }

  inline bool lbLogValue::operator==(lbLogValue const& value) const {
    return _val == value._val;
  }

  inline bool lbLogValue::operator!=(lbLogValue const& value) const {
    return !operator==(value);
  }

  inline lbLogValue lbLogValue::operator+(lbLogValue const& value) const {
    lbLogValue p;
    p.setLogValue(lbAddLog(_val, value._val));
    return p;
  }

  inline lbLogValue lbLogValue::operator-(lbLogValue const& value) const {
    lbLogValue p;
    if ( _val < value._val )
      cerr << "operator-: Subtraction yields negative number represented in logspace.\n";
    p.setLogValue(lbSubLog(_val, value._val));  
    return p;
  }

  inline lbLogValue lbLogValue::operator/(lbLogValue const& value) const {
    lbLogValue p;
  
    if (value._val <= -HUGE_VAL) {
      cerr << "ERROR(1): Dividing lbLogValue by 0" << endl;
      NOT_REACHED;
    }

    probType pLogVal = 0;
    if (_val != value._val)
      pLogVal = _val - value._val;
    
    p.setLogValue(pLogVal);
    
    return p;
  }

  inline lbLogValue lbLogValue::operator*(lbLogValue const& value) const {
    lbLogValue p;
    p.setLogValue(_val + value._val);
    return p;
  }

  inline lbLogValue lbLogValue::raiseToThePower(probType prob) const {
    lbLogValue p;
    p.setLogValue(_val*prob);
    return p;
  }

  inline lbLogValue lbLogValue::takeLog() const {
    lbLogValue p;
    p.setValue(_val);
    return p;
  }

  inline lbLogValue lbLogValue::takeExp() const {
    lbLogValue p;
    p.setValue(exp(getValue()));
    return p;
  }

  inline lbLogValue absDiff(lbLogValue const& value1, lbLogValue const& value2) {
    lbLogValue d;
    /* this comment is Depracated!
       NOTE: If one lbLogValue is -HUGE_VAL and the other is not,
       AbsSubLog returns HUGE_VAL [and NOT the other lbLogValue]. */
    
    //log |e^value1._val - e^value2._val|, except as noted above:
    d.setLogValue( lbAbsSubLog(value1._val, value2._val) );
    return d;
  }

  inline lbLogValue const& lbLogValue::operator+=(lbLogValue const& value) {
    _val = lbAddLog(_val, value._val);
    return *this;
  }

  inline lbLogValue const& lbLogValue::operator-=(lbLogValue const& value) {
    if (value._val > _val) {
      cerr << "operator -=: Subtraction yields negative number represented in logspace.\n";
    }
    _val = lbSubLog(_val, value._val);
    return *this;
  }

  inline lbLogValue const& lbLogValue::operator/=(lbLogValue const& value) {
    if (value._val <= -HUGE_VAL) {
      cerr << "ERROR(3): Dividing lbLogValue by 0" << endl;
      NOT_REACHED;
    }

    if (_val != value._val)
      _val -= value._val;
    else
      _val = 0;
    
    return *this;
  }

  inline lbLogValue const& lbLogValue::operator*=(lbLogValue const& value) {
    _val += value._val;
    return *this;
  }

  inline void lbLogValue::setLogValue(probType lp) {
    _val = lp;

    if ( isinf(_val) ) { //_val = +-inf
      if (_val < 0) //_val = -inf
	_val = -HUGE_VAL;
      else //_val = +inf
	_val = HUGE_VAL;
    }
  }

  inline void lbLogValue::setValue(probType prob) {
    if (prob == 0) {
      _val = -HUGE_VAL;
    }
    else {
      _val = log(prob);
    }
  }

  inline probType lbLogValue::getLogValue() const {
    return _val;
  }

  inline probType lbLogValue::getLog2Value() const {
    return _val/log(2.0);
  }

  inline probType lbLogValue::getValue() const {
    probType v;
    if ( _val == -HUGE_VAL )
      v = 0;
    else
      v = exp(_val);

    if (v > LDBL_MAX) {
      //cerr << "WARNING: Got value " << v << " getting " << LDBL_MAX << endl;
      v = LDBL_MAX;
      return v;
    }
    else {
      return v;
    }
  }

  inline void lbLogValue::print(ostream & out) const {
    out << "Value(" << getValue() << ") ";
    out << "Log-Value(" << getLogValue() << ") ";
    cerr << "Log representation." << endl;
  }

  inline ostream& operator<<(ostream & out, lbLogValue const& value) {
    out << value.getValue();
    return out;
  }

  inline lbLogValue::lbLogValue(lbValue const& value) :
    _val(value.getLogValue()) {};

  inline lbValue::lbValue(lbLogValue const& value) :
    _val(value.getValue()) {};

  inline lbLogValue const& lbLogValue::operator=(lbValue const& value) {
    _val = value.getLogValue();
    return *this;
  }

  inline lbValue const& lbValue::operator=(lbLogValue const& value) {
    _val = value.getValue();
    return *this;
  }

};

#endif
