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

#include <cmath>
#include <lbRandomProb.h>
#include <lbDefinitions.h>
#include <stdlib.h>
using namespace std;
#ifdef _MSC_VER 
#include <mathplus.h>
#endif

lbRandomGenerator _lbRandomProbGenerator;

unsigned long
lbMarsagliaGenerator::Next()
{
  register unsigned long hold;
  if (--i==0) i=43;
  if (--j==0) j=43;
  hold = word1[i]+carry;
  if (word1[j] < hold)
    {
      word1[i] = 4294967291u - (hold-word1[j]);
      carry = 1;
    }
  else
    {
      word1[i] = word1[j]-hold;
      carry=0;
    }
  weyl-=362436069u;
  return word1[i] - weyl;
}

lbMarsagliaGenerator::lbMarsagliaGenerator(unsigned query)
{
  unsigned start=1;  // default

  if (query)
    {
      cout << "Enter random number seed, 0<seed<65000." << endl;
      cin >> start;
    }

  srand(start);

  for (j=1;j<=43;j++)        		// make initial numbers.
    {
      word1[j]=rand();
      word1[j]=((word1[j] << 15 ) + rand());
      word1[j]=((word1[j] << 2 ) + rand()%4);
      if (word1[j]>4294967291u) word1[j] = 4294967291u;
    }

  // initialize markers
  i=44;  j=23;  carry=1; weyl=rand();

  for (unsigned long a=1,garbage; a<100000; a++)       	// Warm-up
    garbage = Next();
}

void
lbMarsagliaGenerator::Initialize(unsigned start)
{
  srand(start);

  for (j=1;j<=43;j++)        		// make initial numbers.
    {
      word1[j]=rand();
      word1[j]=((word1[j] << 15 ) + rand());
      word1[j]=((word1[j] << 2 ) + rand()%4);
      if (word1[j]>4294967291u) word1[j] = 4294967291u;
    }

  i=44;  j=23;  carry=1; weyl=rand();				// initialize markers

  for (unsigned long a=1,garbage; a<100000; a++)       	// Warm-up
    garbage = Next();
}

long double lbMarsagliaGenerator::RandomDouble(long double range)
{
  return ((((long double)Next())/4294967295u)*range);
}

#ifdef USE_GSL_RAND
lbRandomGenerator::lbRandomGenerator (unsigned long int seed) {
  // initialize
  _rng_ = gsl_rng_alloc (gsl_rng_ranlxd2) ;  //ranlxs2
  Initialize (seed) ;
}

void lbRandomGenerator::Initialize (unsigned long int seed) {
  gsl_rng_set (_rng_, seed) ;
}
    
lbRandomGenerator::~lbRandomGenerator() {
  // cleanup
  gsl_rng_free (_rng_) ;
}

long double lbRandomGenerator::RandomDouble(long double range)
{
  return (gsl_rng_uniform(_rng_) * range) ;
}

unsigned long int lbRandomGenerator::RandomInt(unsigned long int range)
{
  return (gsl_rng_uniform_int(_rng_,range)) ;
}
#endif

long double
lbRandomGenerator::RandomProb(void)
{
  return RandomDouble(1.0);
}



// Code adopted from David Heckerman
//-----------------------------------------------------------
//	DblGammaGreaterThanOne(dblAlpha)
//
//	routine to generate a gamma random variable with unit scale and
//      alpha > 1
//	reference: Ripley, Stochastic Simulation, p.90 
//	Chang and Feast, Appl.Stat. (28) p.290
//-----------------------------------------------------------
long double lbRandomGenerator::DblGammaGreaterThanOne(long double dblAlpha)
{
  long double rgdbl[6];

  rgdbl[1] = dblAlpha - 1.0;
  rgdbl[2] = (dblAlpha - (1.0 / (6.0 * dblAlpha))) / rgdbl[1];
  rgdbl[3] = 2.0 / rgdbl[1];
  rgdbl[4] = rgdbl[3] + 2.0;
  rgdbl[5] = 1.0 / sqrt(dblAlpha);

  for (;;)
  {
    long double  dblRand1;
    long double  dblRand2;
    do
    {
      dblRand1 = RandomProb();
      dblRand2 = RandomProb();

      if (dblAlpha > 2.5)
	dblRand1 = dblRand2 + rgdbl[5] * (1.0 - 1.86 * dblRand1);

    } while (!(0.0 < dblRand1 && dblRand1 < 1.0));

    long double dblTemp = rgdbl[2] * dblRand2 / dblRand1;

    if (rgdbl[3] * dblRand1 + dblTemp + 1.0 / dblTemp <= rgdbl[4] ||
	rgdbl[3] * log(dblRand1) + dblTemp - log(dblTemp) < 1.0)
    {
      return dblTemp * rgdbl[1];
    }
  }
  assert(false);
  return 0.0;
} 


      
/* routine to generate a gamma random variable with unit scale and alpha
< 1

   reference: Ripley, Stochastic Simulation, p.88 */

