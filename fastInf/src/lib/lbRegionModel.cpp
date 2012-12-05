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

// lbRegionModel.cpp
// Author: Ian McGraw

#include <lbRegionModel.h>

using namespace lbLib;

const string lbRegionModel::FAC_DELIM = "@Factors";
const string lbRegionModel::VAR_DELIM = "@Variables";
const string lbRegionModel::END_DELIM = "@End";

lbRegionModel::lbRegionModel(lbMeasureDispatcher const& disp) 
  : _factors(new lbAssignedMeasureCofwPtrVec()), _disp(disp) {
  assert(isValid());
}

lbRegionModel::lbRegionModel(lbRegionGraph const& rgngraph,
			     cardVec const& cardVec,
			     lbAssignedMeasurePtrVec const& ampv,
			     lbMeasureDispatcher const& disp)
  : _rgnGraph(rgngraph),
    _cardVec(cardVec),
    _factors(new lbAssignedMeasureCofwPtrVec()),
    _disp(disp) {

  for (uint i = 0; i < ampv.size(); i++) {
    _factors->push_back( lbAssignedMeasureCofwPtr(ampv[i]->duplicate(),true) );
    _factors->back().setDeleteCopiedPointer(); //so that new measure will be deleted
  }

  initializeMessages();
  assert(isValid());
}

lbRegionModel::lbRegionModel(lbRegionGraph const& rgngraph,
			     cardVec const& cardVec,
			     lbAssignedMeasureCofwPtrVec* amcpv,
			     lbMeasureDispatcher const& disp)
  : _rgnGraph(rgngraph),
    _cardVec(cardVec),
    _factors(amcpv),
    _disp(disp) {

  initializeMessages();
  assert(isValid());
}

lbRegionModel::lbRegionModel(lbRegionModel const& rgnmodel) 
  : _rgnGraph(rgnmodel._rgnGraph),
    _cardVec(rgnmodel._cardVec),
    _factors(new lbAssignedMeasureCofwPtrVec()),
    _disp(rgnmodel._disp) {
uint  i;
  for (i = 0; i < rgnmodel._messages.size(); i++) {
    _messages.push_back(rgnmodel._messages[i]->duplicate());
  }

  for (i = 0; i < rgnmodel._factors->size(); i++) {
    lbAssignedMeasureCofwPtr const& amCp = (*(rgnmodel._factors))[i];
    _factors->push_back( lbAssignedMeasureCofwPtr(amCp->duplicate(),true) );
    _factors->back().setDeleteCopiedPointer(); //so that new measure will be deleted
  }

  assert(isValid());
}


lbRegionModel::~lbRegionModel() {
  if (_factors != NULL)
    delete _factors;
  
  for (arcIndex ai = 0; ai < getRegionGraph().getNumArcs(); ai++) {
    delete _messages[ai];
  }
}

