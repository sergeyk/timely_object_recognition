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

#ifndef _LB_RANDOM_PROB_H
#define _LB_RANDOM_PROB_H

#define USE_GSL_RAND // uses GSL random generator
//#undef USE_GSL_RAND // uses old Maraglia generator code

// Marsaglia's subtractive R.N. generator with carry; combined with a weyl
// generator.
// Source: Computer Physics Communications 60 (1990) 345-349.
// Written by Geoffrey Zweig, 1992.
// IMPLEMENTATION FILE


#ifdef __GNUG__
#ifdef PRAGMA_TEMPLATES
#pragma interface
#endif
#endif


#include <vector>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <cmath>

#ifdef USE_GSL_RAND
#include <gsl/gsl_rng.h>
#endif
using namespace std;

///
class lbMarsagliaGenerator
{
public:
  ///
  lbMarsagliaGenerator(unsigned query=0);
  ///
  void Initialize(unsigned start);
  ///
  unsigned long RandomLong() {return Next();} 

  ///
  unsigned long RandomLong(unsigned long range)
  {return (Next() % range);}

  ///
  unsigned RandomInt(unsigned range) {return unsigned(Next() % range);} 

  ///
  long double RandomDouble(long double range);	

private:
  ///
  unsigned long word1[44];
  ///
  unsigned long weyl;
  ///
  int i,j,carry;
  ///
  unsigned long Next();
}///
;

#ifdef USE_GSL_RAND
class lbRandomGenerator {
public:
  ///
  lbRandomGenerator(unsigned long int seed = 0);
  ~lbRandomGenerator();

  void Initialize (unsigned long int seed = 0) ;
  long double RandomDouble(long double range);
  unsigned long int RandomInt(unsigned long int range);
#else
class lbRandomGenerator : public lbMarsagliaGenerator {
public:
  lbRandomGenerator(unsigned long int seed = 0)
  { Initialize(seed); };
#endif
      
  long double RandomProb();
  ///
  long double DblGammaGreaterThanOne(long double dblAlpha);
  ///
  long double DblGammaLessThanOne(long double dblAlpha);
  ///
  long double DblRanGamma(long double dblAlpha);

  ///
  void   SampleDirichlet(const vector<long double>& rgalpha,
			 vector<long double>& rgprob);
  
  
  ///
  void   SampleDirichlet(const vector<long double>& mean,
			 long double precision,
			 vector<long double>& rgprob );

  ///
  void   SampleDirichlet( int n,
			  long double const*alpha,
			  long double* prob );
  
  ///
  /*
  void   SampleDirichlet( int n,
			  long double const* mean,
			  long double        precision,
			  long double* prob );
  */

  ///
  void   SampleDirichlet( int n,
			  long double const* mean,
			  long double        precision,
			  long double* prob );

  ///
  long double SampleUniform(void);

  ///
  long double SampleNormal(void);
  long double SampleNormal(long double variance);
  
private:
  #ifdef USE_GSL_RAND
  gsl_rng * _rng_ ;
  #endif
}///
;

///
extern lbRandomGenerator _lbRandomProbGenerator;

///
inline
long double lbRandomProb(void)
{
  return _lbRandomProbGenerator.RandomProb();
}

///
inline
void
lbRandomGenerator::SampleDirichlet(const vector<long double>& rgalpha,
				  vector<long double>& rgprob)
{
  SampleDirichlet( rgalpha, 1.0, rgprob );
}

///
inline
void
lbRandomGenerator::SampleDirichlet( int n,
				   long double const* alpha,
				   long double * prob )
{
  SampleDirichlet( n, alpha, 1.0, prob );
}


#endif
