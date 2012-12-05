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

#include <cmath>
#include <functional>
#include <set>
#include <map>
#include <cmath>
#include <climits>
#include <algorithm>
using namespace std;

#include <lbRandomProb.h>
#include <lbGraphStruct.h>
#include <lbModel.h>
#include <lbJunctionTree.h>
#include <lbBasicGraph.h>
#include <lbPriorityQueue.h>
using namespace lbLib;


static const realVal LN_10 = log(10.0);

/////////////////////////////////////
// 10^8.5 [since 10^8.5 long doubles take up: ((10^8.5 * 12) / 2^30) = 3.5341 GigaBytes]
/////////////////////////////////////

#define MAX_LOG10_CLIQUES_WEIGHT 8.5

/////////////////////////////////////


lbModel* lbJunctionTree::CalcJunctionTreeGraphicalModel
(const lbModel* model, lbJunctionTree::JTmethod JTmeth) {

  lbBasicGraph* g = CreatelbBasicGraphFromModel(model);
  pair<realVal, lbBasicGraph::CliqueList*> JTcliqsWeight = CalcJunctionTreeCliques(g, JTmeth);
  realVal cliquesWeight = JTcliqsWeight.first;
  lbBasicGraph::CliqueList* JTcliqs = JTcliqsWeight.second;

  //Create JT (tree) based on JTcliqs (tree nodes):
  lbBasicGraph::CliqueEdges* JTedges = CreateJunctionTree(JTcliqs);

#if defined(DEBUG)
  assert(satisfiesJunctionTreeProperty(JTcliqs,JTedges));
#endif

  cerr << "Clique state space cardinality in JT: 10^(" << cliquesWeight << ")" << endl;
    
  if (cliquesWeight > MAX_LOG10_CLIQUES_WEIGHT) {
    for (lbBasicGraph::CliqueList::iterator cliqIt = JTcliqs->begin(); cliqIt != JTcliqs->end(); ++cliqIt)
      delete *cliqIt;
    delete JTcliqs;
    delete JTedges;

    stringstream str;
    str << "JT infeasible, since total weight of cliques = 10^(" << cliquesWeight << ") > 10^(" << MAX_LOG10_CLIQUES_WEIGHT << ") !";
    throw new string(str.str());
  }

#if defined(DEBUG)
  TriangulateBasedOnCliques(*g,JTcliqs);
  assert(isTriangulated(*g));
#endif
  delete g;

  lbModel* JTgraphModel = CreateModelBasedOnCliquesMeasures(JTcliqs,JTedges,model);
  return JTgraphModel;
}

lbBasicGraph* lbJunctionTree::CreatelbBasicGraphFromModel(const lbModel* model) {

  const lbGraphStruct& lbGraph = model->getGraph();
  const int numVars = lbGraph.getNumOfVars();
  const int numCliques = lbGraph.getNumOfCliques();
  const lbCardsList& cardsList = model->getCards();

  // Create cards:
  cardVec* cards = new cardVec(numVars);
  for (rVarIndex varInd = 0; varInd < numVars; ++varInd) {
    (*cards)[varInd] = cardsList.getCardForVar(varInd);
  }

  // Create adjacencyList:
  lbBasicGraph::CliqueList* cliques = new lbBasicGraph::CliqueList();
  for (cliqIndex cliqInd = 0; cliqInd < numCliques; ++cliqInd) {
    const varsVec& cliqVars = lbGraph.getVarsVecForClique(cliqInd);
    cliques->push_back( new lbBasicGraph::Clique(cliqVars.begin(), cliqVars.end()) );
  }
  set<int>** adjacencyList = lbBasicGraph::createNewAdjacencyList(numVars);
  lbBasicGraph* g = new lbBasicGraph(adjacencyList, cards);
  TriangulateBasedOnCliques(*g, cliques);
  
  for (lbBasicGraph::CliqueList::iterator cliqIt = cliques->begin(); cliqIt != cliques->end(); ++cliqIt)
    delete *cliqIt;
  delete cliques;
  
  return g;
}

pair<realVal, lbBasicGraph::CliqueList*> lbJunctionTree::CalcJunctionTreeCliques
(const lbBasicGraph* g, lbJunctionTree::JTmethod JTmeth) {

  realVal cliquesWeight = HUGE_VAL;
  lbBasicGraph::CliqueList* cliquesFormed = NULL;

  /*For branch and bound (BranchAndBoundEliminationHeuristic),
    only try up to 100 random permutations [can be increased, but will,
    of course, increase computation time]:*/
  const realVal MAX_ELIMINATIONS_TO_TRY = 1e2;
  
  if (JTmeth == eBest) {
    string bestMethName;
    JTmethod bestMeth = eDUMMY_METHOD;
    realVal bestWeight = HUGE_VAL;
    
    for (JTmethodMap::const_iterator methIt = JT_METHOD_MAPPING.begin();
	 methIt != JT_METHOD_MAPPING.end(); ++methIt) {
      
      JTmethod currentMeth = methIt->second;
      if (currentMeth == eBest) {
	//don't want to recurse
	continue;
      }
      string methName = methIt->first;
      
      //test for the cliques' weight of the JT formed by currentMeth:
      pair<realVal, lbBasicGraph::CliqueList*> resultCliqs = CalcJunctionTreeCliques(g, currentMeth);
      realVal weight = resultCliqs.first;
      lbBasicGraph::CliqueList* deleteCliquesFormed = NULL;
      
      if (weight < bestWeight) {
	bestWeight = weight;
	bestMeth = currentMeth;
	bestMethName = methName;

	//delete old cliquesFormed:
	deleteCliquesFormed = cliquesFormed;
	cliquesFormed = resultCliqs.second;
      }
      else {
	deleteCliquesFormed = resultCliqs.second;
      }
      
      if (deleteCliquesFormed != NULL) {
	for (lbBasicGraph::CliqueList::iterator cliqIt = deleteCliquesFormed->begin();
	     cliqIt != deleteCliquesFormed->end(); ++cliqIt) {
	  delete *cliqIt;
	}
	delete deleteCliquesFormed;
      }
    }
    cerr << "Best JT method = " << bestMethName << endl;

    //used for final processing outside of if-elses:
    cliquesWeight = bestWeight;
  }
  else {//elimination heuristic methods:
    EliminationHeuristic* heur = NULL;
    switch(JTmeth) {
      case eRandom:
	heur = new RandomHeuristic();
	break;
      case eMaxCard:
	heur = new MaxCardinalityHeuristic();
	break;
      case eMinSize:
	heur = new MinSizeHeuristic();
	break;
      case eMinFill:
	heur = new MinFillHeuristic();
	break;
      case eMinWeight:
	heur = new MinWeightHeuristic();
	break;
      case eSeq:
	heur = new SequentialOrderHeuristic(false);
	break;
      case eSeqReverse:
	heur = new SequentialOrderHeuristic(true);
	break;
      case eBranchBound:
	heur = new BranchAndBoundEliminationsHeuristic<MinFillHeuristic,
	CalculateLogWeightOfClique>(MAX_ELIMINATIONS_TO_TRY);
	break;
	
      case eBest: //dealt with above separately
	break;	
      case eDUMMY_METHOD: //never used
	break;
    }
    //Calculate cliques resulting from triangulation of this lbBasicGraph:
    pair<lbBasicGraph::CliqueList*, list<int>*> triangRes =
      GetTriangulatedCliquesUsingEliminationHeuristic(*g, heur);
    delete triangRes.second;
    cliquesFormed = triangRes.first;
    if (heur != NULL)
      delete heur;
  }
  
  DiscardNonMaximalCliques(cliquesFormed);
  // want log_10:
  cliquesWeight = CalculateLogSumLogValuesOfCliques<CalculateLogWeightOfClique>(cliquesFormed,g) / LN_10;
  
  return pair<realVal, lbBasicGraph::CliqueList*>(cliquesWeight,cliquesFormed);
}

