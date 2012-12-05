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

#include <lbNewtonOptimizer.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_permutation.h>
#include <iostream>
#include <Matrix.h>
#include <ObjectiveFunction.h>
#include <math.h>

using namespace std;
using namespace lbLib;

lbNewtonOptimizer::lbNewtonOptimizer (ObjectiveFunction& Opt)
  : Optimizer(Opt)
{}

long double 
lbNewtonOptimizer::Optimize( long double const *p, long double *res, 
			     long double eps, long double step, int MaxIter )
{
  double retVal = 0.0;
  int N = _Func.paramNum();
  int signum ;
  cerr << "Optimizing with " << N << " parameters\n";
  long double* deriv = new long double[N];
  long double* oldDeriv = new long double[N];
  long double** hessianPtr = NewMatrix(N,N); // used for calling ObjectiveFunction.ddf
  gsl_matrix * hessian = gsl_matrix_alloc(N,N) ;
  gsl_matrix * invhessian = gsl_matrix_alloc(N,N) ;
  gsl_permutation * perm = gsl_permutation_alloc(N);
  for ( int i=0 ; i<N ; i++ ) {
    oldDeriv[i] = HUGE_VAL;
    for ( int j=0 ; j<N ; j++ ) {
      hessianPtr[i][j] = 0.0 ;
      gsl_matrix_set (hessian, i, j, 0.0) ;
      gsl_matrix_set (invhessian, i, j, 0.0) ;
    }
  }
  long double* p_curr = new long double[N];
  for ( int i=0 ; i<N ; i++ )
    p_curr[i] = p[i];
  long double oldVal,newVal;
  
  // Newton Outer Loop
  _iterationsTaken = 0;
  bool firstStep = true;
  do { 
    cerr << "Iteration " << _iterationsTaken << " of Newton optimization with objective value of ";
    // Compute gradient
    oldVal = -_Func.fdf(p_curr,deriv);
    cerr << oldVal << endl;
    
    // Compute Hessian
    _Func.ddf(p, hessianPtr);
    // copy into gsl matrix
    for ( int i=0 ; i<N ; i++ ) {
      for ( int j=0 ; j<N ; j++ ) {
        gsl_matrix_set (hessian, i, j, hessianPtr[i][j]) ;
      }
    }
    
    /*
    cerr << "Hessian diagonals:\n";
    for ( int i=0 ; i<N ; i++ ) {
      cerr << " " << hessian[i][i];
    }
    cerr << endl;
    */
    
    gsl_linalg_LU_decomp (hessian, perm, &signum) ;
    gsl_linalg_LU_invert (hessian, perm, invhessian) ;
    
    /*
    cerr << "Inverse hessian bands:\n";
    for (int i = 0; i < N-1; ++i) {
      cerr << " " << invhessian[i][i+1];
    }
    cerr << endl;
    for (int i = 0; i < N; ++i) {
      cerr << " " << invhessian[i][i];
    }
    cerr << endl;
    for (int i = 1; i < N; ++i) {
      cerr << " " << invhessian[i-1][i];
    }
    cerr << endl;
    */
    
    // check if need to reduce step size
    if (!firstStep) {
      double dotProduct = 0;
      for (int i = 0; i < N; ++i)
	dotProduct += deriv[i] * oldDeriv[i];
      if (dotProduct < 0) {
	step = step *.75;
	cerr << "Dot product = " << dotProduct << ", decreasing step size to " << step << endl;
      }
    }
    firstStep = false;

    // take step
    // p_new = p - step_size * inv_H * grad
    long double* p_new = new long double[N];
    for ( int i=0 ; i<N ; i++ ) {
      p_new[i] = 0.0;
      for ( int k=0 ; k<N ; k++ ) 
	p_new[i] += gsl_matrix_get(invhessian,i,k) * deriv[k];
    }
    for ( int i=0 ; i<N ; i++ ) 
      p_new[i] = p[i] - step*p_new[i];
    newVal = -_Func.fdf(p_new,deriv);

    delete[] p_curr;
    p_curr = p_new;

    // check convergence
    long double der_magnitude = 0;
    for (int i = 0; i < N; ++i)
      der_magnitude += deriv[i]*deriv[i];
    if ( der_magnitude < eps )
      break;

    // update old deriv
    for (int i = 0; i < N; ++i)
      oldDeriv[i] = deriv[i];

    _iterationsTaken++;
    
  } while ( _iterationsTaken < MaxIter);

  // clean up
  delete[] deriv;
  gsl_matrix_free (hessian) ;
  gsl_matrix_free (invhessian) ;
  gsl_permutation_free (perm) ;
  res = p_curr;
  return retVal;
}