long double
lbRandomGenerator::DblGammaLessThanOne(long double dblAlpha)
{
  long double dblTemp;

  const long double	dblexp = exp(1.0);

  for (;;)
  {
    long double dblRand0 = RandomProb();
    long double dblRand1 = RandomProb();
    if (dblRand0 <= (dblexp / (dblAlpha + dblexp))) 
    {
      dblTemp = pow(((dblAlpha + dblexp) * dblRand0) /
		    dblexp, 1.0 / dblAlpha);
      if (dblRand1 <= exp(-1.0 * dblTemp)) 
	return dblTemp;
    }
    else 
    {
      dblTemp = -1.0 * log((dblAlpha + dblexp) * (1.0 - dblRand0) /
			   (dblAlpha * dblexp)); 
      if (dblRand1 <= pow(dblTemp,dblAlpha - 1.0)) 
	return dblTemp;
    }				
  }
  assert(false);
  return 0.0;
}  /* DblGammaLessThanOne */

// Routine to generate a gamma random variable with unit scale (beta = 1)
long double
lbRandomGenerator::DblRanGamma(long double dblAlpha)
{
  assert(dblAlpha > 0.0);
  if( dblAlpha < 1.0 )
    return DblGammaLessThanOne(dblAlpha);
  else
    if( dblAlpha > 1.0 )
      return DblGammaGreaterThanOne(dblAlpha);

  return -log(RandomProb());
}  /* gamma */

//-------------------------------------------------------------------------
// SampleDirichlet expects an RGALPHA such that the Dirichlet's parameters
// are a vector of alphas such that
// p(\theta) = c * \prod_i  \theta_i^{\alpha_i - 1} 
//-------------------------------------------------------------------------

void
lbRandomGenerator::SampleDirichlet(const vector<long double>& mean,
				  long double precision,
				  vector<long double>& rgprob)
{
  assert(mean.size() == rgprob.size());
  long double dblSum = 0.0;

  uint i;
  
  for( i = 0; i < mean.size(); i++ )
    dblSum += (rgprob[i] = DblRanGamma(mean[i] * precision));

  for( i = 0; i < rgprob.size(); i++)
  {
    rgprob[i] = rgprob[i] / dblSum;
  }
} 

/*
void
lbRandomGenerator::SampleDirichlet(int n,
				  long double const * mean,
				  long double precision,
				  long double * rgprob)
{
  long double dblSum = 0.0;
  int i;
  
  for( i = 0; i < n; i++ )
    dblSum += (rgprob[i] = DblRanGamma(mean[i] * precision));

  for( i = 0; i < n; i++)
  {
    rgprob[i] = rgprob[i] / dblSum;
  }
}
*/ 

void
lbRandomGenerator::SampleDirichlet(int n,
				  long double const * mean,
				  long double precision,
				  long double * rgprob)
{
  long double dblSum = 0.0;
  int i;
  
  for( i = 0; i < n; i++ )
    dblSum += (rgprob[i] = DblRanGamma(mean[i] * precision));

  for( i = 0; i < n; i++)
  {
    rgprob[i] = rgprob[i] / dblSum;
  }
} 

long double
lbRandomGenerator::SampleUniform(void)
{
  return RandomDouble(1.0);
}



long double
lbRandomGenerator::SampleNormal(void)
{
#ifdef notdef  
  // Ratio of uniforms, Ripley, Stochastic Simulation, p. 82
  long double U, V;
  long double X;
  long double Z;
  long double c = sqrt(2/exp(1));
  do {
  retry:
    U = c*(SampleUniform()*2-1);
    V = c*(SampleUniform()*2-1);
    if( fabs(U) < EPS )
      goto retry;
    X = V/U;
    Z = .25*X*X;
    if( Z < 1 - U )
      break;
  } while( (Z > 0.259/U + 0.35 ) || Z > -log(U) );
  return X;
#endif

  // Box-Muller, Ripley p. 54
  
  long double theta = 2*M_PI*SampleUniform();
  long double R = sqrt(2*-log(SampleUniform()));
  return R * cos(theta);
}

long double lbRandomGenerator::SampleNormal(long double variance)
{
  if ( variance == 0.0 )
    return 0.0;
  else {
    long double x = SampleNormal();
    long double scale = pow((double)variance,0.5);
    return x*scale;
  }
};