lbModel* lbJunctionTree::CreateModelBasedOnCliquesMeasures
(lbBasicGraph::CliqueList* JTcliqs, lbBasicGraph::CliqueEdges* JTedges, const lbModel* oldModel) {

  const lbGraphStruct& oldGraph = oldModel->getGraph();
  const lbVarsList& oldVars = oldGraph.getVars();
  const lbCardsList& oldCards = oldModel->getCards();

  // Create vars and cards:
  int numVars = oldVars.getNumOfVars();
  lbVarsList* varsList = new lbVarsList(numVars);
  cardVec* cards = new cardVec(numVars);
  for (rVarIndex varInd = 0; varInd < numVars; ++varInd) {
    varsList->addRandomVar(oldVars.getNameOfVar(varInd), varInd);
    (*cards)[varInd] = oldCards.getCardForVar(varInd);
  }
  lbCardsList* cardsList = new lbCardsList(*varsList, *cards);
  delete cards;

  // Create graph:
  typedef map<const lbBasicGraph::Clique*, cliqIndex> CliqMap;
  CliqMap* cliqMap = new CliqMap();
  
  lbGraphStruct* graph = new lbGraphStruct(*varsList, true);
  for (lbBasicGraph::CliqueList::iterator cliqIt = JTcliqs->begin(); cliqIt != JTcliqs->end(); ++cliqIt) {
    const lbBasicGraph::Clique* cliqVars = *cliqIt;
    varsVec* varsList = new varsVec(cliqVars->begin(), cliqVars->end());
    cliqIndex cliqInd = graph->addClique(*varsList);
    (*cliqMap)[cliqVars] = cliqInd;
    delete varsList;
    delete cliqVars;
  }
  delete JTcliqs;

  // Add the JT edges:
  typedef set<cliqIndex> NeighbSet;
  typedef map<cliqIndex, NeighbSet> AllNeighbsMap;
  AllNeighbsMap* neighbsMap = new AllNeighbsMap();
  
  for (lbBasicGraph::CliqueEdges::const_iterator edgesIt = JTedges->begin();
       edgesIt != JTedges->end(); ++edgesIt) {
    const lbBasicGraph::Clique* neighb1 = edgesIt->first;
    const lbBasicGraph::Clique* neighb2 = edgesIt->second;

    cliqIndex c1 = (*cliqMap)[neighb1];
    cliqIndex c2 = (*cliqMap)[neighb2];
    
    (*neighbsMap)[c1].insert(c2);
    (*neighbsMap)[c2].insert(c1);
  }
  delete cliqMap;
  delete JTedges;

  // The neigbors will be sorted [since map and set are BOTH sorted]:
  for (AllNeighbsMap::const_iterator neighbIt = neighbsMap->begin();
       neighbIt != neighbsMap->end(); ++neighbIt) {
    cliqIndex c = neighbIt->first;
    const NeighbSet& neighbs = neighbIt->second;

    for (NeighbSet::const_iterator nIt = neighbs.begin();
         nIt != neighbs.end(); ++nIt) {
      cliqIndex neighb = *nIt;
      if (c < neighb) //since addCliqueNeighbor is already symmetric:
        graph->addCliqueNeighbor(c, neighb);
    }
  }
  delete neighbsMap;

  // Create model:
  lbMeasureDispatcher* disp = new lbMeasureDispatcher(oldModel->measDispatcher().getType());
  lbModel* model = new lbModel(*graph, *((lbLib::lbCardsList const *)cardsList), *disp, true);

  const int numOldCliques = oldGraph.getNumOfCliques();
  typedef set<cliqIndex> CliqSet;
  CliqSet* oldCliqsToAdd = new CliqSet();
  for (cliqIndex oldCliqInd = 0; oldCliqInd < numOldCliques; ++oldCliqInd)
    oldCliqsToAdd->insert(oldCliqInd);

  typedef list<cliqIndex> CliqList;

  const int numCliques = graph->getNumOfCliques();
  for (cliqIndex cliqInd = 0; cliqInd < numCliques; ++cliqInd) {
    const varsVec& cliqVars = graph->getVarsVecForClique(cliqInd);
    cardVec cards = model->getCardForVars(cliqVars);

    // Initialize measure to have log(potential) = 0:
    lbMeasure_Sptr newMeas = lbMeasure_Sptr(disp->getNewMeasure(cards, false));

    // Add in all relevant potentials (i.e., those of sub-cliques) not yet inserted into any other clique:
    CliqList* relevantOldCliqs = new CliqList();
    CliqSet::iterator oldIt = oldCliqsToAdd->begin();
    while (oldIt != oldCliqsToAdd->end()) {
      cliqIndex oldCliqInd = *oldIt;
      const varsVec& oldCliqVars = oldGraph.getVarsVecForClique(oldCliqInd);

      //Check if oldCliqVars is contained in cliqVars:
      lbBasicGraph::Clique* oldVars = new lbBasicGraph::Clique(oldCliqVars.begin(), oldCliqVars.end());
      //cliqVars is already sorted, since taken from lbBasicGraph::Clique above:
      if (includes(cliqVars.begin(), cliqVars.end(), oldVars->begin(), oldVars->end())) {
	relevantOldCliqs->push_back(oldCliqInd);
	oldCliqsToAdd->erase(oldIt++);
      }
      else {
	++oldIt;
      }
      delete oldVars;
    }

    // Iterate over all values of cliqVars:
    lbAssignment_ptr assignPtr = new lbAssignment();
    assignPtr->zeroise(cliqVars);
    do {
      realVal val = newMeas->logValueOfFull(*assignPtr, cliqVars);
      for (CliqList::const_iterator addIt = relevantOldCliqs->begin();
	   addIt != relevantOldCliqs->end(); ++addIt) {
	cliqIndex oldCliqInd = *addIt;
	const lbAssignedMeasure& oldMeas = oldModel->getAssignedMeasureForClique(oldCliqInd);
	const varsVec& oldCliqVars = oldMeas.getVars();
	lbAssignment_ptr oldAssignPtr = new lbAssignment();
	for (varsVec::const_iterator cliqIt = oldCliqVars.begin();
	     cliqIt != oldCliqVars.end(); ++cliqIt) {
	  rVarIndex var = *cliqIt;
	  varValue value =  assignPtr->getValueForVar(var);
	  oldAssignPtr->setValueForVar(var, value);
	}
	val += oldMeas.logValueOfFull(*oldAssignPtr);
	delete oldAssignPtr;
      }
      newMeas->setLogValueOfFull(*assignPtr, cliqVars, val);
    }
    while (assignPtr->advanceOne(cards, cliqVars));

    delete assignPtr;
    delete relevantOldCliqs;
    
    //Add measure for clique:
    measIndex measInd = model->addMeasure(newMeas);
    model->setMeasureForClique(cliqInd, measInd);
  }

  bool missingCliqs = false;
  if (!oldCliqsToAdd->empty()) {
    missingCliqs = true;
#if defined(DEBUG)
    cerr << "Junction tree cliques:" << endl;
    for (cliqIndex cliqInd = 0; cliqInd < numCliques; ++cliqInd) {
      const varsVec& cliqVars = graph->getVarsVecForClique(cliqInd);
      for (varsVec::const_iterator cliqIt = cliqVars.begin(); cliqIt != cliqVars.end(); ++cliqIt)
	cerr << *cliqIt << " ";
      cerr << endl;
    }
    cerr << "Orphan cliques:" << endl;
    for (CliqSet::const_iterator oldIt = oldCliqsToAdd->begin(); oldIt != oldCliqsToAdd->end(); ++oldIt) {
      const varsVec& cliqVars = oldGraph.getVarsVecForClique(*oldIt);
      for (varsVec::const_iterator cliqIt = cliqVars.begin(); cliqIt != cliqVars.end(); ++cliqIt)
	cerr << *cliqIt << " ";
      cerr << endl;
    }
#endif
  }
  delete oldCliqsToAdd;
  if (missingCliqs)
    throw new string("lbJunctionTree: internal error -- not all original cliques are covered in junction tree!");
  
  return model;
}

