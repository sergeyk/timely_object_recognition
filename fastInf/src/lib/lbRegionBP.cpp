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


/*!
  Here is an implementation of an extension the original LBP algorithm
  suggested by Yedidia, Freeman & Weiss (Constructing Free Energy Approximations and 
  Generalized Belief Propagation Algorithms, IEEE 2005).
  This algorithm is based on the Kikuchi approximation to the free energy.
 */

#include <lbUtils.h>
#include <lbRegionBP.h>

using namespace lbLib;


bool lbRegionBP::_energyWithCountNums = true ;

/*
 * CONSTRUCTORS / DESTRUCTORS
 */
lbRegionBP::lbRegionBP(lbRegionGraph const& rg,
		       lbModel & model,
		       lbMeasureDispatcher const& disp) 
  : lbBeliefPropagation(*createRegionModel(rg, model, disp), disp) {
  
  _modifyAndDeleteModel = true; //since this "owns" the model created by createRegionModel
  setInduceSpanningTrees(false);
}

lbRegionBP::lbRegionBP(lbRegionBP const& otherRegion) 
  : lbBeliefPropagation(otherRegion) {
  NOT_IMPLEMENTED_YET;
}

lbRegionBP::~lbRegionBP() {
  delete _regionGraph;
  delete _convertedGraph;

  /* Instead, in ctor set _modifyAndDeleteModel = true (so that parent class
     lbModelListener will delete _convertedModel). */
  //delete _convertedModel;
  
  delete _convertedVars;
  delete _convertedCards;
}

lbRegionGraph * lbRegionBP::createRegionGraph(lbModel const& model, string clusterFile) {
  ifstream_ptr file = lbUtils::getSmartFileStream(clusterFile.c_str());
  lbVarsList varaList(model.getGraph().getNumOfVars());

  lbGraphStruct clusterGraph(varaList);
  clusterGraph.readCliques(file);
  file->close();

  return lbRegionBP::createRegionGraph(model, clusterGraph);
}

lbRegionGraph * lbRegionBP::createRegionGraph(lbModel const& model, lbGraphStruct const& clusterGraph) {
  pair<varIndicesVecVec, facIndicesVecVec> vecPair =
    lbRegionBP::getVecsForRegionGraph(model, clusterGraph);
  
  lbRegionGraph * rg = new lbRegionGraph();

  rg->setClusterRegions(vecPair.first, vecPair.second);
  //rg->print(cerr);
  return rg;
}

lbRegionGraph * lbRegionBP::createTwoLayerRegionGraph(lbModel const& model, lbGraphStruct const& clusterGraph) {
  pair<varIndicesVecVec, facIndicesVecVec> vecPair =
    lbRegionBP::getVecsForRegionGraph(model, clusterGraph);
  
  lbRegionGraph * rg = new lbRegionGraph();

  rg->setClusterTwoLayerRegions(vecPair.first, vecPair.second);
  //  rg->print(cerr);
  return rg;
}

pair<varIndicesVecVec, facIndicesVecVec>
lbRegionBP::getVecsForRegionGraph(lbModel const& model, lbGraphStruct const& clusterGraph) {

  // GET VIVV READY FOR REGION GRAPH
  varIndicesVecVec vivv;
  int i;
  for (i = 0; i < clusterGraph.getNumOfCliques(); i++) {
    const varsVec & vars = clusterGraph.getVarsVecForClique(i);
    vivv.push_back(vars);
  }

  // GET FIVV READY FOR REGION GRAPH
  facIndicesVecVec fivv(vivv.size());
  for (i = 0; i < model.getGraph().getNumOfCliques(); i++) {
    const lbAssignedMeasure & mes = model.getAssignedMeasureForClique(i);
    for (uint j = 0; j < vivv.size(); j++) {
      const varsVec & regionVars = vivv[j];
      const varsVec & factorVars = mes.getVars();

      if (vecSubset(factorVars, regionVars)) {
	fivv[j].push_back(i);
	break;
      }
    }
  }
  
  return make_pair(vivv, fivv);
}

