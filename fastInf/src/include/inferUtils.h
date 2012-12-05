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

#ifndef _INFER_UTILS_H_
#define _INFER_UTILS_H_

#include <lbDefinitions.h>
#include <lbRegionBP.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include "glpk.h"


namespace lbLib {

  typedef vector<double> doubleVec ;
  typedef vector<doubleVec> doubleMat ;
  typedef map<rVarIndex,cliquesVec> varCliqvecMap ;
  typedef varCliqvecMap::iterator varCliqvecMapIter ;
  typedef map<cliqIndex,int> cliqIntMap ;
  typedef cliqIntMap::const_iterator cliqIntMapIter ;

  // TRW:
  bool TRWcheckConvergence (const doubleVec & prevCountingNums,
                            const doubleVec & curCountingNums,
                            double tol) ;

  void updateCountNums (lbRegionBP * inf,
                        const doubleVec & newCountNums) ;

  void TRWupdateCountingNums (double alpha,
                              lbRegionBP * inf,
                              const doubleVec & prevCountNums,
                              const doubleVec & gradientTree) ;

  double TRWupdateAlpha (double alpha0,
                         lbRegionBP * inf,
                         const doubleVec & prevCountNums,
                         const doubleVec & gradientTree) ;

  typedef adjacency_list < vecS, vecS, undirectedS,
    no_property, property < edge_weight_t, double > > Graph;
  typedef graph_traits < Graph >::edge_descriptor Edge;
  typedef graph_traits < Graph >::vertex_descriptor Vertex;
  typedef std::pair<int, int> E;

  doubleVec TRWfindMinSpanTree (const E * modelEdges,
                                int numEdges,
                                int modelNumNodes,
                                const probVector & pvWeights) ;

  void TRWbuildGraphEdges (const lbRegionBP * inf,
                           E *& modelEdges,
                           int & modelNumNodes,
                           cliquesVec & edgeCliqs) ;

  void trbp (lbRegionBP * inf) ;

  glp_prob * valoptBuildLP (lbRegionBP * inf) ;

  doubleVec valoptSolveLP (glp_prob * lp,
                           const probVector & edgeGradient) ;

  doubleVec valoptUpdateCountNums (lbRegionBP * inf,
                                   const doubleVec & prevCountNums,
                                   const doubleVec & gradientNums) ;

  void varvalOpt (lbRegionBP * inf) ;

} // namespace lbLib

#endif // _INFER_UTILS_H_