pair< lbBasicGraph::CliqueList*, list<int>* > lbJunctionTree::GetTriangulatedCliquesUsingEliminationHeuristic
(const lbBasicGraph& g, lbJunctionTree::EliminationHeuristic* heur,
 pair<lbJunctionTree::calculateCliqueValue, realVal>* upperBound) {  
  if (heur == NULL) {
    return pair< lbBasicGraph::CliqueList*, list<int>* >(NULL,NULL);
  }
  calculateCliqueValue boundFunction = NULL;
  realVal upperBoundValue = HUGE_VAL;
  if (upperBound != NULL) {
    boundFunction = upperBound->first;
    upperBoundValue = upperBound->second;
  }
  
  lbBasicGraph* eliminatedlbBasicGraph = new lbBasicGraph(g);
  lbBasicGraph::CliqueList* cliques = new lbBasicGraph::CliqueList();
  list<int>* elimOrder = new list<int>();
  while(eliminatedlbBasicGraph->numVertices() > 0) {//not all vertices eliminated
    int next = heur->chooseNextVertexToEliminate(*eliminatedlbBasicGraph);
    const lbBasicGraph::Clique* neighbs = eliminatedlbBasicGraph->eliminateVertex(next);
    cliques->push_back(neighbs);
    elimOrder->push_back(next);

    if (upperBound != NULL) {
      realVal currentCliqueValue = boundFunction(neighbs,g);
      if (currentCliqueValue > upperBoundValue) {
	delete eliminatedlbBasicGraph;
	for (lbBasicGraph::CliqueList::iterator cliqIt = cliques->begin(); cliqIt != cliques->end(); ++cliqIt) {
	  delete *cliqIt;
	}	
	delete cliques;
	delete elimOrder;
	return pair< lbBasicGraph::CliqueList*, list<int>* >(NULL,NULL);
      }
    }
  }
  delete eliminatedlbBasicGraph;
  return pair< lbBasicGraph::CliqueList*, list<int>* >(cliques,elimOrder);
}

