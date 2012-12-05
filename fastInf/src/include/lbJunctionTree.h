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

#ifndef _LB_JUNCTION_TREE__
#define _LB_JUNCTION_TREE__

#include <stdlib.h>

#include <set>
#include <map>
#include <list>
#include <algorithm>
#include <string>
#include <utility>
using namespace std;

#include <lbBasicGraph.h>
#include <lbMathFunctions.h>
using namespace lbLib;

namespace lbLib {

  typedef long double realVal;

  class lbGraphStruct;
  class lbModel;

  class lbJunctionTree {
  public:
  
    enum JTmethod{eRandom = 0, eMaxCard, eMinSize, eMinFill, eMinWeight, eSeq, eSeqReverse, eBranchBound, eBest, eDUMMY_METHOD};

    typedef map<string,JTmethod> JTmethodMap;
    const static JTmethodMap::value_type JT_METHOD_PAIRS[];
    const static JTmethodMap JT_METHOD_MAPPING;

    static lbModel* CalcJunctionTreeGraphicalModel(const lbModel* model, JTmethod JTmeth = eBest);

  protected:
    static lbBasicGraph* CreatelbBasicGraphFromModel(const lbModel* model);
    static pair<realVal, lbBasicGraph::CliqueList*> CalcJunctionTreeCliques(const lbBasicGraph* g, JTmethod JTmeth);
    static lbModel* CreateModelBasedOnCliquesMeasures
    (lbBasicGraph::CliqueList* JTcliqs, lbBasicGraph::CliqueEdges* JTedges, const lbModel* oldModel);

  public:
    //For variable elimination:
    class EliminationHeuristic;
    typedef realVal (*calculateCliqueValue)(const lbBasicGraph::Clique* clique, const lbBasicGraph& g);

    /*If (upperBound != NULL), and during the triangulation of g, a clique c is formed s.t.
      upperBound->first(c) > upperBound->second  --> then pair(NULL, NULL) is returned. */
    static pair< lbBasicGraph::CliqueList*, list<int>* >
    GetTriangulatedCliquesUsingEliminationHeuristic(const lbBasicGraph& g, lbJunctionTree::EliminationHeuristic* heur,
						    pair<lbJunctionTree::calculateCliqueValue, realVal>*
						    upperBound = NULL);
  
    static inline realVal CalculateSizeOfClique(const lbBasicGraph::Clique* clique, const lbBasicGraph& g) {
      return clique->size();
    }
  
    //calculates log of state space size of clique in g:
    static realVal CalculateLogWeightOfClique(const lbBasicGraph::Clique* clique, const lbBasicGraph& g);
    static realVal CalculateFillEdgesOfClique(const lbBasicGraph::Clique* clique, const lbBasicGraph& g);
  
    //calculates max, over all cliques given, of: {CalculateValueOfClique(clique,g)}
    template<lbJunctionTree::calculateCliqueValue CalculateValueOfClique>
    static realVal CalculateMaxValueOfCliques(const lbBasicGraph::CliqueList* cliques, const lbBasicGraph* g);

    //calculates sum, over all cliques given, of: {CalculateValueOfClique(clique,g)}
    template<lbJunctionTree::calculateCliqueValue CalculateValueOfClique>
    static realVal CalculateLogSumLogValuesOfCliques(const lbBasicGraph::CliqueList* cliques, const lbBasicGraph* g);

    static void DiscardNonMaximalCliques(lbBasicGraph::CliqueList* cliques);
    static void TriangulateBasedOnCliques(lbBasicGraph& g, const lbBasicGraph::CliqueList* cliques);

    static bool isTriangulated(const lbBasicGraph& g);

    //note that CliqueEdges are asymmetric [i.e. contain edge (c1,c2), and not (c2,c1), for example]:
    static lbBasicGraph::CliqueEdges* CreateJunctionTree(const lbBasicGraph::CliqueList* cliques);

    static bool satisfiesJunctionTreeProperty(const lbBasicGraph::CliqueList* cliques,
					      const lbBasicGraph::CliqueEdges* JTedges);

    //returns true iff graph is 1 single connected component:
    static bool isConnected(const lbBasicGraph& g);

    typedef lbBasicGraph::CliqueList ConnectedComponents;
    static ConnectedComponents* getConnectedComponents(const lbBasicGraph& g);
  
    class EliminationHeuristic {
    public:
      virtual ~EliminationHeuristic() {}
      virtual int chooseNextVertexToEliminate(const lbBasicGraph& eliminatedSoFar) = 0;

