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

/*
 *  lbNewtonOptimizer.h
 */
#ifndef __lbNewtonOptimizer_h__
#define __lbNewtonOptimizer_h__

#include <Optimizer.h>

class ObjectiveFunction;

namespace lbLib {

  class lbNewtonOptimizer : public Optimizer {
    
  public:
    lbNewtonOptimizer( ObjectiveFunction& Opt );
    
    virtual ~lbNewtonOptimizer() {};
    
    virtual long double Optimize (long double const *p, long double *res, 
				  long double eps = 0.0001, 
				  long double step = 0.01,
				  int MaxIter = 100);
  
    virtual int iterationsTaken() {
      return _iterationsTaken;
    }
  
  protected:
    int _iterationsTaken;
  };


  class lbNewtonDiagonalOptimizer : public Optimizer {
  public:
    lbNewtonDiagonalOptimizer (ObjectiveFunction& obj);
    virtual ~lbNewtonDiagonalOptimizer () {};
    
    virtual long double Optimize (long double const *p, long double *res, 
				  long double eps = 0.0001, 
				  long double step = 0.01,
				  int MaxIter = 100);
    virtual int iterationsTaken() {
      return _iterationsTaken;
    }
  protected:
    int _iterationsTaken;
  };

};

#endif