lbModel * lbRegionBP::createRegionModel(lbRegionGraph const& rg, 
					lbModel const & model,
					lbMeasureDispatcher const& disp) {

  // GET ACTUAL FACTORS READY FOR REGION MODEL
  lbAssignedMeasureCofwPtrVec* amcpv = new lbAssignedMeasureCofwPtrVec();
  uint i;
  for (i = 0; i < (uint)model.getGraph().getNumOfCliques(); i++) {
    const lbAssignedMeasure & mes = model.getAssignedMeasureForClique(i);
    amcpv->push_back( lbAssignedMeasureCofwPtr(&mes) );
  }

  lbRegionModel* rgnmodel = new lbRegionModel(rg, model.getCardVec(), amcpv, disp);

  _regionGraph = new lbRegionGraph(rgnmodel->getRegionGraph());
  convertModel(&_convertedGraph, &_convertedModel, &_convertedVars, &_convertedCards, *rgnmodel);
  delete rgnmodel;
  
  return _convertedModel;
}

void lbRegionBP::addSingleCliqueMeasure(lbModel *model, 
					lbRegionModel const& rgnmodel, 
					nodeIndex ni) {
  const varsVec& cliqVars = model->getGraph().getVarsVecForClique((cliqIndex) ni);
  cardVec cards = model->getCardForVars(cliqVars);
  
  // Create uniform measure over vars in region
  lbMeasure_Sptr mes =  model->measDispatcher().getNewMeasure(cards, false /* random bool */);
  lbRegion const& region = rgnmodel.getRegionGraph().getRegion(ni);
  lbAssignedMeasureConstPtrVec amcpv = rgnmodel.getCorrespondingFactors(region._facIndices);

  assert(vecSubset(region._varIndices, cliqVars));
  assert(vecSubset(cliqVars, region._varIndices));

  // Iterate through each measure, multiplying them in
  for (uint i = 0; i < amcpv.size(); i++) {
    amcpv[i]->getMeasure().marginalize(*mes, cliqVars, amcpv[i]->getVars(), true);
  }

  lbMeasure_Sptr origMeas = mes->duplicate() ;
  model->setOrigMeas((cliqIndex) ni,origMeas) ;


  //mes->print(cerr);

  //setEnergyWithCountNums (true) ;
  setEnergyWithCountNums (false) ;

  probType power = model->getCountingNum(ni);
  //cerr<<"[DEBUG] setting measure for cliq "<<ni<<endl;
  //cerr<<"[DEBUG] before raising power of: "<<power<<endl;

  if (energyWithCountingNums()) {
    // F = \sum_r cr Ur + \sum_r cr Hr
    if (cliqVars.size() == 1) { // univar
      mes->raiseToThePower(power);
    }
    // leave factor parameters unchanged
  }
  else { // !energyWithCountingNums()
    // F = \sum_r Ur + \sum_r cr Hr  (no counting numbers in U)
    if (cliqVars.size() > 1) { // factor (we don't need to raise variabels potentials)
      mes->raiseToThePower(1.0/power);
    }
    // leave univariate parameters unchanged
  }
  //cerr<<"[DEBUG] after raising power:"<<endl;
  //mes->print(cerr);

  measIndex ind = model->addMeasure(mes);
  model->setMeasureForClique((cliqIndex) ni, ind);
}

void lbRegionBP::addMeasures(lbModel *model, lbRegionModel const& rgnmodel) {
  for (nodeIndex ni = 0; ni < rgnmodel.getRegionGraph().getNumNodes(); ni++) {
    addSingleCliqueMeasure(model, rgnmodel, ni);
  }
}

