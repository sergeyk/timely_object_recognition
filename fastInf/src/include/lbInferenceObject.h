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

#ifndef _Loopy__Inference_Object
#define _Loopy__Inference_Object

#include <lbDefinitions.h>
#include <lbModel.h>
#include <lbModelListener.h>
#include <lbMeasureDispatcher.h>

namespace lbLib {
  

/**
   This Class holds the Global Inference Object

   General Idea:
   this class is the "mother" type of all inference objects
   (i.e loopy belief).
   
   Part of the loopy belief library
   @version November 2002
   @author Ariel Jaimovich
*/

//#define PRINT_INFERENCE_STATS

  class lbInferenceObject : public lbModelListener {
    
  public:
    //contsructor :
    lbInferenceObject(lbModel & model,lbMeasureDispatcher const& disp); 
    
    //copy ctor
    explicit lbInferenceObject(lbInferenceObject const& otherObj) ;

    //dtor
    virtual ~lbInferenceObject() ;
    
    //setting evidence :
    virtual void setEvidence(lbAssignment const& assign) =0;
    
    //cahniging evidence (we can change only part of the
    //                    evidence if the other nodes are marked -1) :
    virtual void changeEvidence(lbAssignment const& assign,bool forceUpdate = false)=0;
    
    // getting full probability
    virtual probType evidenceProb() {
      return  exp(evidenceLogProb());
    }
    // getting full probability
    virtual probType evidenceLog2Prob() {
      return evidenceLogProb() * log2(exp(1)) ;
    }
    virtual probType evidenceLogProb()=0;

    // getting probabilty for a partial assignment
    virtual lbAssignedMeasure_ptr prob(varsVec const& vars) = 0;

    virtual lbAssignedMeasure_ptr prob(varsVec const& vars,cliqIndex cliq,bool computeProbs = true) = 0;
    
    // reset evidence
    virtual void resetEvidence() =0;

    virtual probType partitionFunction() ;

    virtual probType initialPartitionFunction() ;

    virtual void calculatePartition(lbAssignedMeasure_ptr* exactBeliefs = NULL)=0;

    virtual probType entropyFunction() ;

    // get MAP assignment:
    virtual lbAssignment_ptr getMAPassign() ;
    
    //here the model can notify the inference object that something has changed
    //(should there be something more specific ,i.e addClique removeClique etc. ?)
    //    virtual void modelHasChanged() =0;

    virtual lbModel& getModel() const {
      return _model;
    };

    virtual const lbGraphStruct & getGraph() const {
      return getModel().getGraph() ;
    }

    virtual void reset(bool hard = true) = 0;

    virtual void revertFactorsFromModel() = 0;

    // performs the inference (when to activate ?!)
    virtual bool calcProbs() = 0;

    virtual bool factorsUpdated(measIndicesVec const& vec)=0;    

    virtual lbMeasureDispatcher const& getDispatcher() const {
      return _measDispatcher;
    }

    bool getCalculatedInitPartition() { return _calculatedInitialPartition; }
    bool getCalculatedPartition() { return _calculatedPartition; }
    void setCalculatedInitPartition(bool b) { _calculatedInitialPartition = b; }
    void setCalculatedPartition(bool b) { _calculatedPartition = b; }
    void setPartition(probType part) { _currentPartition = part; }
    void setEntropy(probType entropy) { _currentEntropy = entropy; }

    bool isBuilt() const { return _built; }

    bool areFactorsUpdated() const { return _factorsUpdated; }
    void setFactorsUpdated(bool b) { _factorsUpdated = b; }

  protected:

    bool _built;
    
    lbAssignment _evidence;

    bool _factorsUpdated; // boolean for whether factors have been updated since last inference.

    lbMeasureDispatcher _measDispatcher; 

    bool _calculatedInitialPartition;
    bool _calculatedPartition;           // Already calced for this evidence

    probType _initialPartition;
    probType _currentPartition;
    probType _currentEntropy;

  private:

  };

      
  //contsructor :
  inline lbInferenceObject::lbInferenceObject(lbModel & model,lbMeasureDispatcher const& disp):
    lbModelListener(model),_measDispatcher(disp){
  }
    
  //copy ctor
  inline lbInferenceObject::lbInferenceObject(lbInferenceObject const& otherObj): 
    lbModelListener(otherObj._model),
    _measDispatcher(otherObj._measDispatcher){
  }

  //dtor
  inline lbInferenceObject:: ~lbInferenceObject() {    
  }
    

}

#endif

