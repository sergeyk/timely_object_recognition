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

#ifndef _Loopy__Vars__List
#define _Loopy__Vars__List

#include <string>
#include "lbRandomVar.h"

namespace lbLib {

  /**
   
     Part of the loopy belief library
     @version November 2002
     @author Ariel Jaimovich
  */

  class lbVarsList;
  typedef lbVarsList* lbVarsList_ptr;

  class lbVarsList {
    
  public:
    inline lbVarsList();
    inline lbVarsList(const lbVarsList & other);
    inline lbVarsList(int size);
    inline ~lbVarsList();
    inline rVarIndex addRandomVar(string name);
    inline void addRandomVar(string name,rVarIndex index);
    inline void addVarList(vector<string>);
    inline int getNumOfVars() const;
    inline string getNameOfVar(rVarIndex var) const;
    inline varsVec getVarsVec() const;
  private:
    vector <string> _varsList;
  
  };

  inline lbVarsList::lbVarsList(){
    _varsList = vector<string>();
  }

  inline lbVarsList::lbVarsList(const lbVarsList & other) {
    for (uint i = 0; i < other._varsList.size(); ++i) {
      _varsList.push_back(other._varsList[i]);
    }
  }

  inline lbVarsList::lbVarsList(int size){
    _varsList = vector<string>(size);
  }

  inline lbVarsList::~lbVarsList() {
  }

  inline rVarIndex lbVarsList::addRandomVar(string var) {
    //    cerr<<"var is "<<var<<" cur size is "<<_varsList.size()<<endl;
    _varsList.push_back(var);
    //cerr<<"pushed"<<endl;
    return _varsList.size()-1;
  }
  
  inline void lbVarsList::addRandomVar(string var,rVarIndex index) {
    _varsList[index]=(var);
  }

  inline void lbVarsList::addVarList (vector<string> varsList){
    for (uint var = 0 ; var<varsList.size();var++){
      _varsList.push_back(varsList[var]);
    }
  }
  
  inline int lbVarsList::getNumOfVars() const{
    return _varsList.size();
  }

  inline string lbVarsList::getNameOfVar(rVarIndex var) const{
    return _varsList[var];
  };
  
  inline varsVec lbVarsList::getVarsVec() const{
    uint size = _varsList.size();
    varsVec res(size);
    for (uint i=0;i<size;i++)
      res[i]=i;
    return res;
  }
}

#endif
