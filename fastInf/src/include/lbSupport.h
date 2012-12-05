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

// lbSupport.h
// Author: Ian McGraw

#ifndef _LB_SUPPORT_
#define _LB_SUPPORT_

#include <lbDefinitions.h>
#include <lbRegionGraph.h>
#include <lbGraphStruct.h>
#include <lbSubgraph.h>
#include <lbRandomProb.h>
#include <lbWeightedArcs.h>

using namespace std; 

#define VERY_VERBOSE 0

/*
 * A class used to generate and keep track of a set of
 * spanning trees for a graph.
 */
namespace lbLib {

  class lbSupport {
  public:
    lbSupport (lbGraphBase const* graph, 
	       lbGraphStruct const* clusterGraph, 
	       varIndicesVecVec const& scopes);

    explicit lbSupport(lbSupport const&);
    virtual ~lbSupport();

    inline int getNumForests() const;
    inline lbSubgraph const& getForest(int i) const;
    
    inline void print(ostream & out) const;
    static inline void setVerbose(bool verbose) { _verbose = verbose; }
    static inline void setWeightedArcs(lbWeightedArcs & wa) { _weightedArcs = &wa; }
    static inline void setWeighted(bool weighted) { _weighted = weighted; }
    static inline void setPartiallyWeighted(bool pw) { _partiallyWeighted = pw; }
    static inline bool getWeighted() { return _weighted; }
    static inline void setSupportSize(int size) { _supportSize = size; }
    static inline void setStochastic(bool stochastic) { _stochastic = stochastic; }
    static inline void setMinTreeSize(int minTreeSize) { _minTreeSize = minTreeSize; }
    static inline void setMaxTreeSize(int maxTreeSize) { _maxTreeSize = maxTreeSize; }
    static inline void setWeightThreshold(double threshold) { _weightThreshold = threshold; }
    static inline void setCoveragePercentage(double percent) { _coveragePercentage = percent; }

    static inline bool getStochastic() { return _stochastic; }
    static inline bool getVerbose() { return _verbose; }
  private:
    inline bool requirementsMet();

    inline bool normalizeWeights(vector<double> & weights);
    inline void generateSupport();
    inline void generateForestSatisfyingRIP(lbSubgraph & spanning);
    inline bool tryInsertingArc(arcIndex ai, lbSubgraph & forest, vector<arcIndex> & fringe);
    inline bool violatesRIP(arcIndex ai, nodeIndex newNode, lbSubgraph const& forest);
    inline void updateFringe(nodeIndex ni, lbSubgraph const&forest, vector<arcIndex> &fringe);
    inline arcIndex findHeaviestArcIndex();
    inline arcIndex popHeaviestFringeIndex(vector<arcIndex> & fringe, bool force);
    inline arcIndex sampleWeightedFringeIndex(vector<arcIndex> & fringe, bool force);
    
    static bool _verbose;
    static int _supportSize;
    static bool _stochastic;
    static int _minTreeSize;
    static lbWeightedArcs *_weightedArcs;
    static int _maxTreeSize;
    static double _coveragePercentage;
    static double _weightThreshold;
    static bool _weighted;
    static bool _partiallyWeighted;

    const lbGraphBase * _graph;
    const lbGraphStruct *_clusterGraph;
    vector<lbSubgraph> _spanningForests;
    varIndicesVecVec const *_scopes;
  };


  inline int lbSupport::getNumForests() const {
    return (int) _spanningForests.size();
  }
  
  inline lbSubgraph const& lbSupport::getForest(int i) const {
    assert (i >= 0 && (uint) i < _spanningForests.size());
    return _spanningForests[i];
  }
  
  inline bool lbSupport::requirementsMet() {
    if ((int) _spanningForests.size() >= _supportSize) {
      return true;
    }
    return false;
  }

  inline void lbSupport::generateSupport() {
    assert (_spanningForests.empty());
    
    while (!requirementsMet()) {
      lbSubgraph spanning(_graph);
      generateForestSatisfyingRIP(spanning);
    
      double total = 0;
      arcIndexVec aiv = spanning.getArcIndices();
      
      if (_weightedArcs->size() > 0) {
	for (uint i = 0; i < aiv.size(); i++) {
	  total += _weightedArcs->getWeight(aiv[i]);
	}
	
	if (_verbose) {
	  cerr << (!_stochastic ? "Adding stochastic " : "Adding deterministic ");
	  cerr << "tree with " << aiv.size();
	  cerr << " edges with total weight " << total << endl;
	}
      }
	
      _spanningForests.push_back(spanning);
    }
  }
  
