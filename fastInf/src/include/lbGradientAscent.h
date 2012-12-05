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
 *  lbGradientAscent.h
 */
#ifndef __lbGradientAscent_h__
#define __lbGradientAscent_h__

#include <lbGSLLearningObject.h>

namespace lbLib {

  class lbGradientAscent : public Optimizer {
    
  public:
    lbGradientAscent( ObjectiveFunction& Opt );
    
    virtual ~lbGradientAscent() {};

    /** Simply calls Optimize with default arguments for factor and minStep of
     * .75 and 1e-3, respectively */
    virtual long double Optimize (long double const *p, long double *res, 
				  long double eps = 1e-5 , long double step = 1,
				  int MaxIter = 100);

    
    long double Optimize (long double const *p, long double *res, 
			  long double eps, long double step,
			  int MaxIter, double factor,
			  double minStep = 1e-3 );

    long double OptimizeGradOnly (long double const *p, long double *res,
				  long double eps = 1e-5, long double step = 1,
				  int MaxIter = 100, double factor = 0.75 ,
				  double minStep = 1e-3 );

    long double OptimizeGradFunc (long double const *p, long double *res,
				  long double eps = 1e-5 , long double step = 1,
				  int MaxIter = 100, double factor = 0.75 ,
				  double minStep = 1e-3);
    
    virtual int iterationsTaken() {
      return _iterationsTaken;
    }
    
  protected:
    int _iterationsTaken;
  };

};
#endif