realVal lbJunctionTree::CalculateLogWeightOfClique(const lbBasicGraph::Clique* clique, const lbBasicGraph& g) {
  realVal weight = 0;
  for (lbBasicGraph::Clique::const_iterator it = clique->begin(); it != clique->end(); ++it) {
    weight += log((realVal)g.getStateSpaceCardinality(*it));
  }
  return weight;
}

realVal lbJunctionTree::CalculateFillEdgesOfClique(const lbBasicGraph::Clique* clique, const lbBasicGraph& g) {
  int numEdgesNecessary = 0;
  for (lbBasicGraph::Clique::const_iterator it1 = clique->begin(); it1 != clique->end(); ++it1) {
    for (lbBasicGraph::Clique::const_iterator it2 = clique->begin(); it2 != clique->end(); ++it2) {
      if (*it1 < *it2 && !g.hasEdge(*it1,*it2)) {
	//only count once the edges that would have to be added
	++numEdgesNecessary;
      }
    }
  }
  return numEdgesNecessary;
}

void lbJunctionTree::DiscardNonMaximalCliques(lbBasicGraph::CliqueList* cliques) {
  for (lbBasicGraph::CliqueList::iterator cliqIt1 = cliques->begin();
       cliqIt1 != cliques->end(); ++cliqIt1) {
    const lbBasicGraph::Clique* c1 = *cliqIt1;
    for (lbBasicGraph::CliqueList::const_iterator cliqIt2 = cliques->begin();
	 cliqIt2 != cliques->end(); ++cliqIt2) {
      const lbBasicGraph::Clique* c2 = *cliqIt2;
      
      if (c2 != c1 && includes(c2->begin(), c2->end(), c1->begin(), c1->end())) {
	//there exists a Clique c2 s.t. c1 is a proper subset of c2, so don't retain c1
	delete c1;	
	cliques->erase(cliqIt1++);
	--cliqIt1;
	break;
      }
    }
  }
}

void lbJunctionTree::TriangulateBasedOnCliques(lbBasicGraph& g, const lbBasicGraph::CliqueList* cliques) {
  for (lbBasicGraph::CliqueList::const_iterator allCliquesIt = cliques->begin();
       allCliquesIt != cliques->end(); ++allCliquesIt) {
    const lbBasicGraph::Clique* c = *allCliquesIt;
    for (lbBasicGraph::Clique::const_iterator cliqIt1 = c->begin(); cliqIt1 != c->end(); ++cliqIt1) {

      lbBasicGraph::Clique::const_iterator cliqIt2 = cliqIt1;
      for (++cliqIt2; cliqIt2 != c->end(); ++cliqIt2) {//Start at cliqIt1 + 1
	g.addEdge(*cliqIt1,*cliqIt2);
      }
    }
  }
}

bool lbJunctionTree::isTriangulated(const lbBasicGraph& g) {
  EliminationHeuristic* heur = new MaxCardinalityHeuristic();
  lbBasicGraph* triangulatedlbBasicGraph = new lbBasicGraph(g);

  pair<lbBasicGraph::CliqueList*, list<int>*> triangRes =
    GetTriangulatedCliquesUsingEliminationHeuristic(*triangulatedlbBasicGraph, heur);
  delete triangRes.second;
  lbBasicGraph::CliqueList* cliques = triangRes.first; 
  delete heur;
  DiscardNonMaximalCliques(cliques);
  TriangulateBasedOnCliques(*triangulatedlbBasicGraph,cliques);
  for (lbBasicGraph::CliqueList::iterator cliqIt = cliques->begin(); cliqIt != cliques->end(); ++cliqIt) {
    delete *cliqIt;
  }  
  delete cliques;
  
  bool isTriangulated = (*triangulatedlbBasicGraph == g);//g was triangulated <==> no edge added to triangulatedlbBasicGraph
  delete triangulatedlbBasicGraph;
  return isTriangulated;  
}

