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

#include <lbTableMeasure.h>
#include <lbMeasureDispatcher.h>
using namespace lbLib;

lbMeasure_Sptr lbMeasureDispatcher::getNewMeasure() const {
    if (_type == MT_TABLE) {
      return lbMeasure_Sptr(new lbTableMeasure<lbLogValue>());
    }
    else {
      return lbMeasure_Sptr(new lbTableMeasure<lbValue>());
    }
  }

lbMeasure_Sptr lbMeasureDispatcher::getNewMeasure(cardVec const& card) const{
    if (_type == MT_TABLE) {
      return lbMeasure_Sptr(new lbTableMeasure<lbLogValue>(card));
    }
    else {
      return lbMeasure_Sptr(new lbTableMeasure<lbValue>(card));
    }
  }

lbMeasure_Sptr lbMeasureDispatcher::getNewMeasure(cardVec const& card,
							   bool random) const{
    if (_type == MT_TABLE) {
      return lbMeasure_Sptr(new lbTableMeasure<lbLogValue>(card, random));
    }
    else {
      return lbMeasure_Sptr(new lbTableMeasure<lbValue>(card, random));
    }
  }

lbMeasure_Sptr lbMeasureDispatcher::getNewMeasure(lbMeasure const& oldMeas) const{
    if (_type == MT_TABLE) {
      return lbMeasure_Sptr(new lbTableMeasure<lbLogValue>(oldMeas));
    }
    else {
      return lbMeasure_Sptr(new lbTableMeasure<lbValue>(oldMeas));
    }
  }

