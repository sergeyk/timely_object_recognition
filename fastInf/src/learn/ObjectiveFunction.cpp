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

#include <ObjectiveFunction.h>
#include <lbMathFunctions.h>


bool ObjectiveFunction::POS_LOG_PARAMS = false;
bool ObjectiveFunction::GRADIENT_BASED = false;
bool ObjectiveFunction::RESORT_TO_PREV = true;


double ObjectiveFunction::L1Regularizer::regPenalty(long double const* p, int size) const
{
  // Penalty term for L1:  1/2beta * sum_i |Theta_i|
  double sumAbs = 0.0;
  for ( int i=0 ; i<size ; i++ ) {
    sumAbs += abs(p[i]);
  }
  return 1/(2*_regParam) * sumAbs ;
}

void ObjectiveFunction::L1Regularizer::dRegPenalty(long double const* p,
                                                   int size,
                                                   long double* res) const
{
  // dL1/dTheta_j = 1/2beta * sign(Theta_j)
  for (int i=0 ; i<size ; ++i) {
    res[i] = 1/(2*_regParam) * sign(p[i]) ;
  }
}

double ObjectiveFunction::L2Regularizer::regPenalty(long double const* p, int size) const
{
  // Penalty term for L2:  -1/2sigma^2 sum_i Theta_i^2
  double norm = 0.0;
  for ( int i=0 ; i<size ; i++ )
    norm += square(p[i]);
  return 1/(2*_regParam) * norm ;
}

void ObjectiveFunction::L2Regularizer::dRegPenalty(long double const* p,
                                                   int size,
                                                   long double* res) const
{
  // dL2/dTheta_j = -1/2sigma^2 * Theta_j
  for (int i=0 ; i<size ; ++i) {
    res[i] = 1/(2*_regParam) * p[i] ;
  }
}

