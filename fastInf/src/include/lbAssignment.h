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

#ifndef _Loopy__Assignment
#define _Loopy__Assignment

#include <lbDefinitions.h>
#include <lbVarValues.h>

namespace lbLib {
  class lbAssignment;
  typedef lbAssignment* lbAssignment_ptr;
  typedef safeVec< lbAssignment_ptr > assignmentPtrVec;
  typedef safeVec< assignmentPtrVec > assignPtrVecVec;
  
  
  typedef lbAssignment lbFullAssignment;
  typedef lbFullAssignment* lbFullAssignment_ptr;
  typedef safeVec< lbFullAssignment_ptr > fullAssignmentPtrVec;

  class lbSmallAssignment;
  typedef lbSmallAssignment* lbSmallAssignment_ptr;
  typedef safeVec< lbSmallAssignment_ptr > smallAssignmentPtrVec;

  /*!
     This Class holds an assignment to all the variables in the model (e.g., evidence on these variables)
     In general assignment to variables are ordered by positive indices, and missing values are denoted by ? 
 
     see lbVarValues for implementation 

     Part of the fastInf library
     \version July 2009
     \author Ariel Jaimovich
  */
  class lbAssignment : public lbVarValues<varValueMap> {
  public:
    lbAssignment()
      : lbVarValues<varValueMap>() {}

    lbAssignment(uint size)
      : lbVarValues<varValueMap> (size) {}

    lbAssignment(varsVec const& vars)
      : lbVarValues<varValueMap> (vars) {}

    lbAssignment(lbAssignment const& otherAssign)
      : lbVarValues<varValueMap> (otherAssign) {}
  };

  /*!
     This Class holds an assignment to part of the variables in the model 
     It is more useful when creating evidence only on small sets of variables for iterating purposes
 
     see lbVarValues for implementation 

     Part of the fastInf library
     \version July 2009
     \author Ariel Jaimovich
  */
 
  class lbSmallAssignment : public lbVarValues<varValueSmallVec> {
  public:
    lbSmallAssignment(varsVec const& vars) :
      lbVarValues<varValueSmallVec>(vars) { };
  };

  /*
  class lbFullAssignment : public lbVarValues<varValueVec> {
  public:
    inline lbFullAssignment() : lbVarValues<varValueVec>(lbDefinitions::_numOfVars) {
      reset();
    };
  };
  */
  
};

#endif


