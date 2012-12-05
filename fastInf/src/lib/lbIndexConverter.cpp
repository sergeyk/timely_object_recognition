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


#include <lbIndexConverter.h>

using namespace lbLib ;


inline lbIndexConverter::lbIndexConverter(cardVec const& card, varsVec const& vars) :
  _card(card),
  _vars(vars){
}
  
inline lbIndexConverter::lbIndexConverter(lbIndexConverter const& other) :
  _card(other._card),
  _vars(other._vars){
}

inline lbIndexConverter::~lbIndexConverter() {

}
/*
void lbIndexConverter::setCard(cardVec const& card) {
  _card = card;
}

void lbIndexConverter::setVars(varsVec const& vars) {
  _vars = vars;
}
*/
inline probIndex lbIndexConverter::assignToInd(lbAssignment const& assign) const {
  probIndex index = 0 ;
  //assign.print(cout);
  int factor =1;
  for (int i =_card.size()-1 ; i>=0 ; i--) {
    index += (factor*assign.getValueForVar(_vars[i]));
    factor*= _card[i];
  }
  return index;
}

inline probIndex lbIndexConverter::assignToIndPartial(lbAssignment const& assign) const {
  probIndex index = 0 ;
  //assign.print(cout);
  int factor =1;
  for (int i =_card.size()-2 ; i>=0 ; i--) {
    index += (factor*assign.getValueForVar(_vars[i]));
    factor*= _card[i];
  }
  return index;
}
