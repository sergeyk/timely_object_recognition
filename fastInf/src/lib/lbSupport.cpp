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

#include <lbSupport.h>

using namespace lbLib;



int lbSupport::_supportSize = 1;
bool lbSupport::_verbose = false;
bool lbSupport::_stochastic = false;
int lbSupport::_minTreeSize = 1;
int lbSupport::_maxTreeSize = 1;
bool lbSupport::_weighted = true;
bool lbSupport::_partiallyWeighted = false;
lbWeightedArcs * lbSupport::_weightedArcs = NULL;
double lbSupport::_coveragePercentage = .3;
double lbSupport::_weightThreshold = -100;

lbSupport::lbSupport (lbGraphBase const* graph, 
		      lbGraphStruct const* clusterGraph, 
		      varIndicesVecVec const& scopes) {
  assert (graph != NULL);
  assert (clusterGraph != NULL);
  assert ((int) scopes.size() == graph->getNumArcs());
  _graph = graph;
  _clusterGraph = clusterGraph;
  _scopes = &scopes;

  generateSupport();
}
    

lbSupport::~lbSupport() {
  // Nothing to do.
}

 
lbSupport::lbSupport(lbSupport const& support) {
  NOT_IMPLEMENTED_YET;
}