lbGraphStruct * lbRegionBP::convertGraph(lbRegionGraph const& rgngraph,
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


lbModel & lbRegionBP::convertModel(lbGraphStruct **graph,
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
  (*model)->setBetheCountingNums(rgnmodel.getRegionGraph().computeBetheCountingNums()) ;

  addMeasures(*model, rgnmodel);

  return **model;
}

/*
 * Methods used directly by inference are below.
 */


void lbRegionBP::calculatePartition(lbAssignedMeasure_ptr* exactBeliefs) {

  probType result = 0.0;

  if (getCalculatedPartition()) {
    return;
  }
  probType entropyResult = 0.0;

  for (cliqIndex cliq = 0; cliq < getModel().getGraph().getNumOfCliques(); cliq++){
    varsVec cliqVars = getModel().getGraph().getVarsVecForClique(cliq);
    lbAssignedMeasure const& meas = getModel().getAssignedMeasureForClique(cliq);
    const lbMeasure_Sptr origMeas = getModel().getOrigMeas (cliq) ;

    lbAssignment assign = lbAssignment();
    assign.zeroise(cliqVars);
    varsVec movingVars = getMovingVars(cliqVars,assign);
    cardVec card = getModel().getCardForVars(movingVars);
    lbAssignedMeasure_ptr measBelief;

    if (exactBeliefs)
      measBelief = exactBeliefs[cliq];
    else
      measBelief = prob(cliqVars,cliq);

    do {
      probType lbelief = measBelief->logValueOfFull(assign);
      probType belief = exp(lbelief);
      //probType lprobab1 = meas.logValueOfFull(assign) ;
      probType lprobab = origMeas->logValueOfFull(assign,meas.getVars());
      if (belief >= lbDefinitions::ZEPSILON) {//&& lprobab >= log2(lbDefinitions::ZEPSILON) ) {//do not limit model's measure values
	probType negEntropyContrib = getModel().getCountingNum(cliq) * belief * lbelief;
        //probType negEntropyContrib = getModel().getBetheCountingNum(cliq) * belief * lbelief;
        entropyResult -= negEntropyContrib;
        result += negEntropyContrib;
        if (energyWithCountingNums()) {
          result -= belief * lprobab * getModel().getCountingNum(cliq) ;
        } else {
          result -= belief * lprobab ;
        }

        /*     
        cerr << "[DEBUG] Cliq=" << cliq
             << " b=" << belief << " log(b)=" << lbelief
             << " negEntropy=" << negEntropyContrib
             << " countingNums=" << getModel().getCountingNum(cliq)
             << " BetheCountingNums=" << getModel().getBetheCountingNum(cliq)
             << " log(theta)=" << lprobab << endl ;
        */
        
      }
    } while (assign.advanceOne(card,movingVars));

    assign.zeroise(movingVars);
    if(!exactBeliefs)
      delete measBelief;
  }

  //  cerr << "[DEBUG] Final entropy: " << entropyResult << endl ;

  if (!exactBeliefs) {
    setPartition(-result);
    setCalculatedPartition(true);
    setEntropy(entropyResult);
  }
}

double lbRegionBP::computePowerNum(cliqIndex fromCliq, cliqIndex toCliq) const {
  nodeIndexVec children = _regionGraph->getOutgoingNodeIndices((int) fromCliq);
  for (uint i = 0; i < children.size(); i++) {
    if (children[i] == toCliq) {
      return getModel().getPowerNum(toCliq);
    }
  }

  return getModel().getPowerNum(fromCliq);
}

pair<probType,probType> lbRegionBP::computePowerNumFactor(cliqIndex fromCliq, cliqIndex toCliq, bool & isDownMessage) const {
  probType powerFor,powerBack;

  bool defaultCountingNums = _regionGraph->getCountingNumsFile().empty() ;

  nodeIndexVec children = _regionGraph->getOutgoingNodeIndices((int) fromCliq);
  for (uint i = 0; i < children.size(); i++) {
    if (children[i] == toCliq) {
      // downward message
      isDownMessage = true ;
      if (defaultCountingNums) {
        double betaR = getModel().getPowerNum(toCliq) ;
        powerFor = betaR ;
        powerBack = betaR-1 ;
      }
      else {
        //fromCliq is factor toCliq is var
        //we are computing Mai
        varsVec univVars = getModel().getGraph().getVarsVecForClique(toCliq);
        assert(univVars.size()==1);
        rVarIndex var =univVars[0];
        probType ca = getModel().getCountingNum(fromCliq);
        probType ci = getModel().getCountingNum(toCliq);
        probType di = getModel().getGraph().getAllCliquesForVar(var).size()-1;
        probType qi = (1.0-ci)/di;
        probType tmpSum = (ca+1.0-qi);
        if (tmpSum==0) {
          cerr << "[ERROR] Message from factor clique " << fromCliq
               << " to variable clique " << toCliq
               << " has ca-qi+1=0 (denominator)" << endl ;
          assert(tmpSum!=0);
        }
        probType delIA = 1.0/tmpSum;
        //powerFor from factor to var (on Mai0)
        powerFor = ca*delIA;
        //powerFor from factor to var (on Nia0)
        powerBack= delIA-1.0;
        //assert(powerFor==1);
        //assert(powerBack==0);
      } // else - special counting numbers
      return make_pair(powerFor,powerBack);
    }
  }

  // else - upward message
  isDownMessage = false ;
  if (defaultCountingNums) {
    double betaR = getModel().getPowerNum(fromCliq) ;
    powerFor = betaR ;
    powerBack = betaR-1 ;
  }
  else {
    // fromCliq is var toCliq is factor
    //we are computing Nia
    varsVec univVars = getModel().getGraph().getVarsVecForClique(fromCliq);
    assert(univVars.size()==1);
    rVarIndex var =univVars[0];
    probType ca = getModel().getCountingNum(toCliq);
    probType ci = getModel().getCountingNum(fromCliq);
    //cliquesVec const& cVec = getModel().getGraph().getAllCliquesForVar(var);
    probType di = getModel().getGraph().getAllCliquesForVar(var).size()-1; // -1 since the var itself is also counted
    probType qi = (1.0-ci)/di;
    probType tmpSum = (ca+1.0-qi);
    if (tmpSum==0) {
      cerr << "[ERROR] Message from variable clique " << fromCliq
           << " to factor clique " << toCliq
           << " has ca-qi+1=0 (denominator)" << endl ;
      assert(tmpSum!=0);
    }
    probType delIA = 1.0/tmpSum;
    //powerFor from factor to var (on Nia0)
    powerFor = delIA;
    //powerFor from factor to var (on Mai0)
    powerBack= (ca*delIA)-1.0;
    // if (!(powerFor==1)) {
    //     cerr <<"Cliq num: "<<toCliq<<" var is: "<<var<<endl;
    //     cerr<<"Cliq vec: ";
    //     printVector(cVec,cerr);
    //     cerr <<"[DEBUG]"<<" ca: "<<ca<< " ci: "<< ci<<" di: "<< di<<" qi: " <<qi<<endl;
    //     cerr <<"[DEBUG]"<<" powerFor: "<<powerFor<<" powerBack: "<<powerBack<<endl;
    //     exit(1);
    //   }
    //  assert(powerBack==0);
    //  assert(powerBack==0);
  } // else - special counting numbers
  return make_pair(powerFor,powerBack);
}


lbAssignedMeasure_ptr lbRegionBP::computeMessage(messageIndex forwardIndex) const {

  cliqIndex fromCliq = forwardIndex.first;
  cliqIndex toCliq = forwardIndex.second;
  
  bool isDownMessage ;
  pair<probType,probType> powerNums = computePowerNumFactor(fromCliq, toCliq, isDownMessage);
  probType powerFor = powerNums.first;
  probType powerBack = powerNums.second;

//   cerr << "[DEBUG] From cliq " << fromCliq << " to cliq " << toCliq
//        << " fromCN=" << getModel().getCountingNum(fromCliq)
//        << " toCN=" << getModel().getCountingNum(toCliq)
//        << " forPower=" << powerFor << " backPower=" << powerBack << endl ;

  lbAssignedMeasure_ptr forward = lbBeliefPropagation::computeMessage(forwardIndex);
  //cerr << "*** Forward message:\n";
  //forward->print(cerr);

  /* For the cases below where HUGE_VAL may have been created due to 0^-POWER,
     we modify them to 0.  0 is appropriate since, in such cases, technically
     ANY value is acceptable based on the update equations for two-way GBP (see
     equations E12, E14, E15, and E16 in: Constructing free-energy
     approximations and generalized belief propagation algorithms. JS Yedidia,
     WT Freeman, Y Weiss - Information Theory, IEEE Transactions on, 2005).  We
     only require that it does not affect the relative ratio of the other
     values.  0 is guaranteed to fulfill this requirement; conversely, HUGE_VAL
     would leave ALL non-HUGE_VAL values as 0 (after normalization). */
  forward->raiseToThePower(powerFor);
  if (powerFor < 0)
    forward->replaceValues(HUGE_VAL, 0); //to remove HUGE_VALs created by 0^powerFor
  
  forward->normalize();
  if (isDownMessage && getModel().getCountingNum(fromCliq)==0) {
    forward->makeUniform() ;
  }

  messageIndex backwardIndex(toCliq, fromCliq);
  lbAssignedMeasure_ptr backward = lbBeliefPropagation::computeMessage(backwardIndex);
  //cerr << "*** Backward message:\n";
  //backward->print(cerr);
  backward->raiseToThePower(powerBack);
  if (powerBack < 0)
    backward->replaceValues(HUGE_VAL, 0); //to remove HUGE_VALs created by 0^powerBack
  
  backward->normalize();
  if (!isDownMessage && getModel().getCountingNum(toCliq)==0) {
    backward->makeUniform() ;
  }

  forward->marginalizeAndMultiply(*backward, backward->getVars());
  delete forward;

  //cerr << "*** Returning after multiplication\n";
  //backward->print(cerr);
  return backward;
}

lbMessageBank * lbRegionBP::getNewMessageBank() {
  lbMessageBank * bank = lbBeliefPropagation::getNewMessageBank();
  bank->setAffectAll(true);
  return bank;
}

