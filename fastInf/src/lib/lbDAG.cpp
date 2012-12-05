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

// lbDAG.h
// Author: Ian McGraw

#include <lbDAG.h>

using namespace lbLib;

lbDAG::lbDAG() : lbDirectedGraph() {
  // Nothing to do.
}

lbDAG::lbDAG(int numNodes, arcVec const& arcVec) : lbDirectedGraph(numNodes, arcVec) {
  // Nothing to do.
}

lbDAG::lbDAG(lbDAG const& dag) : lbDirectedGraph(dag) 
{
  // Nothing to do.
}

lbDAG::~lbDAG() {
  // Nothing to do.
}
