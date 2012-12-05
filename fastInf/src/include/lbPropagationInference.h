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

#ifndef _Propagation_Inference_Object_
#define _Propagation_Inference_Object_


#include <lbDefinitions.h>
#include <lbOptions.h>
#include <lbMeasureDispatcher.h>
#include <lbMessageBank.h>
#include <lbInferenceObject.h>
#include <lbSpanningTree.h>
#include <lbModelListener.h>
#include <lbMathFunctions.h>
#include <lbMathUtils.h>
#include <lbGraphBase.h>

namespace lbLib {

  const int NOT_CLIQ = -1;
  
  /*
   *  lbPropagationInference is a virtual inference class for general propagation algorithms.\
   *  It encapsulates operations
   *  not dependent on messages.
   * \version July 2009
   * \author Ariel Jaimovich
   */
  class lbPropagationInference : public lbInferenceObject {
    
  public:

    /************** 
    Ctors and Dtors
    **************/

    lbPropagationInference(lbModel & model, lbMeasureDispatcher const & disp);
    explicit lbPropagationInference(lbPropagationInference const& otherPropagation);
    virtual ~lbPropagationInference();

    /********************
    Mutators
    *********************/

    virtual void setEvidence(lbAssignment const& assign);
    virtual void changeEvidence(lbAssignment const& assign, bool forceUpdate = false);
    virtual void resetEvidence();
    virtual bool factorsUpdated(measIndicesVec const& vec);  // Model measure updated.  Rerun.
    virtual void revertFactorsFromModel();  // Rerun with original measures
    void reset(bool hard = true) { initialize(!_built,!hard); }
    virtual void updateCountingNums (const vector<double> newCountNums) {
      getModel().updateCountingNums (newCountNums) ;
      resetAllCalc() ;
    }
    virtual void makeVarValidCountingNums() {
      getModel().makeVarValidCountingNums() ;
      resetAllCalc() ;
    }
    virtual void resetAllCalc() {
      reset();
      setCalculatedInitPartition (false) ;
      setCalculatedPartition (false) ;
      setCalculatedBeliefs (false) ;
    }


    /******************** 
    Accessors
    ********************/

    probType evidenceLogProb();
    lbAssignedMeasure_ptr prob(varsVec const& vars);
    lbAssignedMeasure_ptr prob(varsVec const& vars, cliqIndex cliq, bool computeProbs = true);

    // These methods return the un-normalized score of the network
    // assuming it has a full instantiation to its variables
    // (otherwise they return -HUGE_VAL or 0, respectively)
    probType fullEvidenceLogScore();
    probType fullEvidenceScore();
    
    /*******************************************
     change \ view properties of message passing
     ******************************************/

    virtual void setOptions(lbOptions & opt, int argc, char *argv[]);
    
    void setInduceSpanningTrees(bool b) { _induceSpanningTrees = b; }
    bool getInduceSpanningTrees() const { return _induceSpanningTrees; }

    lbAssignment const & getEvidence() { return _evidence; }

  protected:

    /**********************
     Initialization Methods
    **********************/

    virtual void initialize(bool allocate,bool useOldInfo = false);
    void resetFactors(measIndicesVec const& vec);
    void buildSpanningTree();  // Ensure spanning tree for each var
    void addEdgesToLocalGraph(edgeSet const& edges,rVarIndex var);



    /***********************
     Performing computations
     **********************/
    
    // Returns current belief about cliq
    virtual lbAssignedMeasure_ptr getBelief(cliqIndex cliq) const = 0;

    // Determine if this message is relevant
    virtual bool messageIsRelevant(messageIndex messIndex) const = 0;

    // Compute a message - note that it returns a ptr - don't forget to delete
    // it !
    virtual lbAssignedMeasure_ptr computeMessage(messageIndex messIndex) const = 0;

    // Messages stay dirty if they have changed.
    virtual bool stayDirty() = 0;


    /********************************
     Helper Functions
    ********************************/

    // call prob when you know which clique has most of the vars
    lbAssignedMeasure_ptr prob(varsVec const& allVars,varsVec const& vars,
			       varsVec const& notFound,cliqIndex bestCliq);

    int getNumCliques() { return _model.getGraph().getNumOfCliques(); }

    void cleanInference();

    // This method finds the index of ofCliq in wrtCliqs adj list.
    int getIndexOfNeighbor(cliqIndex wrtCliq, cliqIndex ofCliq) const;
    cliquesVec const& getNeighbors(cliqIndex cliq) const;
    varsVec const& getScope(messageIndex messIndex) const;
    lbAssignedMeasure const* getFactor(cliqIndex cliq) const;

    virtual void setCalculatedBeliefs(bool b) = 0;
    adjListVec const& localMessagesAdjList() { return _localMessagesAdjList; }
    vecSizeVec const& localSizes() { return _localSizes; }

    virtual void updateFactorFromModel(cliqIndex cliq);
  private:

    // An empty variables vector to return in case someone asks for an empty
    // scope ref
    const varsVec _emptyScope;

    // Holds a copy of all factors from model:
    lbAssignedMeasureCofwPtrVec* _factors;

    // The graph structure is maintained implicitly here
    neighborSeparatorVec _localMessagesScopes;  // Scopes of messages (in adj list order)
    adjListVec _localMessagesAdjList;           // For each clique we hold list of neighbors.
    vecSizeVec _localSizes;                     // Holds number of neighbors for each clique

    bool _induceSpanningTrees;

    void initFactors();

    friend class lbInferenceMonitor;
  };

};

#endif