lbBasicGraph::CliqueEdges* lbJunctionTree::CreateJunctionTree(const lbBasicGraph::CliqueList* cliques) {
  lbBasicGraph::CliqueVector* cliqueNodes =
    new lbBasicGraph::CliqueVector(cliques->begin(), cliques->end());

  typedef int CliqueNode;
  typedef unsigned int CliquelbBasicGraphEdgeWeight;
  typedef pair<CliqueNode,CliquelbBasicGraphEdgeWeight> CliquelbBasicGraphEdge;
  typedef map<CliqueNode, set<CliquelbBasicGraphEdge>*> CliquelbBasicGraph;
  CliquelbBasicGraph* cliquelbBasicGraph = new CliquelbBasicGraph();
  
  typedef vector<pair<CliqueNode, CliquelbBasicGraphEdgeWeight> > CliqueNodeWeightVector;
  CliqueNodeWeightVector* cliqueWeightVec = new CliqueNodeWeightVector();
  
  int numCliques = (int)cliqueNodes->size();
  for (CliqueNode index = 0; index < numCliques; ++index) {
    (*cliquelbBasicGraph)[index] = new set<CliquelbBasicGraphEdge>();
  }
  
  for (CliqueNode index1 = 0; index1 < numCliques; ++index1) {
    cliqueWeightVec->push_back(pair<CliqueNode,CliquelbBasicGraphEdgeWeight>(index1,0));
    
    for (CliqueNode index2 = index1+1; index2 < numCliques; ++index2) {      
      lbBasicGraph::Clique* intersectClique = new lbBasicGraph::Clique();
      insert_iterator<lbBasicGraph::Clique> resultIt(*intersectClique, intersectClique->begin());
      set_intersection((*cliqueNodes)[index1]->begin(), (*cliqueNodes)[index1]->end(),
		       (*cliqueNodes)[index2]->begin(), (*cliqueNodes)[index2]->end(),
		       resultIt);
      CliquelbBasicGraphEdgeWeight edgeWeight = intersectClique->size();
      delete intersectClique;
      if (edgeWeight > 0) {
	(*cliquelbBasicGraph)[index1]->insert(CliquelbBasicGraphEdge(index2,edgeWeight));
	(*cliquelbBasicGraph)[index2]->insert(CliquelbBasicGraphEdge(index1,edgeWeight));
      }
    }
  }
  
  //implement Prim's algorithm for finding a Maximum-Weight Spanning Tree:  
  lbPriorityQueue<CliqueNode, CliquelbBasicGraphEdgeWeight>* cliqueWeightQueue =
    new lbPriorityQueue<CliqueNode, CliquelbBasicGraphEdgeWeight>(*cliqueWeightVec);
  delete cliqueWeightVec;

  map<CliqueNode, CliqueNode>* treeParents = new map<CliqueNode, CliqueNode>();
  for (CliqueNode index = 0; index < numCliques; ++index) {
    (*treeParents)[index] = -1; //no CliqueNode has a parent yet
  }
  lbBasicGraph::CliqueEdges* jte = new lbBasicGraph::CliqueEdges();
  
  while (!cliqueWeightQueue->empty()) {
    CliqueNode currentClique = cliqueWeightQueue->deleteMax();
    CliqueNode parent = (*treeParents)[currentClique];
    if (parent != -1) {
      const lbBasicGraph::Clique* cur = (*cliqueNodes)[currentClique];
      const lbBasicGraph::Clique* par = (*cliqueNodes)[parent];
      jte->insert(pair<const lbBasicGraph::Clique*,const lbBasicGraph::Clique*>(cur,par));
    }
    //relax currentClique's neighbors:
    set<CliquelbBasicGraphEdge>* currentNeighbs = (*cliquelbBasicGraph)[currentClique];
    for (set<CliquelbBasicGraphEdge>::const_iterator neighbsIt = currentNeighbs->begin();
	 neighbsIt != currentNeighbs->end(); ++neighbsIt) {
      CliquelbBasicGraphEdge currentEdge = *neighbsIt;
      CliqueNode currentNeighb = currentEdge.first;
      if ((*cliquelbBasicGraph)[currentNeighb] != NULL) {//currentNeighb not yet added to JT
	CliquelbBasicGraphEdgeWeight edgeWeight = currentEdge.second;
	CliquelbBasicGraphEdgeWeight currentNeighbMaxWeight = cliqueWeightQueue->getValue(currentNeighb);
	if (edgeWeight > currentNeighbMaxWeight) {
	  cliqueWeightQueue->increaseValue(currentNeighb,edgeWeight);
	  (*treeParents)[currentNeighb] = currentClique;
	}
      }
    }
    //remove currentClique from graph, so know that already visited it:
    delete currentNeighbs;
    (*cliquelbBasicGraph)[currentClique] = NULL;
  }
  delete cliqueNodes;
  delete treeParents;
  delete cliquelbBasicGraph;
  delete cliqueWeightQueue;
  return jte;
}

bool lbJunctionTree::satisfiesJunctionTreeProperty(const lbBasicGraph::CliqueList* cliques,
						   const lbBasicGraph::CliqueEdges* JTedges) {
  if (cliques->empty()) {
    return true;
  }
  set<int>* allVariables = new set<int>();
  typedef map<const lbBasicGraph::Clique*, lbBasicGraph::CliqueSet*> CliquelbBasicGraph;
  CliquelbBasicGraph* cliquelbBasicGraph = new CliquelbBasicGraph();
  
  //takes O(V+E) time, but saves time so that in each variable's BFS, can check each node's neighbors only:
  for (lbBasicGraph::CliqueList::const_iterator cliqIt = cliques->begin();
       cliqIt != cliques->end(); ++cliqIt) {
    const lbBasicGraph::Clique* currentClique = *cliqIt;
    (*cliquelbBasicGraph)[currentClique] = new lbBasicGraph::CliqueSet();
    for (lbBasicGraph::Clique::const_iterator cliqMembers = currentClique->begin();
	 cliqMembers != currentClique->end(); ++cliqMembers) {
      allVariables->insert(*cliqMembers);
    }
  }
  for (lbBasicGraph::CliqueEdges::const_iterator JTedgeIt = JTedges->begin();
       JTedgeIt != JTedges->end(); ++JTedgeIt) {
    const lbBasicGraph::Clique* neighb1 = JTedgeIt->first;
    const lbBasicGraph::Clique* neighb2 = JTedgeIt->second;
    
    (*cliquelbBasicGraph)[neighb1]->insert(neighb2);
    (*cliquelbBasicGraph)[neighb2]->insert(neighb1);
  }
  
  set<int>* variablesAlreadyChecked = new set<int>();
  set<int>::const_iterator allVariablesIt;
  for (allVariablesIt = allVariables->begin(); allVariablesIt != allVariables->end(); ++allVariablesIt) {
    int currentVariable = *allVariablesIt;    
    variablesAlreadyChecked->insert(currentVariable);

    lbBasicGraph::CliqueSet* cliquesContainingMember = new lbBasicGraph::CliqueSet();
    for (lbBasicGraph::CliqueList::const_iterator checkCliqIt = cliques->begin();
	 checkCliqIt != cliques->end(); ++checkCliqIt) {
      const lbBasicGraph::Clique* checkClique = *checkCliqIt;
      if (checkClique->find(currentVariable) != checkClique->end()) {
	cliquesContainingMember->insert(checkClique);
      }
    }
    //run BFS on the cliquesContainingMember sub-graph to make sure that it is connected:
    lbBasicGraph::CliqueList* BFSqueue = new lbBasicGraph::CliqueList();
    lbBasicGraph::CliqueSet* alreadyVisited = new lbBasicGraph::CliqueSet();
      
    //start BFS with some clique containing currentVariable:
    const lbBasicGraph::Clique* startClique = *(cliquesContainingMember->begin());
    BFSqueue->push_back(startClique);
    alreadyVisited->insert(startClique);

    while (!BFSqueue->empty()) {
      const lbBasicGraph::Clique* nextClique = BFSqueue->front();
      BFSqueue->pop_front();

      //check all of nextClique's neighbs:
      lbBasicGraph::CliqueSet* nextCliqueNeighbs = (*cliquelbBasicGraph)[nextClique];
      for (lbBasicGraph::CliqueSet::const_iterator neighbIt = nextCliqueNeighbs->begin();
	   neighbIt != nextCliqueNeighbs->end(); ++neighbIt) {
	const lbBasicGraph::Clique* neighbClique = *neighbIt;
	if (cliquesContainingMember->find(neighbClique) == cliquesContainingMember->end() ||
	    alreadyVisited->find(neighbClique) != alreadyVisited->end()) {
	  //irrelevant neighbor OR already checked neighbor, respectively
	  continue;
	}
	BFSqueue->push_back(neighbClique);
	alreadyVisited->insert(neighbClique);
      }
    }
    //check that all cliques in cliquesContainingMember were visited:
    bool memberlbBasicGraphSatisfiesProperty = (*cliquesContainingMember == *alreadyVisited);
    
    delete BFSqueue;
    delete alreadyVisited;
    delete cliquesContainingMember;
    if (!memberlbBasicGraphSatisfiesProperty) {
      break;
    }
  }
  //managed to check all variables, without breaking out of for loop:
  bool allVariablesSatisfyJTproperty = (allVariablesIt == allVariables->end());
  delete allVariables;
  for (lbBasicGraph::CliqueList::const_iterator cliqIt = cliques->begin();
       cliqIt != cliques->end(); ++cliqIt) {
    delete (*cliquelbBasicGraph)[*cliqIt];
  }
  delete cliquelbBasicGraph;
  delete variablesAlreadyChecked;  
  return allVariablesSatisfyJTproperty;
}

