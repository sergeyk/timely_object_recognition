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

#ifndef __OPTIMIZER_H
#define __OPTIMIZER_H

class ObjectiveFunction;

/** Abstract interface for a class that can optimize an ObjectiveFunction
 *  TODO:
 *  We must be clear about whether it minimizes or maximizes the function.
 *  This might be done with a boolean or something. 
 */
class Optimizer {
 public:
  Optimizer (ObjectiveFunction& objectiveFunction) : 
    _Func(objectiveFunction) {};
  
  virtual ~Optimizer(){}
  
  /** 
   * Optimizes the ObjectiveFunction with which this Optimizer was
   * constructed, starting from the point p, giving result in res, and
   * with precision eps. Also, the initial step size is given in step, and
   * the max iterations to take is in maxIter.
   * Note that the default arguments given here will apply to any
   * objects that are Optimizers if the compiler thinks that they are
   * optimizers (i.e. Optimizer o = new SpecialOptimizer(), then calling
   * o->Optimize() will use these default arguments).
   */
  virtual long double Optimize (long double const* p, long double *res,
				long double eps = 0.0001, long double step = 0.1,
				int maxIter = 100) = 0;
  
  /** Gives the ObjectiveFunction (see ObjectiveFunction.h) that is internal
   * to this Optimizer object. ObjectiveFunctions provide the ability to
   * change the parameters of the problem, etc. */
  ObjectiveFunction& getObjectiveFunction(){ return _Func;}
  
  virtual int iterationsTaken() = 0;
  
 protected:
  /**
   * The objective function this Optimizer object is trying to optimize. */
  ObjectiveFunction& _Func;
};

#endif