  inline int sample(vector<double> const& distribution) {
    
    vector<double> cumulative(distribution.size());
    
    double total = 0.0;

    for (uint i = 0; i < distribution.size(); i++) {
      total += distribution[i];
      cumulative[i] = total;
    }
    
    if (cumulative.back() <= .99999999999 || cumulative.back() >= 1.000000000001) {
      cumulative.clear();
      
      for (uint i = 0; i < distribution.size(); i++) {
	cumulative.push_back((i+1)/distribution.size());
      }
    }

    assert (cumulative.back() >= .99999999999 && cumulative.back() <= 1.000000000001);

    double randomNum = _lbRandomProbGenerator.RandomDouble(1.0);
    assert (randomNum >= 0 && randomNum <= 1.0);

    uint index = 0;
    for (index = 0; index < cumulative.size(); index++) {
      if (cumulative[index] > randomNum) {
	break;
      }      
    }

    return (int) index;
  }

  inline void lbSupport::generateForestSatisfyingRIP(lbSubgraph & forest) {
    arcIndexVec fringe;
    
    if (_maxTreeSize <= 0) {
      return;
    }

    arcIndex highestIndex = -1;

    if (!_weighted) {
      assert (!_partiallyWeighted);
      highestIndex = (int) (_graph->getNumArcs()*_lbRandomProbGenerator.RandomDouble(.999999));
    }
    else {
      highestIndex = _weightedArcs->getMaxArcIndex();
    }

    bool worked = tryInsertingArc(highestIndex, forest, fringe);
    assert(worked);

    if (_maxTreeSize <= 1) {
      return;
    }

    updateFringe(_graph->getArc(highestIndex).first, forest, fringe);
    updateFringe(_graph->getArc(highestIndex).second, forest, fringe);

    while(!fringe.empty()) {
      arcIndex ai;

      bool forced = forest.getNumArcs() < _minTreeSize;
      if (_stochastic) {
	ai = sampleWeightedFringeIndex(fringe, forced);
      }
      else {
	ai = popHeaviestFringeIndex(fringe, forced);
      }

      if (ai == -1) {
	return;
      }

      if (forest.getNumNodes() >= _maxTreeSize) {
	if (_verbose) {
	  cerr << "Returning early. Max " << _maxTreeSize << " reached." << endl;
	}
	return;
      }

      arc a = _graph->getArc(ai);
      nodeIndex newNode = forest.nodePresent(a.first) ? a.second : a.first;

      if (tryInsertingArc(ai, forest, fringe)) {
	updateFringe(newNode, forest, fringe);
      }
    }
  }

  inline arcIndex lbSupport::popHeaviestFringeIndex(vector<arcIndex> & fringe, bool force) {
    double highestWeight = -HUGE_VAL;
    arcIndex highestIndex = 0;
    int toRemove = -1;

    assert (!fringe.empty());

    bool found = false;

    for (uint i = 0; i < fringe.size(); i++) {
      arcIndex ai = fringe[i];

      if (!_weighted || _partiallyWeighted) {
	found = true;
	highestIndex = fringe[0]; // Just take the first edge
	toRemove = 0;  
	break;
      }

      if (_weightedArcs->getWeight(ai) > highestWeight && (_weightedArcs->getWeight(ai) >= _weightThreshold || force)) {
	found = true;
	highestWeight = _weightedArcs->getWeight(ai);
	highestIndex = ai;
	toRemove = i;
      }
    }

    if (!found && !force) {
      return -1;
    }

    assert (toRemove != -1);
    fringe.erase(fringe.begin() + toRemove);
    return highestIndex;
  }


