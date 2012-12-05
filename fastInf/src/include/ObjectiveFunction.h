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

#ifndef __OBJECTIVE_FUNCTION_H
#define __OBJECTIVE_FUNCTION_H

#include <iostream>
#include <cassert>
#include <lbDefinitions.h>

using namespace std;

/** Represents a function to be optimized by providing methods to get entries
 * in the gradient, hessian, and the value of the function. Implements the
 * hessian method (ddf) to assert false, because it is very conceivable that
 * some optimizers would not care to implement the hessian.
 *
 * Also provides a method for setting the specified parameters of whatever
 * function one is optimizing.
 *
 * @author John Duchi
 * @author Varun Ganapathi
 * @date August 2006
 */
class ObjectiveFunction {

 protected:
  class Regularizer ;

 public:
  ObjectiveFunction() {
    _regularizer = NULL ;
  }

  virtual ~ObjectiveFunction() {
    if (_regularizer != NULL)
      delete _regularizer ;
  }

  /**
   * Calculates the value of the function at the specified point (or set
   * of parameters). Returns the value. */
  virtual double f (long double const* p) = 0;
  
  /** 
   * Calculates the gradient of the objective function based on
   * the current parameters p of the model. */
  virtual void df (long double const* p, long double* res) = 0;

  /**
   * Calculates both the gradient and objective of the function (i.e. its
   * value), which is the return result of the function. */
  virtual long double fdf (long double const* p, long double* res) { df(p,res); return f(p); };
    
  /**
   * Calculates the hessian (or at least some approximation of it) given
   * the current parameters p of the model.
   */
  virtual void ddf (long double const* p, long double** res) {
    cerr << "Hessian not implemented yet.\n"; assert(false);
  }
  
  /**
   * Returns the number of parameters for this objective function. */
  virtual int paramNum() const = 0;
  
  /**
   * Sets the parameter at the specified index to have the value
   * given. A note: GSLFuncWrapper ignores this method. */
  virtual void setParameter(int paramNum, long double value) { cerr << "Must implement set parameter if you want to use it!\n"; assert(false); };

  /**
   * Sets whether we are doing gradient based optimization, that is,
   * attempting to minimize the magnitude of the gradient. */
  static void setGradientBased(bool b) { GRADIENT_BASED = b; };

  /**
   * Returns true if we are doing gradient based optimization, that
   * is, that f should return the magnitude of the gradient, because
   * we are trying to minimize it. */
  static bool isGradientBased() { return GRADIENT_BASED; };

  static void setResortToPrevious(bool b) { RESORT_TO_PREV = b; };
  static bool resortToPrevious() { return RESORT_TO_PREV; };

  void setRegularizeParamL1(double beta) {
    if (_regularizer != NULL)
      delete _regularizer ;
    _regularizer = new ObjectiveFunction::L1Regularizer() ;
    _regularizer->setRegParam(beta) ;
  };
  void setRegularizeParamL2(double sigmaSq) { 
    if (_regularizer != NULL)
      delete _regularizer ;
    _regularizer = new ObjectiveFunction::L2Regularizer()  ;
    _regularizer->setRegParam(sigmaSq) ;
  };

  virtual double regPenalty(long double const* p, int size) const {
    if (regularize())
      return _regularizer->regPenalty (p,size) ;
    NOT_REACHED ; // should not reach here
    return 0.0 ;
  }
  virtual void dRegPenalty(long double const* p, int size, long double* res) const {
    if (regularize())
      _regularizer->dRegPenalty (p,size,res) ;
    else
      NOT_REACHED ; // should not reach here
  }

  static void setPosLogParams(bool b) { POS_LOG_PARAMS = b; };
  static bool PosLogParams() { return POS_LOG_PARAMS; };
      
  bool regularize() const { return (_regularizer!=NULL); } ;

 protected:
  static bool   GRADIENT_BASED;
  static bool   RESORT_TO_PREV;
  static bool   POS_LOG_PARAMS;

  Regularizer * _regularizer ;


  // Regularization:
 protected:

  // Regularization base class
  class Regularizer {

  public:
    Regularizer()
      : _regParam(0.0)
    {}

    virtual ~Regularizer() {}

    virtual double regPenalty(long double const* p, int size) const {
      return 0.0 ;  // no penalty by default
    }
    virtual void dRegPenalty(long double const* p, int size, long double* res) const {
      for ( int i=0 ; i<size ; i++ )
        res[i] = 0.0 ;
      // no penalty by default
    }

    virtual bool shouldRegularize() {
      return (_regParam != 0.0) ;
    }

    virtual void setRegParam (double param) {
      _regParam = param ;
    }

  protected:
    double _regParam ;
  }; // class Regularizer

  // L1 regularizer:
  class L1Regularizer : public Regularizer {
    virtual double regPenalty(long double const* p, int size) const;
    virtual void dRegPenalty(long double const* p, int size, long double* res) const;
  }; // class L1Regularizer

  // L2 regularizer:
  class L2Regularizer : public Regularizer {
    virtual double regPenalty(long double const* p, int size) const;
    virtual void dRegPenalty(long double const* p, int size, long double* res) const;
  }; // class L2Regularizer

};

#endif
