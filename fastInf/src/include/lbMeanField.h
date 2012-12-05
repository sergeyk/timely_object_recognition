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

#ifndef _Mean_Field_h__
#define _Mean_Field_h__

#include <lbInferenceObject.h>

namespace lbLib {
  
/**
   Mean field approximate inference
   
   @author Gal Elidan
*/
  
  class lbMeanField : public lbInferenceObject {
    
  public:

    enum lbMFIterMethod { IM_NORMAL, IM_RESIDUAL, IM_MB_RESIDUAL, IM_LARGE_EFFECT };

    //contsructor :
    lbMeanField(lbModel & model,lbMeasureDispatcher const& disp); 
    
    //copy ctor
    explicit lbMeanField(lbMeanField const& otherObj) ;

    //dtor
    virtual ~lbMeanField() ;
    
    //setting evidence :
    virtual void setEvidence(lbAssignment const& assign);
    //                    
    virtual void changeEvidence(lbAssignment const& assign,bool forceUpdate = false);
    
    virtual probType evidenceLogProb() ;
    
    // getting probabilty for a partial assignment
    virtual lbAssignedMeasure_ptr prob(varsVec const& vars);

    virtual lbAssignedMeasure_ptr prob(varsVec const& vars,bool calc);

    virtual lbAssignedMeasure_ptr prob(varsVec const& vars,
				       cliqIndex cliq,
				       bool computeProbs = true);
    
    // reset evidence
    virtual void resetEvidence();

    virtual void calculatePartition(lbAssignedMeasure_ptr* exactBeliefs = NULL);

    virtual void reset(bool hard = true);

    virtual void revertFactorsFromModel() { assert(false); };

    // performs the inference
    virtual bool calcProbs();

    //
    virtual void testRandomOrderings();

    //
    void printMarginals(int num, bool calc = true);

    // Model listening methods
    virtual bool factorsUpdated(measIndicesVec const& vec) { assert(false); return false; };    
    virtual bool edgesUpdated(cliqIndex clique, const cliquesVec & neighbors)  { assert(false); return false; };    
    virtual void cliqueAdded(cliqIndex clique)  { assert(false); };    
    virtual void cliqueRemoved(cliqIndex clique) { assert(false); };    

    void SetIterationMethod(lbMFIterMethod IM) { _MFMethod = IM; };
    
  protected:

    void initialize();
    lbMeasure_Sptr computeMarginal(int index);
    // do mean field iterations and return number of updates
    int iterate();
    //
    int iterateResiduals();
    //
    int iterateMBResiduals();
    //
    int iterateLargestEffect();

  private:

    bool _calculated;
    measurePtrVec _measures; // marginal factors of the q distribution
    assignedMesVec _factors; // assigned measure that wrap the measures
    lbMFIterMethod _MFMethod;

  };
  
}

#endif