  inline arcIndex lbSupport::sampleWeightedFringeIndex(vector<arcIndex> & fringe, bool force) {
    assert (!fringe.empty());
    vector<double> fringeWeights;

    bool found = false;

    for (uint i = 0; i < fringe.size(); i++) {
      arcIndex ai = fringe[i];

      if (!_weighted || _partiallyWeighted) {
	fringeWeights.push_back(1.0);
	found = true;
      }
      else {
	fringeWeights.push_back(_weightedArcs->getWeight(ai));
	if (_weightedArcs->getWeight(ai) >= _weightThreshold) {
	  found = true;
	}
      }
    }

    if (!force && !found) {
      if (_verbose) {
	cerr << "Not forced and not found.";
      }
      return -1;
    }

    bool worked = normalizeWeights(fringeWeights);
   
    if (!force && !worked) {
      cerr << "Had trouble normalizing, and minimum reached, so we're giving up now." << endl;
      return -1;
    }

    int index = sample(fringeWeights);

    arcIndex highestIndex = fringe[index];
    fringe.erase(fringe.begin() + index);
    return highestIndex;
  }

  inline void lbSupport::updateFringe(nodeIndex ni, lbSubgraph const& forest, vector<arcIndex> &fringe) {
    arcIndexVec aiv = _graph->getNeighboringArcIndices(ni);

    for (uint i = 0; i < aiv.size(); i++) {
      arc a = _graph->getArc(aiv[i]);

      if (!forest.arcPresent(aiv[i])) {
	if (!forest.nodePresent(a.first) || !forest.nodePresent(a.second)) {
	  nodeIndex newNode = (a.first == ni) ? a.second : a.first;
	  if (!violatesRIP(aiv[i], newNode, forest)) {
	    fringe.push_back(aiv[i]);
	  }
	}
      }
    }
  }

  inline bool lbSupport::tryInsertingArc(arcIndex ai, lbSubgraph & forest, vector<arcIndex> & fringe) {
    assert (!forest.arcPresent(ai));
    nodeIndex first = _graph->getArc(ai).first;
    nodeIndex second = _graph->getArc(ai).second;
    assert (forest.nodePresent(first) || forest.nodePresent(second) || forest.isEmpty());
    
    if (forest.nodePresent(first) && forest.nodePresent(second)) {
      return false;
    }

    nodeIndex newNode = forest.nodePresent(first) ? second : first;

    if (violatesRIP(ai, newNode, forest)) {
      return false;
    }
		     
    forest.addArc(ai);
    forest.addNode(first);
    forest.addNode(second);

    varIndicesVecVec const& scopes = *_scopes;
    varsVec const& arcScope = scopes[ai];
    for (uint j = 0; j < arcScope.size(); j++) {
      forest.addVar(arcScope[j]);
    }
    
    return true;
  }

  inline bool lbSupport::violatesRIP(arcIndex ai, nodeIndex newNode, lbSubgraph const& subgraph) {
    varsVec const& vars = _clusterGraph->getVarsVecForClique((cliqIndex) newNode);
    varIndicesVecVec const& scopes = *_scopes;
    
    for (uint i = 0; i < vars.size(); i++) {
      if ( subgraph.varPresent(vars[i]) ) {
	// Scope of ai must contain the variable for RIP to hold
	assert (ai >= 0 && ai < (int) scopes.size());
	varsVec const& arcScope = scopes[ai];
	
	bool found = false;
	for (uint j = 0; j < arcScope.size(); j++) {
	  if (arcScope[j] == vars[i]) {
	    found = true;
	    break;
	  }
	}
	
	if (!found) {
	  return true;
	}
      }
    }

    return false;
  }

  inline bool lbSupport::normalizeWeights(vector<double> & weights) {
    double total = 0;
    
    assert (weights.size() > 0);
	uint i;
    for (i = 0; i < weights.size(); i++) {
      if (weights[i] < 0) {
	weights[i] = 0;
      }

      total += weights[i];
    }

    if (total == 0) {
      return false;
    }

    for (i = 0; i < weights.size(); i++) {
      weights[i] /= total;
    }

    return true;
  }

  inline void lbSupport::print(ostream & out) const {
    out << "Support: " << endl;
    out << "Num Forests: " << getNumForests() << endl << endl;

    for (uint i = 0; i < _spanningForests.size(); i++) {
      out << "Forest " << i << ": " << endl;
      
      arcVec ordering;
      bool worked =_spanningForests[i].getRootedArcs(ordering);
      assert (worked);

      for (uint j = 0; j < ordering.size(); j++) {
	arc a = ordering[j];
	out << a.first << " --> " << a.second << endl;
      }
      out << endl;
    }
  }
};

#endif
