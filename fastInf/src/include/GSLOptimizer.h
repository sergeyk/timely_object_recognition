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
 *  GSLOptimizer.h
 *
 *  Created by Nir Friedman on 3/23/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __GSLOptimizer_h
#define __GSLOptimizer_h

#include <gsl/gsl_multimin.h>
#include <Optimizer.h>

class ObjectiveFunction;

class tGSLOptimizer : public Optimizer {
 public:
  enum tProcType {
    FR,
    PR,
    BFGS,
    STEEP,
    SIMPLEX,
    NEWTON, // this is not really part of GSL but its convenient to put it here
    GRADIENT
  };
  
  /**
   * Creates a new tGSLOptimizer, which uses the GSL to do optimization of the
   * given objective function. Takes as a parameter the type of optimization to
   * use, defaulting to Fletcher-Reeves conjugate-gradient.
   */
  tGSLOptimizer( ObjectiveFunction& obj, tProcType T = FR );
  
  virtual ~tGSLOptimizer();
  
  long double Optimize (long double const *p, long double *res,
			long double eps = 0.0001, long double step = 0.01,
			int MaxIter = 100);
  
  friend double GSLOptimizer_f( gsl_vector const *v, void *params);
  friend void GSLOptimizer_df( gsl_vector const *v, void *params, gsl_vector *df);
  friend void GSLOptimizer_fdf( gsl_vector const *v, void *params,double* f, gsl_vector *df);
	
  virtual int iterationsTaken() {
    return _iterationsTaken;
  }

 protected:
  int _iterationsTaken;

 private:
  long double *_Params, *_Deriv;
  gsl_multimin_function_fdf _GSL_func;
  tProcType _ProcType;
};

#endif

