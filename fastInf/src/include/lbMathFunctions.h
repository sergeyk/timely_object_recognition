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

#ifndef __lbMathFunctions_h
#define __lbMathFunctions_h

#include <assert.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdlib.h>

using namespace std;

#ifdef _MSC_VER
#include <mathplus.h>
#endif

#ifndef exp2
inline long double exp2(long double x) 
{ 
  return exp(x*log(2.0));
}
#endif

#ifndef log2
inline long double log2(long double x) 
{
  return log(x)/log(2.0);
}
#endif

///
inline long double lChoose( long double k , long double n )
{
  if( k > 0 && k < n )
    return (lgamma( n + 1 ) - lgamma( k + 1 ) - lgamma( n - k + 1 ))/log(2.0);
  else
    return 0;
}

///
inline long double l2gamma(long double x )
{
  return lgamma(x) / log(2.0);
}

///
inline long double digamma(long double x)
{
  return ( log(x) - 1/(2*x) - 1/(12*pow(x,2)) +
	   1/(120*pow(x,4)) - 1/(252*pow(x,6)));
}

///
inline long double digamma1(long double x)
{
  return (1/x + 1/(2*pow(x,2) + 1/(6*pow(x,3)) -
	   1/(30*pow(x,5)) + 1/(42*pow(x,7)) ));
}

///
inline long double
GaussPDF(long double x, long double mu, long double sigma2 )
{
  long double z = (x -mu)/sqrt(sigma2);
  return 0.5*( 1 + erf(z / M_SQRT2 ));
}

///
inline long double
Gausspdf(long double x, long double mu, long double sigma2 )
{
  long double z = (x -mu);
  return 1/sqrt(2*M_PI*sigma2)*exp(-0.5*z*z/sigma2);
}

///
inline long double
lbAddLog(long double x, long double y )
{
  if (x == HUGE_VAL || y == HUGE_VAL) return HUGE_VAL; //log( e^HUGE_VAL + e^y ) = HUGE_VAL
  
  if( x == -HUGE_VAL ) return y;
  if( y == -HUGE_VAL ) return x;
  
  long double z = max(x,y);
  return z + log(exp(x-z) + exp(y-z));
}



///
inline long double
lbSubLog2(long double x, long double y )
{
  if ( x < y ) {
    cerr << "Can't represent negative numbers in log space\n";
    assert(false);
  }

  if ( y == -HUGE_VAL )
    return x;

  return x + log2(1.0 - exp2(y-x));
}

///
inline long double
lbSubLog(long double x, long double y )
{
  if ( x < y ) {
    cerr << "Can't represent negative numbers in log space\n";
    exit(1);
  }

  if ( y == -HUGE_VAL )
    return x;

  return x + log(1 - exp(y-x));
}


// If x = log a, y = log b, returns log |a-b|
// NOTE: THIS IS NO LONGER TRUE !! (If x = -HUGE_VAL, y > -HUGE_VAL [or vice versa], returns HUGE_VAL).
inline long double
lbAbsSubLog(long double x, long double y )
{
  if (x == -HUGE_VAL && y == -HUGE_VAL) {
    //log |e^-HUGE_VAL - e^-HUGE_VAL| = log |0-0| = log(0) = -HUGE_VAL
    return -HUGE_VAL;
  }
  else if (x == -HUGE_VAL || y == -HUGE_VAL)  {//handle special case, as noted above
    //return HUGE_VAL;
    if (x==-HUGE_VAL)
      return y;
    else 
      return x;
  }
  else if ( x >= y )
    return x + log(1 - exp(y-x));
  else
    return y + log(1 - exp(x-y));
}

// If x = log2 a, y = log2 b, returns log2 |a-b|
// NOTE: THIS IS NO LONGER TRUE !!NOTE: If x = -HUGE_VAL, y > -HUGE_VAL [or vice versa], returns HUGE_VAL.
inline long double
lbAbsSubLog2(long double x, long double y )
{
  return lbAbsSubLog(x,y)/log(2.0);
/*   if (x == -HUGE_VAL && y == -HUGE_VAL) { */
/*     //log2 |2^-HUGE_VAL - 2^-HUGE_VAL| = log2 |0-0| = log2(0) = -HUGE_VAL */
/*     return -HUGE_VAL; */
/*   } */
/*   else if (x == -HUGE_VAL || y == -HUGE_VAL) //handle special case, as noted above */
/*     return HUGE_VAL; */
/*   else if ( x >= y  ) */
/*     return x + log2(1 - exp2(y-x)); */
/*   else */
/*     return y + log2(1 - exp2(x-y)); */
}

///
inline long double
lbAddLog2(long double x, long double y )
{
  if (x == HUGE_VAL || y == HUGE_VAL) return HUGE_VAL; //log( 2^HUGE_VAL + 2^y ) = HUGE_VAL
  
  if( x == -HUGE_VAL ) return y;
  if( y == -HUGE_VAL ) return x;

  long double z;
  if( x > y )
    z = y - x;
  else
  {
    z = x - y;
    x = y;
  }
  return x + (long double)log2(1.0 + exp2(z));
}

inline long double Min (long double x, long double y) {
  if (x<y) { return x ; }
  else { return y ; }
}

inline long double Max (long double x, long double y) {
  if (x>y) { return x ; }
  else { return y ; }
}

inline long double sign (long double x) {
  if (x>0) {
    return 1 ;
  } else if (x<0) {
    return -1 ;
  } else {
    return 0 ;
  }
}

inline long double square (long double x) {
  return x*x ;
}


#define EPS 0.000000001
#define UNDEF_VAL    -1000000000
#define UNDEF_PROB   1.0

#endif
