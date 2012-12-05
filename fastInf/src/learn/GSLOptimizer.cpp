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
 *  GSLOptimizer.cpp
 *  ProteinInterface
 *
 *  Created by Nir Friedman on 3/23/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */
#include <ObjectiveFunction.h>
#include <assert.h>
#include <iostream>
using namespace std;

#include <GSLOptimizer.h>

double
GSLOptimizer_f( gsl_vector const *v, void *params)
{
	tGSLOptimizer *Opt = (tGSLOptimizer*) params;
	
	int n = Opt->_Func.paramNum();
	for( int i = 0; i < n; i++ )
		Opt->_Params[i] = gsl_vector_get (v, i);	
	
	return Opt->_Func.f( Opt->_Params );
}

void
GSLOptimizer_df( gsl_vector const *v, void *params, gsl_vector *df)
{
	tGSLOptimizer *Opt = (tGSLOptimizer*) params;
	
	int n = Opt->_Func.paramNum();
	int i;
	for( i = 0; i < n; i++ )
		Opt->_Params[i] = gsl_vector_get (v, i);	
	
	Opt->_Func.df( Opt->_Params, Opt->_Deriv );

	for( i = 0; i < n; i++ )
		gsl_vector_set (df, i, Opt->_Deriv[i]);	
}


void
GSLOptimizer_fdf( gsl_vector const *v, void *params, double *f, gsl_vector *df)
{
	tGSLOptimizer *Opt = (tGSLOptimizer*) params;
	
	int n = Opt->_Func.paramNum();
	int i;
	for( i = 0; i < n; i++ )
		Opt->_Params[i] = gsl_vector_get (v, i);	
	
	*f = Opt->_Func.fdf( Opt->_Params, Opt->_Deriv );
	
	for( i = 0; i < n; i++ )
		gsl_vector_set (df, i, Opt->_Deriv[i]);	
}

tGSLOptimizer::tGSLOptimizer (ObjectiveFunction& Opt, tProcType T )
: Optimizer(Opt), _ProcType( T )
{
	_GSL_func.f = GSLOptimizer_f;
	_GSL_func.df = GSLOptimizer_df;
	_GSL_func.fdf = GSLOptimizer_fdf;
	_GSL_func.n = _Func.paramNum();
	_GSL_func.params = this;
	
	_Params = new long double[ _Func.paramNum() ];
	_Deriv = new long double[ _Func.paramNum() ];
}

tGSLOptimizer::~tGSLOptimizer()
{
	delete [] _Params;
	delete [] _Deriv;
}

long double
tGSLOptimizer::Optimize( long double const *p, long double *res, long double eps, long double step, int MaxIter )
{
  _iterationsTaken = 0;
  int status;
	
  gsl_multimin_fdfminimizer_type const *T = NULL;
	
  switch( _ProcType )
  {
  case FR:
    T = gsl_multimin_fdfminimizer_conjugate_fr;
    //    cerr << "FR" << endl;
    break;
  case PR:
    T = gsl_multimin_fdfminimizer_conjugate_pr;
    //    cerr << "PR" << endl;
    break;
  case BFGS:
    T = gsl_multimin_fdfminimizer_vector_bfgs;
    //    cerr << "BFGS" << endl;
    break;
  case STEEP:
    T = gsl_multimin_fdfminimizer_steepest_descent;
    //    cerr << "SD" << endl;
    break;
  default:
    assert(false);
    break;

  }
	
  gsl_multimin_fdfminimizer *s;

  gsl_vector *x;
	
	
  /* Starting point, x = (5,7) */
  x = gsl_vector_alloc (_Func.paramNum());
	
  int n = _Func.paramNum();
  int i;
  for( i = 0; i < n; i++ )
    gsl_vector_set (x, i, p[i]);	

  s = gsl_multimin_fdfminimizer_alloc (T, n);
	
  gsl_multimin_fdfminimizer_set (s, &_GSL_func, x, step, eps);
	
  do
  {
    status = gsl_multimin_fdfminimizer_iterate (s);
		
    if (status)
      break;
		
    status = gsl_multimin_test_gradient (s->gradient, eps);		
    
    /*
    cerr << "GSLOpt " << _iterationsTaken;
    cerr << " [";
    for( int i = 0; i < n; i++ )
      cerr << " " << gsl_vector_get(s->x, i);
    cerr << " ] ";
    cerr << "F: " << s->f << endl;
    */

    _iterationsTaken++;
  }
  while (status == GSL_CONTINUE && _iterationsTaken < MaxIter);

  //  cerr << "GSL Optimizer: Finished after " << _iterationsTaken << " iterations (max=" << MaxIter << ")\n";

  for( i = 0; i < n; i++ )
    res[i] = gsl_vector_get (s->x, i);	

  long double retVal = gsl_multimin_fdfminimizer_minimum (s);
	
  gsl_multimin_fdfminimizer_free (s);
  gsl_vector_free (x);

  return retVal;
}
