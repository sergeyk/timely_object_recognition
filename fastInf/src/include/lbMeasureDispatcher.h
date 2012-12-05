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

#ifndef _Loopy__MEASURE__DISPATCHER
#define _Loopy__MEASURE__DISPATCHER

#include <lbMeasure.h>

namespace lbLib {

  /**
     This Class holds the index converter
   
     Part of the loopy belief library
     @version November 2002
     @author Ariel Jaimovich
  */


  class lbMeasureDispatcher {

  public:

    /**
       Default Ctor, build zeroised assignment.
    */
    inline lbMeasureDispatcher(lbMeasureType type = MT_TABLE);
    
    inline ~lbMeasureDispatcher();

    inline lbMeasureType getType() const;

    lbMeasure_Sptr getNewMeasure() const;

    lbMeasure_Sptr getNewMeasure(cardVec const& card) const;

    lbMeasure_Sptr getNewMeasure(cardVec const& card, bool random) const;

    lbMeasure_Sptr getNewMeasure(lbMeasure const& oldMeas) const;

    inline void setType (lbMeasureType type);
    
  private:
    lbMeasureType _type;
  };

  inline lbMeasureDispatcher::lbMeasureDispatcher(lbMeasureType type) {
    _type = type;
  }
  
  inline lbMeasureDispatcher::~lbMeasureDispatcher() {
    
  }

  inline lbMeasureType lbMeasureDispatcher::getType() const {
    return _type;
  }

  inline void lbMeasureDispatcher::setType(lbMeasureType type) {
    _type = type;
  }
};
#endif
