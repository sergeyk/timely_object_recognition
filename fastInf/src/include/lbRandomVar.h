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

#ifndef _Loopy__RandomVar
#define _Loopy__RandomVar

#include "lbDefinitions.h"

namespace lbLib {
/**
   This Class holds a Random Variable.

   Part of the loopy belief library
   @version November 2002
   @author Ariel Jaimovich
*/



  class lbRandomVar {

  public:

    // constructor with given index and num of values (num of values = -1
    // will create a continous random var.
    inline lbRandomVar (string name);

    //copy constructor
    inline explicit lbRandomVar(lbRandomVar const& oldVar) ;

    //dtor
    inline ~lbRandomVar() ;

    inline string getName() ;
    
    //operator ==
    //bool operator==(lbRandomVar const& otherVar) ;

  private:
    string _name;
  
  };

  
// constructor with given index and num of values (num of values = -1)
// will create a continous random var.
inline lbRandomVar::lbRandomVar (string name) {
  _name =name;
}
	      
//copy constructor
inline lbRandomVar::lbRandomVar(lbRandomVar const& oldVar) :
  _name(oldVar._name){
}

//dtor
inline lbRandomVar::~lbRandomVar() {

}

inline string lbRandomVar::getName() {
  return _name;
}

} 

#endif

