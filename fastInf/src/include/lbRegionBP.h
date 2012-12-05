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

#ifndef _Region_BP_
#define _Region_BP_

#include <lbBeliefPropagation.h>
#include <lbRegionModel.h>

#include<utility>
using namespace std;

namespace lbLib {
  
  class lbRegionBP : public lbBeliefPropagation {
  public:
    lbRegionBP(lbRegionGraph const& rg,
	       lbModel & model,
	       lbMeasureDispatcher const& disp);

    explicit lbRegionBP(lbRegionBP const& otherRegion) ;

    virtual ~lbRegionBP();

    virtual void calculatePartition(lbAssignedMeasure_ptr* exactBeliefs = NULL);

    virtual lbAssignedMeasure_ptr computeMessage(messageIndex forwardIndex) const;

    static lbRegionGraph * createRegionGraph(lbModel const& model, string clusterFile);

    static lbRegionGraph * createRegionGraph(lbModel const& model, lbGraphStruct const& clusterGraph);

    static lbRegionGraph * createTwoLayerRegionGraph(lbModel const& model, lbGraphStruct const& clusterGraph);

    virtual lbMessageBank * getNewMessageBank();

    //protected:

    static pair<varIndicesVecVec, facIndicesVecVec>
      getVecsForRegionGraph(lbModel const& model, lbGraphStruct const& clusterGraph);

    static bool energyWithCountingNums() {
      return _energyWithCountNums ;
    }
    static void setEnergyWithCountNums (bool val) {
      _energyWithCountNums = val ;
    }

  protected:


    lbModel * createRegionModel(lbRegionGraph const& rg, 
				lbModel const& model,
				lbMeasureDispatcher const& disp);

    /*
     * Hack helpers to convert between model types.  Needs cleaning.
     */
    static lbModel & convertModel(lbGraphStruct **graph,
				  lbModel **model,
				  lbVarsList ** varsList,
				  lbCardsList ** cardsList,
				  lbRegionModel const& rgnmodel);

    static void addSingleCliqueMeasure(lbModel *model, 
				       lbRegionModel const& rgnmodel, 
				       nodeIndex ni);
    
    static void addMeasures(lbModel *model, lbRegionModel const& rgnmodel);
    
    static lbGraphStruct * convertGraph(lbRegionGraph const& rgngraph,
					lbVarsList & varsList);

    double computePowerNum(cliqIndex fromCliq, cliqIndex toCliq) const;

    pair<probType,probType> computePowerNumFactor(cliqIndex fromCliq, cliqIndex toCliq, bool & fac2var) const;    

    typedef shared_ptr<lbRegionModel> lbRegionModel_Sptr;
    lbRegionGraph * _regionGraph;
    lbModel_ptr _convertedModel;
    lbGraphStruct_ptr _convertedGraph;
    lbVarsList_ptr _convertedVars;
    lbCardsList_ptr _convertedCards;
    static bool _energyWithCountNums ; // in the free energy formula
                                       // if true: U(mu)=sum_r Cr*U(mu_r)
                                       // if false: U(mu)=sum_r U(mu_r)
  };

};

#endif
