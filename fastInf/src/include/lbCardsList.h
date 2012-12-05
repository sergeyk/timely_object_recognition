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

#ifndef _Loopy__Cards__List
#define _Loopy__Cards__List

#include <lbVarsList.h>

namespace lbLib {

  /**


   
     Part of the loopy belief library
     @version November 2002
     @author Ariel Jaimovich
  */

  class lbCardsList;
  typedef lbCardsList* lbCardsList_ptr;

  class lbCardsList {
    
  public:
    inline lbCardsList(lbVarsList const& varsList);
    inline lbCardsList(lbVarsList const& varsList,cardVec cardsList);
    inline lbCardsList(lbCardsList const &other);
    inline ~lbCardsList();
    //inline lbCardsList& operator=(const lbCardsList& other);
    //Check if this is used. If it is,  we need to change the _varsList reference to a pointer for this to work! 

    inline string getNameOfVar(rVarIndex var) const;
    inline void setCardsForAllVars(cardVec const& cards);
    inline void setCardForVar(rVarIndex var,int card);
    inline int getCardForVar(rVarIndex var) const;
    inline cardVec getCardsVecForVarsVec(varsVec var) const;
    inline int getNumOfCards() const;
    inline void print(ostream& out) const;
    inline void printToFile(ostream& out) const;
    inline const lbVarsList& getVarsList() const;

  private:
    lbVarsList const & _varsList;
    cardVec _cards;
    
  };
  
    
  inline
    lbCardsList::lbCardsList(lbVarsList const& varsList) :
    _varsList(varsList) {
    _cards=cardVec(_varsList.getNumOfVars());
  }
  
  inline
    lbCardsList::lbCardsList(lbVarsList const& varsList,cardVec cardsList) :
    _varsList(varsList) {
    _cards=cardVec(cardsList);
  }

  inline
    lbCardsList::lbCardsList(lbCardsList const&other) :
    _varsList(other._varsList) {
    _cards = cardVec(other._cards);
  }

  inline
    string lbCardsList::getNameOfVar(rVarIndex var) const {
    return _varsList.getNameOfVar(var);
  }
  
  lbCardsList::~lbCardsList(){
  }

  inline
    const lbVarsList& lbCardsList::getVarsList() const {
    return _varsList;
  }

  inline
    void lbCardsList::setCardsForAllVars(cardVec const& cards){
    _cards=cardVec(cards);
  }
  
  void lbCardsList::setCardForVar(rVarIndex var,int card){
    if (var<(rVarIndex)_cards.size()){
      _cards[var]=card;
    }
    else if (var<_varsList.getNumOfVars()){
      int tmp=var;
      _cards.resize((int)(tmp+1));
      _cards[var]=card;
    }
    else
      cerr<<"This Var index is not legal"<<endl;
  }
  
  int lbCardsList::getCardForVar(rVarIndex var) const {
    return _cards[var];
  }

  inline cardVec lbCardsList::getCardsVecForVarsVec(varsVec vars) const{
    cardVec result;
    for (uint i=0 ; i<vars.size() ; i++)
      result.push_back(getCardForVar(vars[i]));
    return result;
  }
  
  inline int lbCardsList::getNumOfCards() const {
    return _cards.size() ;
  }

  void lbCardsList::print(ostream& out) const {
    //out<<"CARD SIZE: "<<_cards.size()<<" VARS SIZE "<<_varsList.size()<<endl;
    for (int ind =0 ;ind<_varsList.getNumOfVars();ind++)
      out<<ind<<" : "<<_cards[ind]<<endl;
  }

  inline void lbCardsList::printToFile(ostream& out) const {
    out<<"@Variables"<<endl;
    for (rVarIndex ind=0;ind<_varsList.getNumOfVars();ind++) {
      out<<_varsList.getNameOfVar(ind)<<"\t"<<_cards[ind]<<endl;
    }
    out<<"@End"<<endl;
  }
  
  /*
  inline lbCardsList& lbCardsList::operator=(const lbCardsList& other) {
    _cards = other._cards;
    _varsList = other._varsList;
    return *this;
  }
  */
    
}

#endif
