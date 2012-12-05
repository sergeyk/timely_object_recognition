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

#include <lbGradientAscent.h>
#include <iostream>
#include <Matrix.h>

using namespace std;
using namespace lbLib;

lbGradientAscent::lbGradientAscent( ObjectiveFunction& Opt )
  : Optimizer(Opt)
{}

long double
lbGradientAscent::Optimize (long double const* p, long double* res,
			    long double eps, long double step, int MaxIter) {
  return Optimize(p, res, eps, step, MaxIter, .75, 1e-3);
}

long double 
lbGradientAscent::Optimize( long double const *p, long double *res, 
			    long double eps, long double initStep, int MaxIter, 
			    double factor , double minStep )
{
  if (ObjectiveFunction::isGradientBased())
    return OptimizeGradOnly(p, res, eps, initStep, MaxIter, factor, minStep);
  else
    return OptimizeGradFunc(p, res, eps, initStep, MaxIter, factor, minStep);
}


long double 
lbGradientAscent::OptimizeGradFunc( long double const *p, long double *res, 
			    long double eps, long double initStep, int MaxIter, 
			    double factor , double minStep )
{

  // setup
  int n = _Func.paramNum();
  probType* bestRes = new probType[n];
  for ( int i=0 ; i<n ; i++ ) {
    res[i] = p[i];
    bestRes[i] = p[i];
  }
  probType* der = new probType[n]; 
  double step = initStep;
  probType oldLike = -1 * _Func.f(p);
  probType newLike = oldLike;
  probType bestLike = oldLike;
   
  cerr<<"*** Start of gradient ascent with LL = " << oldLike << " ***\n";

  // central look
  _iterationsTaken = 0;
  while ( true ) {

    oldLike = newLike;

    // get deriviative
    _Func.fdf(res,der);
    double derMax = 0.0;
    for ( int i=0 ; i<n ; i++ )
      if ( fabs(der[i]) > derMax )
	derMax = fabs(der[i]);

    // take the step
    for ( int i=0 ; i<n ; i++ ) {
        res[i] -= step * der[i]/derMax;
        if ( ObjectiveFunction::PosLogParams() && res[i]<0.0 )
            res[i] = 0.0;
    }

    newLike = -1 * _Func.f(res);

    if ( fabs(newLike-oldLike)/fabs(oldLike)<eps ) {
      cerr << "*** Converged! ***\n";
      break;
    }

    if ( newLike > bestLike ) {
      bestLike = newLike;
      for ( int i=0 ; i<n ; i++ )
	bestRes[i] = res[i];
    }

    cout << "GRADIENT: iter " << _iterationsTaken << "\t Step: " << step << "\tOld Like:  " 
	 << oldLike << "\tNew Like: " << newLike << endl;

    // determine next step size
    if (newLike < oldLike) {
      step = max(step*factor,minStep);
      if ( ObjectiveFunction::resortToPrevious() ) {
	for ( int i=0 ; i<n ; i++ )
	  res[i] = bestRes[i];
	newLike = bestLike;
      }
    }

    _iterationsTaken++;
    if ( _iterationsTaken>= MaxIter ) {
      cerr << "Did not converge.\n";
      break;
    }

  }

  if ( newLike < oldLike ) {
    for ( int i=0 ; i<n ; i++ )
      res[i] = bestRes[i];
    newLike = bestLike;
  }

  delete[] bestRes;
  delete[] der;
  cerr<<"*** End of gradient ascent after " << _iterationsTaken << " iterations with LL = " << newLike << " ***\n";
  return newLike;
}


long double 
lbGradientAscent::OptimizeGradOnly( long double const *p, long double *res, 
			    long double eps, long double initStep, int MaxIter, 
			    double factor , double minStep )
{

  // setup
  int n = _Func.paramNum();
  probType* oldRes = new probType[n];
  for ( int i=0 ; i<n ; i++ ) {
    res[i] = p[i];
  }  
  probType* der = new probType[n]; 
  probType* oldDer = new probType[n]; 
  double step = initStep;
  double derMag = DBL_MAX;
  double oldDerMag = DBL_MAX;
  
  bool firstStep = true;
  
  // central look
  _iterationsTaken = 0;
  while ( true ) {

    for (int i = 0; i < n; i++)
    {
	oldDer[i] = der[i];
    }
    oldDerMag = derMag;
	  
    // get deriviative
    _Func.df(res,der);
    
    //cerr << endl;
    // compute magnitude, max of derivative
    double derMax = 0.0;
    derMag = 0.0;
    for ( int i=0 ; i<n ; i++ ) {
      if ( fabs(der[i]) > derMax )
	derMax = fabs(der[i]);
      derMag += der[i]*der[i];
    }
    derMag = sqrt(derMag);

    if (!firstStep) {
      double dotProduct = 0.0;
      for ( int i=0 ; i<n ; i++ )
	dotProduct += der[i] * oldDer[i];
      
      // we stepped too far; decrease step size	
      if (dotProduct < 0) {
	bool alreadyMinStep = false;
	
	if (step == minStep)
	  alreadyMinStep = true;
	
	step = max(step*factor,minStep);
	
	cerr << "Dot product is " << dotProduct << " -- New mag is " << derMag<< " -- decreasing step size to " << step << endl;

	if (alreadyMinStep)
	  {
	    cerr << "*** Minimum step size already reached; can't make any more progress ***" << endl;
	    cerr << "Iterations: " << _iterationsTaken << endl;
	    break;
	  }
	
	// go back and redo the last step
	continue;
      } 
    } else {
      cerr<<"*** Start of gradient ascent with Mag = " << derMag << " ***\n";
      firstStep = false;
    }
    

    if (!(_iterationsTaken <  MaxIter )) {
      cerr << "*** Maximum iteration number MaxIter reached ***\n";
      break;
    }
    
    // if derivative mag is small enough, then stop
    if (derMag < eps) {
      cerr << "Coverged!\n";// with mag " << derMag << "***\n";
      break;
    }
    
    // take the step
    for ( int i=0 ; i<n ; i++ ) {
      oldRes[i] = res[i];
      res[i] -= step * der[i]/derMax;
    }
    
    cout << "GRADIENT: iter " << _iterationsTaken << "\t Step: " << step << "\tOld Mag:  " 
	 << oldDerMag << "\tNew Mag: " << derMag << endl;

    _iterationsTaken++;
  }


  delete[] der;
  delete[] oldDer;
  delete[] oldRes;
  
  cerr<<"*** End of gradient ascent after " << _iterationsTaken << " iterations with Mag = " << derMag << " ***\n";
  return derMag;
}