lbNewtonDiagonalOptimizer::lbNewtonDiagonalOptimizer(ObjectiveFunction& obj)
  : Optimizer(obj)
{}

long double
lbNewtonDiagonalOptimizer::Optimize(long double const *p, long double *res, 
				    long double eps, long double step, int MaxIter)
{
  double retVal = 0.0;
  int N = _Func.paramNum();
  bool firstStep = true;
  cerr << "Optimizing with " << N << " parameters\n";
  long double* deriv = new long double[N];
  long double* hessian = new long double[N];
  long double* invhessian = new long double[N];
  long double** full_hessian = NewMatrix(N, N);
  for ( int i=0 ; i<N ; i++ ) {
    hessian[i] = 0.0;
    invhessian[i] = 0.0;
  }
  long double* p_curr = new long double[N];
  long double* p_new = new long double[N];
  long double* oldDeriv = new long double[N];
  long double* temp;
  for ( int i=0 ; i<N ; i++ ) {
    p_curr[i] = p[i];
  }

  // Newton Outer Loop
  _iterationsTaken = 0;
  do { 
    //cerr << "Iteration " << _iterationsTaken << " of Newton optimization with objective value of ";
    // Compute gradient
    _Func.df(p_curr,deriv);
    // Compute Hessian
    _Func.ddf(p_curr, full_hessian);
    for (int i = 0; i < N; ++i) {
      hessian[i] = full_hessian[i][i];
      invhessian[i] = 1.0/hessian[i];
    }
    
    if (!firstStep) {
      double dotProduct = 0;
      for (int i = 0; i < N; ++i)
	dotProduct += deriv[i] * oldDeriv[i];
      if (dotProduct < 0) {
	step = step *.75;
	cerr << "Dot product < 0, decreasing step size to " << step << endl;
      }
    }
    firstStep = false;
    // take step
    // p_new = p - step_size * inv_H * grad
    for (int i = 0; i < N; ++i) {
      deriv[i] *= invhessian[i];
    }
    for (int i=0 ; i<N ; ++i) {
      //cerr << "Taking step in negative direction: " << deriv[i] << endl;
      p_new[i] = p_curr[i] - step * deriv[i];
    }
    long double der_magnitude = 0;
    for (int i = 0; i < N; ++i) {
      der_magnitude += deriv[i]*deriv[i];
    }
    // Swap stuff
    temp = p_curr;
    p_curr = p_new;
    p_new = temp;
    temp = oldDeriv;
    oldDeriv = deriv;
    deriv = temp;
    // check if changed
    if ( der_magnitude < eps) {
      break;
    } else {
      cerr << "Newton: iteration " << _iterationsTaken << ", Gradient magnitude: "
	   << der_magnitude << endl;
    }
    ++_iterationsTaken;
    
  } while ( _iterationsTaken < MaxIter);
  
  // clean up
  delete[] deriv;
  delete[] hessian;
  delete[] invhessian;
  for (int i = 0; i < N; ++i) {
    res[i] = p_curr[i];
  }
  delete[] p_curr;
  FreeMatrix(full_hessian, N);
  return retVal;
}

