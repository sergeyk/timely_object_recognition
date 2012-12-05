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

#ifndef _MODEL_CONVERSION_
#define _MODEL_CONVERSION_

#include <lbRegionModel.h>
#include <lbModel.h>

using namespace lbLib;
using namespace std;

static lbModel & convertModel(lbGraphStruct **graph,
			      lbModel **model,
			      lbVarsList ** varsList,
			      lbCardsList ** cardsList,
			      lbRegionModel const& rgnmodel);


static void addSingleCliqueMeasure(lbModel *model, 
				   lbRegionModel const& rgnmodel, 
				   nodeIndex ni) {
  varsVec cliqVars = model->getGraph().getVarsVecForClique((cliqIndex) ni);
  cardVec cards = rgnmodel.getCorrespondingCards(cliqVars);

  // Create uniform measure over vars in region
  lbMeasure_Sptr mes =  model->measDispatcher().getNewMeasure(cards, false /* random bool */);
  lbAssignedMeasure_ptr phi = new lbAssignedMeasure(mes, cliqVars);

  lbRegion const& region = rgnmodel.getRegionGraph().getRegion(ni);
  lbAssignedMeasureConstPtrVec amcpv = rgnmodel.getCorrespondingFactors(region._facIndices);

  // Iterate through each measure, multiplying them in
  for (uint i = 0; i < amcpv.size(); i++) {
    lbAssignedMeasure const* nextMeasure = amcpv[i];
    lbAssignedMeasure *result = phi->combineMeasures(*nextMeasure, model->measDispatcher(), COMBINE_MULTIPLY);
    delete phi;
    phi = result;
  }
  
  lbAssignedMeasure *final = phi->raiseToThePower(model->getCountingNum(ni));
  measIndex ind = model->addMeasure(final->getMeasure().duplicate());
  model->setMeasureForClique((cliqIndex) ni, ind);

  delete final;
  delete phi;
}

static void addMeasures(lbModel *model, lbRegionModel const& rgnmodel) {
  for (nodeIndex ni = 0; ni < rgnmodel.getRegionGraph().getNumNodes(); ni++) {
    addSingleCliqueMeasure(model, rgnmodel, ni);
  }
}

static lbGraphStruct * convertGraph(lbRegionGraph const& rgngraph,
				    lbVarsList & varsList) {
  lbGraphStruct *graph = new lbGraphStruct(varsList);

  for (nodeIndex ni = 0; ni < (int) rgngraph.getNumNodes(); ni++) {
    lbRegion const& region = rgngraph.getRegion(ni);
    varsVec vars = region._varIndices;
    graph->addClique(vars);

    nodeIndexVec neighbors = rgngraph.getNeighboringNodeIndices(ni);
    for (nodeIndex nj = 0; nj < (int) neighbors.size(); nj++) {
      if (neighbors[nj] < ni) {
	graph->addCliqueNeighbor((cliqIndex) ni, (cliqIndex) neighbors[nj]);
      }
    }
  }

  return graph;
}


static lbModel & convertModel(lbGraphStruct **graph,
			      lbModel **model,
			      lbVarsList ** varsList,
			      lbCardsList ** cardsList,
			      lbRegionModel const& rgnmodel) {

  *varsList = new lbVarsList();

  for (uint i = 0; i < rgnmodel.getCards().size(); i++) {
    ostringstream oss;
    oss << "var" << i;
    (*varsList)->addRandomVar(oss.str());
  }

  *graph = convertGraph(rgnmodel.getRegionGraph(), **varsList);

  cardVec cards = rgnmodel.getCards();
  assert (cards.size() == (uint) (*varsList)->getNumOfVars());
  *cardsList = new lbCardsList(**varsList, cards);
  *model = new lbModel(**graph, **cardsList, rgnmodel.measDispatcher());
  (*model)->setRegionNumbers(rgnmodel.getRegionGraph().computeCountingNums(),
			     rgnmodel.getRegionGraph().computePowerNums());
  addMeasures(*model, rgnmodel);

  return **model;
}

#endif