bool lbJunctionTree::isConnected(const lbBasicGraph& g) {
  ConnectedComponents* comp = getConnectedComponents(g);
  bool graphIsConnected = (comp->size() <= 1); //0 or 1 connected components
  for (ConnectedComponents::iterator compIt = comp->begin(); compIt != comp->end(); ++compIt) {
    delete *compIt;
  }
  delete comp;
  return graphIsConnected;
}

lbJunctionTree::ConnectedComponents* lbJunctionTree::getConnectedComponents(const lbBasicGraph& g) {
  ConnectedComponents* components = new ConnectedComponents();
  if (g.numVertices() == 0) {
    return components;
  }
  typedef int Vertex;
  list<Vertex>* BFSqueue = new list<Vertex>();
  set<Vertex>* nonVisited = new set<Vertex>();
  for (lbBasicGraph::vertexIterator vertIt = g.verticesBegin(); vertIt != g.verticesEnd(); ++vertIt) {
    nonVisited->insert(*vertIt);
  }

  //start BFS with some variable:
  set<Vertex>::iterator firstNonVisitIt = nonVisited->begin();
  Vertex startVar = *firstNonVisitIt;
  BFSqueue->push_back(startVar);
  nonVisited->erase(firstNonVisitIt);
  lbBasicGraph::Clique* currentComp = new lbBasicGraph::Clique();
  
  while (!BFSqueue->empty() || !nonVisited->empty()) {
    if (BFSqueue->empty()) {//finished with the previous connected component, so move onto next one:
      components->push_back(currentComp);
      currentComp = new lbBasicGraph::Clique();
      
      set<Vertex>::iterator firstNonVisitIt = nonVisited->begin();
      Vertex startVar = *firstNonVisitIt;
      BFSqueue->push_back(startVar);
      nonVisited->erase(firstNonVisitIt);
    }
    
    Vertex nextVar = BFSqueue->front();
    BFSqueue->pop_front();
    currentComp->insert(nextVar);
    
    //check all of nextVar's neighbs:
    for (lbBasicGraph::neighborIterator neighbIt = g.neighbBegin(nextVar);
	 neighbIt != g.neighbEnd(nextVar); ++neighbIt) {
      Vertex neighbVar = *neighbIt;

      set<Vertex>::iterator findNeighbIt = nonVisited->find(neighbVar);
      if (findNeighbIt == nonVisited->end()) {//already checked neighbor
	continue;
      }
      BFSqueue->push_back(neighbVar);
      nonVisited->erase(findNeighbIt);
    }
  }
  delete BFSqueue;
  delete nonVisited;

  components->push_back(currentComp);
  return components;
}

////////////////////////////////////////////////////////////////////////////////
const realVal lbJunctionTree::EliminationHeuristic::LessThan::worst = HUGE_VAL;
const realVal lbJunctionTree::EliminationHeuristic::GreaterThan::worst = -HUGE_VAL;

////////////////////////////////////////////////////////////////////////////////
realVal lbJunctionTree::EliminationHeuristic::CalculatePossibleCliqueSize(int vertToEliminate, const lbBasicGraph& g) {
  return g.numNeighbors(vertToEliminate);
}

realVal lbJunctionTree::EliminationHeuristic::CalculatePossibleCliqueFillEdges
(int vertToEliminate, const lbBasicGraph& g) {
  int numNeighbs = g.numNeighbors(vertToEliminate);
  int numActualEdges = 0;
  int numPotentialEdges = numNeighbs * (numNeighbs-1) / 2; //numNeighbs Choose 2
  for (lbBasicGraph::neighborIterator neighb1 = g.neighbBegin(vertToEliminate);
       neighb1 != g.neighbEnd(vertToEliminate); ++neighb1) {
    for (lbBasicGraph::neighborIterator neighb2 = g.neighbBegin(vertToEliminate);
	 neighb2 != g.neighbEnd(vertToEliminate); ++neighb2) {
      if (*neighb1 < *neighb2 && g.hasEdge(*neighb1,*neighb2)) {
	++numActualEdges;
      }
    }
  }    
  int numFillEdges = numPotentialEdges - numActualEdges;
  return numFillEdges;
}