      class LessThan {
      public:
	static const realVal worst;
	inline bool operator() (realVal r1, realVal r2) {
	  return r1 < r2;
	}
      };
    
      class GreaterThan {
      public:
	static const realVal worst;
	inline bool operator() (realVal r1, realVal r2) {
	  return r2 < r1;
	}
      };
    
    protected:
      typedef realVal (*calculateEliminationValue)(int vertToEliminate, const lbBasicGraph& g);

      static realVal CalculatePossibleCliqueSize(int vertToEliminate, const lbBasicGraph& g);
      static realVal CalculatePossibleCliqueFillEdges(int vertToEliminate, const lbBasicGraph& g);
      static realVal CalculatePossibleCliqueLogWeight(int vertToEliminate, const lbBasicGraph& g);    

      template<class Order>
      static int chooseNextVertexCheckEachElimination
      (const lbBasicGraph& eliminatedSoFar,
       lbJunctionTree::EliminationHeuristic::calculateEliminationValue criterion1,
       lbJunctionTree::EliminationHeuristic::calculateEliminationValue criterion2);
    };

    /*Performs an exhaustive search of random elimination orderings, using branch and bound,
      and an initial ordering produced by InitialHeuristic: */
    template <class InitialHeuristic, lbJunctionTree::calculateCliqueValue CalculateValueOfClique>
    class BranchAndBoundEliminationsHeuristic : public lbJunctionTree::EliminationHeuristic {
    public:
      BranchAndBoundEliminationsHeuristic(realVal maxOrderings);
      virtual ~BranchAndBoundEliminationsHeuristic();
      int chooseNextVertexToEliminate(const lbBasicGraph& eliminatedSoFar);

    private:
      list<int>* _bestEliminationOrder;
      const lbBasicGraph* _originallbBasicGraph;
      const realVal _MAX_ORDERINGS;
     
      void calculateEliminationOrder();
      void clearDataMembers();
    };

    class SpecificOrderHeuristic : public lbJunctionTree::EliminationHeuristic {
    public:
      SpecificOrderHeuristic(vector<int>* eliminationOrder, const lbBasicGraph* originallbBasicGraph);
      virtual ~SpecificOrderHeuristic() {}
      int chooseNextVertexToEliminate(const lbBasicGraph& eliminatedSoFar);

    private:
      vector<int>::iterator _position;
      const lbBasicGraph* _originallbBasicGraph;
    };

    class RandomHeuristic : public lbJunctionTree::EliminationHeuristic {
    public:
      RandomHeuristic() {}
      virtual ~RandomHeuristic() {}
      int chooseNextVertexToEliminate(const lbBasicGraph& eliminatedSoFar);
    };

    //implements MCS-M of Anne Berry, et. al - Algorithmica 2004 39: 287-298
    class MaxCardinalityHeuristic : public lbJunctionTree::EliminationHeuristic {
    public:
      MaxCardinalityHeuristic();
      virtual ~MaxCardinalityHeuristic();    
      int chooseNextVertexToEliminate(const lbBasicGraph& eliminatedSoFar);

    private:
      list<int>* _eliminationOrderStack;
      const lbBasicGraph* _originallbBasicGraph;
    
      void calculateEliminationOrder();
      void clearDataMembers();
    };

    /*Implements greedy min size heuristic as defined by Kjaerulff
      "Triangulation of graphs -- algorithms giving small total state space",
      Univ. Aalborg tech report, 1990 [breaks tie using min fill criterion] */
    class MinSizeHeuristic : public lbJunctionTree::EliminationHeuristic {
    public:
      virtual ~MinSizeHeuristic() {}
      int chooseNextVertexToEliminate(const lbBasicGraph& eliminatedSoFar);
    };

    /*Implements greedy min fill heuristic as defined by Kjaerulff
      "Triangulation of graphs -- algorithms giving small total state space",
      Univ. Aalborg tech report, 1990 [breaks tie using min size criterion] */
    class MinFillHeuristic : public lbJunctionTree::EliminationHeuristic {
    public:
      virtual ~MinFillHeuristic() {}
      int chooseNextVertexToEliminate(const lbBasicGraph& eliminatedSoFar);
    };

    /*Implements greedy min weight heuristic as defined by Kjaerulff
      "Triangulation of graphs -- algorithms giving small total state space",
      Univ. Aalborg tech report, 1990 [breaks tie using min fill criterion] */
    class MinWeightHeuristic : public lbJunctionTree::EliminationHeuristic {
    public:
      virtual ~MinWeightHeuristic() {}
      int chooseNextVertexToEliminate(const lbBasicGraph& eliminatedSoFar);
    };

