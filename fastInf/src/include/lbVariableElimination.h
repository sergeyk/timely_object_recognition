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

/*
 * lbVariableElimination.h
 * Author: Ian McGraw
 *
 * As braindead an approach to inference as you can get.  This has
 * very few practical applications, and is more for me to just get
 * used to the code.  Essentially I just use the list of measures in
 * the model to perform variable eliminination on all but the variables
 * whos marginals we're looking for.  I don't even do any clever 
 * caching of marginals for later runs. (At least I don't multiply
 * absolutely everything together before summing :-P).
 */

#ifndef _Variable_Elimination_Inference_Object
#define _Variable_Elimination_Inference_Object

#include <lbDefinitions.h>
#include <lbInferenceObject.h>
#include <lbModelListener.h>
#include <lbRegionModel.h>

namespace lbLib {
  
  class lbVariableElimination {
    
  public:

/*
 **********************************************************
 Ctors and Dtors :
 **********************************************************
 */

    // ctor 
    lbVariableElimination(lbModel & model,
			     lbMeasureDispatcher const& disp);

    lbVariableElimination(lbRegionModel const& rgnmodel,
			     lbMeasureDispatcher const& disp);

    // dtor
    virtual ~lbVariableElimination() {}

    
    // getting probabilty for a partial assignment (the belief on this vars)
    virtual lbAssignedMeasure_ptr prob(varsVec const& vars);
       
    
  protected:


  private:
    void filterInvolvedMeasures(lbAssignedMeasurePtrVec & measures, 
				lbAssignedMeasurePtrVec & involved,
				rVarIndex var);
 
    bool isVariableInScope(lbAssignedMeasure_ptr mes, rVarIndex var);
    
    lbAssignedMeasure_ptr multiplyInvolvedMeasures(lbAssignedMeasurePtrVec & involved);

    void marginalizeSingleVariable(lbAssignedMeasurePtrVec & measures, rVarIndex var);

    lbAssignedMeasurePtrVec _measures;
    varsVec _vars;
    lbMeasureDispatcher const& _disp;
  };
};

#endif