realVal lbJunctionTree::EliminationHeuristic::CalculatePossibleCliqueLogWeight(int vertToEliminate,const lbBasicGraph& g){
  realVal weight = log((realVal)g.getStateSpaceCardinality(vertToEliminate));
  for (lbBasicGraph::neighborIterator neighb = g.neighbBegin(vertToEliminate);
       neighb != g.neighbEnd(vertToEliminate); ++neighb) {
    weight += log((realVal)g.getStateSpaceCardinality(*neighb));
  }
  return weight;
}

//////////////////////////////////////////////////////////////////////
lbJunctionTree::SpecificOrderHeuristic::SpecificOrderHeuristic
(vector<int>* eliminationOrder, const lbBasicGraph* originallbBasicGraph)
  : _position(eliminationOrder->begin()), _originallbBasicGraph(originallbBasicGraph) {
  if (eliminationOrder->size() != originallbBasicGraph->numVertices()) {
    throw new string("Error in sizes in lbJunctionTree::SpecificOrderHeuristic");
  }
}

int lbJunctionTree::SpecificOrderHeuristic::chooseNextVertexToEliminate(const lbBasicGraph& eliminatedSoFar) {
  return *(_position++);
}

//////////////////////////////////////////////////////////////////////
int lbJunctionTree::RandomHeuristic::chooseNextVertexToEliminate
(const lbBasicGraph& eliminatedSoFar) {
  if (eliminatedSoFar.numVertices() == 0) {
    return -1;
  }
  unsigned int index = _lbRandomProbGenerator.RandomInt(eliminatedSoFar.numVertices());
  unsigned int count = 0;
  lbBasicGraph::vertexIterator vi;
  for (vi = eliminatedSoFar.verticesBegin(); vi != eliminatedSoFar.verticesEnd(); ++vi) {
    if (count++ == index) {
      break;
    }
  }
  return *vi;
}

//////////////////////////////////////////////////////////////////////
lbJunctionTree::MaxCardinalityHeuristic::MaxCardinalityHeuristic()
  : _eliminationOrderStack(NULL), _originallbBasicGraph(NULL) {}

lbJunctionTree::MaxCardinalityHeuristic::~MaxCardinalityHeuristic() {
  clearDataMembers();
}

void lbJunctionTree::MaxCardinalityHeuristic::clearDataMembers() {
  if (_eliminationOrderStack != NULL) {
    delete _eliminationOrderStack;
  }
  _eliminationOrderStack = NULL;
  _originallbBasicGraph = NULL;
}

int lbJunctionTree::MaxCardinalityHeuristic::chooseNextVertexToEliminate
(const lbBasicGraph& eliminatedSoFar) {
  if (_eliminationOrderStack != NULL && &eliminatedSoFar != _originallbBasicGraph) {
    clearDataMembers();
    throw new string("Error in MaxCardinalityHeuristic: lbBasicGraphs not the same!");
  }  
  if (_eliminationOrderStack == NULL) {
    _eliminationOrderStack = new list<int>();
    _originallbBasicGraph = &eliminatedSoFar;
    calculateEliminationOrder();
  }  
  if (_eliminationOrderStack == NULL || _eliminationOrderStack->empty()) {//empty graph - error
    clearDataMembers();
    return -1;
  }
  //pop next vertex to eliminate from top of stack:
  int nextToElim = _eliminationOrderStack->front();
  _eliminationOrderStack->pop_front();
  if (_eliminationOrderStack->empty()) {//finished using elimination order
    clearDataMembers();    
  }
  return nextToElim;
}

void lbJunctionTree::MaxCardinalityHeuristic::calculateEliminationOrder() {
  vector<pair<int,int> >* vertexWeightList = new vector<pair<int,int> >();
  for (lbBasicGraph::vertexIterator vi = _originallbBasicGraph->verticesBegin();
       vi != _originallbBasicGraph->verticesEnd(); ++vi) {
    vertexWeightList->push_back(pair<int,int>(*vi,0)); //initialize all weights to 0
  }
  lbPriorityQueue<int,int>* vertexWeightQueue = new lbPriorityQueue<int,int>(*vertexWeightList);  
  delete vertexWeightList;

  lbBasicGraph* unNumberedlbBasicGraph = new lbBasicGraph(*_originallbBasicGraph);
  while (!vertexWeightQueue->empty()) {
    int currentVertex = vertexWeightQueue->deleteMax();
    _eliminationOrderStack->push_front(currentVertex);

    /*Implements path search according to correspondence with Barry Peyton of
      MCS-M of Anne Berry, et. al:
      [add vertex v to S iff there exists a path from currentVertex to v s.t.
      the weights of each vertex in the path < weight(v), OR v is a neighbor
      of currentVertex in _originallbBasicGraph]*/
    set<int>* S = new set<int>();
    int MAX_WEIGHT = -1;
    
    for (lbBasicGraph::vertexIterator vi = unNumberedlbBasicGraph->verticesBegin();
	 vi != unNumberedlbBasicGraph->verticesEnd(); ++vi) {
      if (*vi == currentVertex) //ignore currentVertex
	continue;
      int vertWeight = vertexWeightQueue->getValue(*vi);
      if (vertWeight > MAX_WEIGHT) {
	MAX_WEIGHT = vertWeight;
      }
    }
    set<int>** B = new set<int>*[MAX_WEIGHT+1];
    for (int j = 0; j <= MAX_WEIGHT; ++j) {
      B[j] = new set<int>();
    }
    set<int>* visitedVertices = new set<int>();

    //For x in adj(currentVertex) and x is unnumbered
    for (lbBasicGraph::neighborIterator currentsNeighbIt = unNumberedlbBasicGraph->neighbBegin(currentVertex);
	 currentsNeighbIt != unNumberedlbBasicGraph->neighbEnd(currentVertex); ++currentsNeighbIt) {
      int neighb = *currentsNeighbIt;
      int neighbWeight = vertexWeightQueue->getValue(neighb);
      
      B[neighbWeight]->insert(neighb);
      S->insert(neighb);
      visitedVertices->insert(neighb);
    }

    for (int j = 0; j <= MAX_WEIGHT; ++j) {
      while (!B[j]->empty()) {
	set<int>::iterator Bj_iter = B[j]->begin();
	int y = *Bj_iter;
	B[j]->erase(Bj_iter);

	for (lbBasicGraph::neighborIterator yNeighbIt = unNumberedlbBasicGraph->neighbBegin(y);
	     yNeighbIt != unNumberedlbBasicGraph->neighbEnd(y); ++yNeighbIt) {//unnumbered neighbors of y
	  int x = *yNeighbIt;
	  if (x == currentVertex) //ignore currentVertex
	    continue;

	  if (visitedVertices->find(x) == visitedVertices->end()) {//unnumbered, unvisited neighbors of y
	    int xWeight = vertexWeightQueue->getValue(x); //w(x)
	    
	    if (xWeight > j) {//found a path from v-->x [thru y] of weight j [j < w(x) ], so add x to S
	      S->insert(x);
	      B[xWeight]->insert(x);
	    }
	    else {/*w(x) <= j, so found a path from v-->x of weight j >= w(x).  Since this is the 1st
		    (and only) time x has been visited, then this j is the lightest path to x,
		    so x will never be added to S. But inform its neighbors of the path of weight j to x */
	      B[j]->insert(x);
	    }
	    visitedVertices->insert(x);
	  }
	}
      }
    }
    delete visitedVertices;
    for (int j = 0; j <= MAX_WEIGHT; ++j) {
      delete B[j];
    }
    delete[] B;

    for (set<int>::const_iterator SIter = S->begin(); SIter != S->end(); ++SIter) {
      int u = *SIter;
      int uWeight = vertexWeightQueue->getValue(u);
      vertexWeightQueue->increaseValue(u, uWeight+1);
    }
    delete S;

    //remove currentVertex from lbBasicGraph of unnumbered vertices only (for next round):
    unNumberedlbBasicGraph->removeVertex(currentVertex);
  }
  delete unNumberedlbBasicGraph;
  delete vertexWeightQueue; 
}

