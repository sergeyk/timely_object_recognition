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

#ifndef __LB_MATH_UTILS
#define __LB_MATH_UTILS

#include <lbMathFunctions.h>
#include <vector>
using namespace std;

// compute average KL (over vars)given a vector of distributions for vars
inline long double
averageKL(vector< vector<long double> >* P,vector< vector<long double> >* Q)
{
  assert( P->size() == Q->size() );
  long double KL =0.0;
  uint vnum = P->size();
  for ( uint v=0 ; v<vnum ; v++ ) {
    uint num = (*P)[v].size();
    assert(num == (*Q)[v].size());
    for ( uint i=0 ; i<num ; i++ ) {
      long double p = (*P)[v][i];
      long double q = (*Q)[v][i];
      //      cerr << "p = " << p << " q = " << q << " + " << p * ( log2(p) - log2(q) ) << endl;
      if ( p != 0.0 ) 
	KL += p * ( log2(p) - log2(q) );
    }
  } // over vars
  KL /= vnum;
  return KL;
}

#endif
