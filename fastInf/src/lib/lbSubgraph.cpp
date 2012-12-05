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

// lbSubgraph.cpp
// Author: Ian McGraw

#include <lbSubgraph.h>

using namespace lbLib;

lbSubgraph::lbSubgraph(lbGraphBase const* graph) {
  assert (graph != NULL);
  
  _graph = graph;
}

lbSubgraph::~lbSubgraph() {
  // Nothing to do.
}

 
lbSubgraph::lbSubgraph(lbSubgraph const& subgraph) :
  _graph(subgraph._graph),
  _nodesPresent(subgraph._nodesPresent),
  _arcsPresent(subgraph._arcsPresent),
  _varsPresent(subgraph._varsPresent)
{
  // Nothing to do.
}