//////////////////////////////////////////////////////////////////////
//min size <==> minimum number of neighbors
int lbJunctionTree::MinSizeHeuristic::chooseNextVertexToEliminate
(const lbBasicGraph& eliminatedSoFar) {
  return chooseNextVertexCheckEachElimination<LessThan>
    (eliminatedSoFar,
     lbJunctionTree::EliminationHeuristic::CalculatePossibleCliqueSize,
     lbJunctionTree::EliminationHeuristic::CalculatePossibleCliqueFillEdges);
}

//////////////////////////////////////////////////////////////////////
int lbJunctionTree::MinFillHeuristic::chooseNextVertexToEliminate
(const lbBasicGraph& eliminatedSoFar) {
  return chooseNextVertexCheckEachElimination<LessThan>
    (eliminatedSoFar,
     lbJunctionTree::EliminationHeuristic::CalculatePossibleCliqueFillEdges,
     lbJunctionTree::EliminationHeuristic::CalculatePossibleCliqueSize);
}

//////////////////////////////////////////////////////////////////////
int lbJunctionTree::MinWeightHeuristic::chooseNextVertexToEliminate
(const lbBasicGraph& eliminatedSoFar) {
  return chooseNextVertexCheckEachElimination<LessThan>
    (eliminatedSoFar,
     lbJunctionTree::EliminationHeuristic::CalculatePossibleCliqueLogWeight,
     lbJunctionTree::EliminationHeuristic::CalculatePossibleCliqueFillEdges);  
}

//////////////////////////////////////////////////////////////////////
lbJunctionTree::SequentialOrderHeuristic::SequentialOrderHeuristic(bool reverseOrder) : _reverseOrder(reverseOrder) {
}

int lbJunctionTree::SequentialOrderHeuristic::chooseNextVertexToEliminate
(const lbBasicGraph& eliminatedSoFar) {
  if (!_reverseOrder) {
    return *(eliminatedSoFar.verticesBegin());
  }
  //_reverseOrder:
  return *(--(eliminatedSoFar.verticesEnd()));
}

//////////////////////////////////////////////////////////////////////
ostream& lbJunctionTree::print(ostream& stream, const lbBasicGraph::Clique& c, char delim) {
  lbBasicGraph::Clique::iterator cliqIt = c.begin();
  while (cliqIt != c.end()) {
    stream << *cliqIt;
    if (++cliqIt == c.end())
      break;
    
    stream << delim;
  }
  return stream;
}

ostream& operator<<(ostream& stream, const lbBasicGraph::Clique& c) {
  return lbJunctionTree::print(stream, c);
}

////////////////////////////////////////////////////////////////////
//Static variables:
////////////////////////////////////////////////////////////////////
const lbJunctionTree::JTmethodMap::value_type lbJunctionTree::JT_METHOD_PAIRS[] = {
  lbJunctionTree::JTmethodMap::value_type("Random",      lbJunctionTree::eRandom),
  lbJunctionTree::JTmethodMap::value_type("MaxCard",     lbJunctionTree::eMaxCard),
  lbJunctionTree::JTmethodMap::value_type("MinSize",     lbJunctionTree::eMinSize),
  lbJunctionTree::JTmethodMap::value_type("MinFill",     lbJunctionTree::eMinFill),
  lbJunctionTree::JTmethodMap::value_type("MinWeight",   lbJunctionTree::eMinWeight),
  lbJunctionTree::JTmethodMap::value_type("Sequential",         lbJunctionTree::eSeq),
  lbJunctionTree::JTmethodMap::value_type("ReverseSequential", lbJunctionTree::eSeqReverse),
  lbJunctionTree::JTmethodMap::value_type("BranchBound", lbJunctionTree::eBranchBound),
  lbJunctionTree::JTmethodMap::value_type("Best",        lbJunctionTree::eBest)
};

const lbJunctionTree::JTmethodMap lbJunctionTree::JT_METHOD_MAPPING
(&lbJunctionTree::JT_METHOD_PAIRS[0],
 &lbJunctionTree::JT_METHOD_PAIRS[lbJunctionTree::eDUMMY_METHOD]);