    //eliminates vertices in graph in sequential order (or reverse)
    class SequentialOrderHeuristic : public lbJunctionTree::EliminationHeuristic {
    public:
      SequentialOrderHeuristic(bool reverseOrder);
      virtual ~SequentialOrderHeuristic() {}
      int chooseNextVertexToEliminate(const lbBasicGraph& eliminatedSoFar);

    private:
      bool _reverseOrder;
    };

    static ostream& print(ostream& stream, const lbBasicGraph::Clique& c, char delim = ' ');
  };

}

//////////////////////////////////////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& stream, const lbBasicGraph::Clique& c);

////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Order>
int lbJunctionTree::EliminationHeuristic::chooseNextVertexCheckEachElimination
(const lbBasicGraph& eliminatedSoFar, lbJunctionTree::EliminationHeuristic::calculateEliminationValue criterion1,
 lbJunctionTree::EliminationHeuristic::calculateEliminationValue criterion2) {  
  Order ord;
  int argOptVertex = -1;
  realVal optSize1 = Order::worst;
  realVal size2ofArgOptVertex = Order::worst;
  for (lbBasicGraph::vertexIterator vi = eliminatedSoFar.verticesBegin(); vi !=eliminatedSoFar.verticesEnd(); ++vi) {
    realVal size1 = (*criterion1)(*vi,eliminatedSoFar);
    if (ord(size1,optSize1)) {
      optSize1 = size1;
      argOptVertex = *vi;
      size2ofArgOptVertex = (*criterion2)(*vi,eliminatedSoFar);
    }
    else if(!ord(optSize1,size1)) {
      /*i.e. [size1 !< optSize1 && optSize1 !< size1] <==> optSize1 == size1,
	so use 2nd criterion for case of tie: */
      realVal size2 = (*criterion2)(*vi,eliminatedSoFar);
      if (ord(size2,size2ofArgOptVertex)) {
	optSize1 = size1;
	argOptVertex = *vi;
	size2ofArgOptVertex = size2;
      }
    }
  }
  return argOptVertex;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
template <class InitialHeuristic, lbJunctionTree::calculateCliqueValue CalculateValueOfClique>
lbJunctionTree::BranchAndBoundEliminationsHeuristic
<InitialHeuristic,CalculateValueOfClique>::BranchAndBoundEliminationsHeuristic(realVal maxOrderings)
  : _bestEliminationOrder(NULL), _originallbBasicGraph(NULL), _MAX_ORDERINGS(maxOrderings) {}

template <class InitialHeuristic, lbJunctionTree::calculateCliqueValue CalculateValueOfClique>
lbJunctionTree::BranchAndBoundEliminationsHeuristic
<InitialHeuristic,CalculateValueOfClique>::~BranchAndBoundEliminationsHeuristic() {
  clearDataMembers();
}

template <class InitialHeuristic, lbJunctionTree::calculateCliqueValue CalculateValueOfClique>
void lbJunctionTree::BranchAndBoundEliminationsHeuristic
<InitialHeuristic,CalculateValueOfClique>::clearDataMembers() {
  if (_bestEliminationOrder != NULL) {
    delete _bestEliminationOrder;
  }
  _bestEliminationOrder = NULL;
  _originallbBasicGraph = NULL;
}

template <class InitialHeuristic, lbJunctionTree::calculateCliqueValue CalculateValueOfClique>
int lbJunctionTree::BranchAndBoundEliminationsHeuristic
<InitialHeuristic,CalculateValueOfClique>::chooseNextVertexToEliminate
(const lbBasicGraph& eliminatedSoFar) {
  if (_bestEliminationOrder != NULL && &eliminatedSoFar != _originallbBasicGraph) {
    clearDataMembers();
    throw new string("Error in BranchAndBoundEliminationsHeuristic: lbBasicGraphs not the same!");
  }
  if (_bestEliminationOrder == NULL) {
    _originallbBasicGraph = &eliminatedSoFar;
    calculateEliminationOrder();
  }  
  if (_bestEliminationOrder == NULL || _bestEliminationOrder->empty()) {//empty graph - error
    clearDataMembers();
    return -1;
  }
  
  int nextToElim = _bestEliminationOrder->front();
  _bestEliminationOrder->pop_front();  
  if (_bestEliminationOrder->empty()) {//finished using elimination order
    clearDataMembers();    
  }
  return nextToElim;
}

template <class InitialHeuristic, lbJunctionTree::calculateCliqueValue CalculateValueOfClique>
void lbJunctionTree::BranchAndBoundEliminationsHeuristic
<InitialHeuristic,CalculateValueOfClique>::calculateEliminationOrder() {
  //find value produced by InitialHeuristic:
  lbJunctionTree::EliminationHeuristic* heur = new InitialHeuristic();
  pair<lbBasicGraph::CliqueList*, list<int>*> triangRes =
    lbJunctionTree::GetTriangulatedCliquesUsingEliminationHeuristic(*_originallbBasicGraph, heur);
  delete heur;
  lbBasicGraph::CliqueList* startCliques = triangRes.first;
  _bestEliminationOrder = triangRes.second;
  realVal bestEliminationValue =
    lbJunctionTree::CalculateLogSumLogValuesOfCliques<CalculateValueOfClique>(startCliques,_originallbBasicGraph);

  for (lbBasicGraph::CliqueList::iterator cliqIt = startCliques->begin(); cliqIt != startCliques->end(); ++cliqIt) {
    delete *cliqIt;
  }
  delete startCliques;  
     
  vector<int>* newEliminationOrder =
    new vector<int>(_bestEliminationOrder->begin(),_bestEliminationOrder->end());  
  for (realVal count = 0; count < _MAX_ORDERINGS; ++count) {
    random_shuffle(newEliminationOrder->begin(), newEliminationOrder->end());    
    lbJunctionTree::EliminationHeuristic* newElimHeur =
      new lbJunctionTree::SpecificOrderHeuristic(newEliminationOrder,_originallbBasicGraph);
    lbBasicGraph* tmplbBasicGraph = new lbBasicGraph(*_originallbBasicGraph);
    pair<lbJunctionTree::calculateCliqueValue, realVal>
      boundingValue(CalculateValueOfClique,bestEliminationValue);
    pair<lbBasicGraph::CliqueList*, list<int>*> triangRes =
      GetTriangulatedCliquesUsingEliminationHeuristic(*tmplbBasicGraph,newElimHeur,&boundingValue);
    lbBasicGraph::CliqueList* newEliminationCliques = triangRes.first;
    delete newElimHeur;
    delete tmplbBasicGraph;
    if (newEliminationCliques == NULL) {
      //the current order exceeded the current bounding value of bestEliminationValue
      continue;
    }
    delete triangRes.second;
    
    realVal newEliminationValue =
      lbJunctionTree::CalculateLogSumLogValuesOfCliques<CalculateValueOfClique>
      (newEliminationCliques,_originallbBasicGraph);
    
    for (lbBasicGraph::CliqueList::iterator cliqIt = newEliminationCliques->begin();
	 cliqIt != newEliminationCliques->end(); ++cliqIt) {      
      delete *cliqIt;
    }
    delete newEliminationCliques;    
    
    if (newEliminationValue < bestEliminationValue) {
      delete _bestEliminationOrder;
      _bestEliminationOrder = new list<int>(newEliminationOrder->begin(),newEliminationOrder->end());
      bestEliminationValue = newEliminationValue;
    }
  }  
  delete newEliminationOrder;  
}

////////////////////////////////////////////////////////////////////////////////
template<lbJunctionTree::calculateCliqueValue CalculateValueOfClique>
realVal lbJunctionTree::CalculateMaxValueOfCliques(const lbBasicGraph::CliqueList* cliques, const lbBasicGraph* g) {
  lbJunctionTree::EliminationHeuristic::GreaterThan gt;
  realVal maxValue = lbJunctionTree::EliminationHeuristic::GreaterThan::worst;
  for (lbBasicGraph::CliqueList::const_iterator cliqIt = cliques->begin(); cliqIt != cliques->end(); ++cliqIt) {
    const lbBasicGraph::Clique* nextClique = *cliqIt;
    realVal value = CalculateValueOfClique(nextClique,*g);
    if (gt(value,maxValue)) {
      maxValue = value;
    }
  }
  return maxValue;
}

template<lbJunctionTree::calculateCliqueValue CalculateLogValueOfClique>
realVal lbJunctionTree::CalculateLogSumLogValuesOfCliques(const lbBasicGraph::CliqueList* cliques, const lbBasicGraph* g) {
  realVal logSumValue = -HUGE_VAL;
  
  for (lbBasicGraph::CliqueList::const_iterator cliqIt = cliques->begin(); cliqIt != cliques->end(); ++cliqIt) {
    const lbBasicGraph::Clique* nextClique = *cliqIt;
    realVal logValue = CalculateLogValueOfClique(nextClique,*g);
    logSumValue = lbAddLog(logSumValue, logValue);
  }

  return logSumValue;
}
//////////////////////////////////////////////////////////////////////////////////


#endif
