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

// lbRegionGraph.h
// Author: Ian McGraw

/*
 * Here I define the region graph which exports functionality particularly useful
 * for region based inference algorithms.  The actual storage of the regions will 
 * be in the lbRegionModel.  This class stores the counting numbers and ensures
 * the validity of the graph.
 *
 * Any graph editting beyond the constructors will require the client to explicitly
 * recompute update rules and/or counting numbers.  That is I don't override graph
 * editting to ensure that these remain up-to-date.
 */

#ifndef _LB_REGION_GRAPH_H_
#define _LB_REGION_GRAPH_H_

#include <lbDAG.h>
#include <lbModel.h>

namespace lbLib {

  typedef measIndicesVec facIndicesVec;
  struct lbRegion;
  typedef vector<lbRegion> lbRegionVec;
  typedef varsVecVector varIndicesVecVec;
  typedef vector<facIndicesVec> facIndicesVecVec;

  implementIntegerStructVectorOps(facIndicesVec);



  class lbRegionGraph : public lbDAG {

  public:
    /*
     * Constructors and Destructors
     */
    lbRegionGraph(); /* Empty region graph */
    lbRegionGraph(lbRegionVec const& rgns, arcVec const& arcVec);
    lbRegionGraph(varIndicesVecVec const& vivv, 
		  facIndicesVecVec const& fivv, 
		  arcVec const& arcVec);
    
    explicit lbRegionGraph(lbRegionGraph const&);
    ~lbRegionGraph();
 
    /*
     * Mutators
     */
    inline virtual void readGraph(istream & in);
    inline virtual void addRegion(lbRegion const& rgn);
    inline virtual void setRegion(nodeIndex ni, lbRegion const& rgn);
    inline virtual void setClusterRegions(varIndicesVecVec const& vivv, facIndicesVecVec const& fivv, bool removeSubregions = true);
    inline virtual void setClusterTwoLayerRegions(varIndicesVecVec const& vivv, facIndicesVecVec const& fivv);
    inline virtual void setBetheRegions(varIndicesVecVec const& vivv);
    inline virtual void removeNode(nodeIndex ni);
    inline virtual void clean(bool killUselessRegions = false);
    inline virtual void setCountingNumsFile (string const& fileName);

    /*
     * Computations
     */
    inline virtual vector<double> computeCountingNums() const;
    inline virtual vector<double> computePowerNums() const;
    inline virtual vector<double> computeBetheCountingNums() const ;
    inline virtual pair<arcIndexVecVec, arcIndexVecVec> computeUpdateRules() const;

    /*
     * Properties
     */
    inline virtual bool isEmpty() const;
    inline virtual bool varIndicesCountCorrectly(vector<double> countingNumVec) const;
    inline virtual bool facIndicesCountCorrectly(vector<double> countingNumVec) const;
    inline virtual measIndex getHighestFacIndex() const;
    inline virtual rVarIndex getHighestVarIndex() const;
    inline virtual bool countingNumsSumToOne() const;

    /*
     * Accessors
     */
    inline virtual lbRegion const& getRegion(nodeIndex ni) const;
    inline virtual const string & getCountingNumsFile() const;

    /*
     * Print
     */
    inline virtual void print(ostream & out) const;
    inline virtual void printFormatted(ostream & out) const;

    /*
     * Debugging
     */

    /*
     * For region graphs the word valid takes on a special meaning.
     * We must make sure that the counting numbers satisfy the condition
     * that for a node n with ancestors A(n), the counting number,
     * c_n = 1 - ( sum_{u \in A(n)} c(u) ).  Also the graph must be
     * a DAG.  This should be true by construction (when we recompute
     * the counting nums).  We test more interesting region properties
     * in the lbRegionModel.
     */
    inline virtual bool isValid() const;

  protected:
    inline void computeNumeratorRule(arcIndexVecVec & numerRules, arcIndex ai) const;
    inline void computeDenominatorRule(arcIndexVecVec & denomRules, arcIndex ai) const;
    inline bool containsIndex(nodeIndexVec niv, nodeIndex ni) const;

    /*
     * Methods to help with Cluster region graph creation.
     */
    inline void addClusterGenerationArcs(nodeIndex prevGenBegin, nodeIndex prevGenEnd,
					 nodeIndex nextGenBegin, nodeIndex nextGenEnd);
    inline void installFirstClusterGeneration(varIndicesVecVec const& genvivv, 
					      facIndicesVecVec const& genfivv);
    inline void installRemainingClusterGenerations(bool removeSubregions);
    inline bool installSingleClusterGeneration(nodeIndex & genBegin, nodeIndex & genEnd, bool removeSubregions);
    inline bool ensureValidClusterGeneration(lbRegionVec & rgns, bool removeSubregions) const;

    inline void installSingleVariableClusterGeneration(varIndicesVecVec const& vivv);

    /*
     * Cleaning supplies
     */
    inline void removeRedundantArcs();
    inline void removeRedundantNodes(bool killUselessRegions);
    inline void mergeAllEqualRegions();
    inline void removeAllUselessRegions();
    inline void mergeEqualRegions(nodeIndex mergeTo, nodeIndex mergeFrom);
    inline void removeUselessRegions(nodeIndex ni);

    /*
     * Reading helpers
     */
    const static string REGIONS_DELIM;
    inline void readRegions(istream & in);

    lbRegionVec _regions;

    string _countingNumsFile;
  };




  /*
   * lbRegion is the class that is represents the region assigned
   * to a particular node of the region graph. Implemented in .cpp file
   */
  struct lbRegion {
    lbRegion();
    lbRegion(varsVec vars, facIndicesVec mes);

    /*
     * Mutators
     */
    void readRegion(istream & in);
    void readVarIndices(istream & in);
    void readFacIndices(istream & in);

    /*
     * Properties
     */
    bool isSubsetOf(lbRegion const& rgn , bool withFactors=true) const;
    bool isEqual(lbRegion const& rgn) const;
    bool isEmpty() const;

    /*
     * Accessors
     */
    lbRegion getIntersection(lbRegion const& rgn) const;

    /*
     * Print
     */
    void print(ostream & out) const;
    void printFormatted(ostream & out) const;

    varsVec _varIndices;
    facIndicesVec _facIndices;
  };

  inline void lbRegionGraph::readGraph(istream & in) {
    assert(getNumNodes() == 0);
    lbDAG::readGraph(in);
    swallowComments(in);
    readRegions(in);
    assert(isValid());
  }



  inline void lbRegionGraph::readRegions(istream & in) {
    char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);

    in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE);
    assert(string(buffer.get()) == REGIONS_DELIM);

    while (in.peek() != '@') {
      lbRegion rgn;
      rgn.readRegion(in);
      _regions.push_back(rgn);
      assert(in.good() && !in.eof());
    }

    in.getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE);
    string str(buffer.get());
    assert(str == END_DELIM);
  }

  inline void lbRegionGraph::addRegion(lbRegion const& rgn) {
    _regions.push_back(rgn);
  }

  inline void lbRegionGraph::setRegion(nodeIndex ni, lbRegion const& rgn) {
    assert(nodeExists(ni));
    assert(ni >= 0 && (uint) ni < _regions.size());
    _regions[ni] = rgn;
  }

  inline void lbRegionGraph::setClusterRegions(varIndicesVecVec const& vivv, facIndicesVecVec const& fivv, bool removeSubregions) {
    assert(isEmpty());
    installFirstClusterGeneration(vivv, fivv);
    installRemainingClusterGenerations(removeSubregions);
    assert(isValid());
  }

  inline void lbRegionGraph::setClusterTwoLayerRegions(varIndicesVecVec const& vivv, facIndicesVecVec const& fivv) {
    assert(isEmpty());
    installFirstClusterGeneration(vivv, fivv);
    installSingleVariableClusterGeneration(vivv);
    assert(isValid());
  }

  
  inline void lbRegionGraph::setBetheRegions(varIndicesVecVec const& vivv) {
    assert(isEmpty());
    assert(!vivv.empty() && !vivv[0].empty());

    rVarIndex highest = vivv[0][0];

    varsVec univars (vivv.size(),0) ;
    int univarNum = 0 ;

    uint i;
    for (i = 0; i < vivv.size(); i++) {
      for (uint j = 0; j < vivv[i].size(); j++) {
	if (highest < vivv[i][j]) {
	  highest = vivv[i][j];
	}
        if (vivv[i].size() == 1) {
          univars[i] = 1 ;
          univarNum++ ;
        }
      }
    }
    _regions.resize((int) highest + vivv.size() - univarNum + 1);

    for (i = 0; i < _regions.size(); i++) {
      addNode();
    }

    // Univars:
    for (i = 0; i <= (uint)highest; i++) {
      varsVec univarReg ;
      univarReg.push_back(i) ;
      facIndicesVec fiv;
      fiv.push_back(i);
      _regions[i] = lbRegion(univarReg, fiv);
    }
    
    // Other factors:
    for (i = 0; i < vivv.size(); i++) {
      if (!univars[i]) {
        nodeIndex fi = (int) highest + 1 + i - univarNum ;
        
        facIndicesVec fiv;
        fiv.push_back(i);
        //for (uint j=0 ; j<vivv[i].size() ; ++j) {
        //fiv.push_back((int)vivv[i][j]) ;          
        //}
        _regions[fi] = lbRegion(vivv[i], fiv);
        
        for (uint j = 0; j < vivv[i].size(); j++) {
          nodeIndex vi = (int) vivv[i][j];
          addArc(fi, vi);
        }
      }
    }

    assert(isValid());
  }
  

  inline void lbRegionGraph::removeNode(nodeIndex ni) {
    lbDAG::removeNode(ni);

    _regions.erase(_regions.begin() + ni);
  }

  inline void lbRegionGraph::setCountingNumsFile (string const& fileName) {
    _countingNumsFile = fileName ;
  }

  inline bool lbRegionGraph::containsIndex(nodeIndexVec niv, nodeIndex ni) const {
    for (uint i = 0; i < niv.size(); i++) {
      if (niv[i] == ni) {
	return true;
      }
    }
    return false;
  }

  inline void lbRegionGraph::computeNumeratorRule(arcIndexVecVec & numerRules, arcIndex curArc) const {
    arcIndexVec rule;

    nodeIndex P = getArc(curArc).first;
    nodeIndex R = getArc(curArc).second;

    nodeIndexVec EP = getDescendants(P, true);
    nodeIndexVec ER = getDescendants(R, true);

    for (uint j = 0; j < EP.size(); j++) {
      nodeIndex nj = EP[j];
      
      if (!containsIndex(ER, nj)) {
	arcIndexVec parentArcs = getIncomingArcIndices(nj);
	for (uint i = 0; i < parentArcs.size(); i++) {
	  nodeIndex ni = getArc(parentArcs[i]).first;
	  assert(nj == getArc(parentArcs[i]).second);
	  
	  if (!containsIndex(EP, ni)) {
	    rule.push_back(parentArcs[i]);
	  }
	}
      }
    }

    numerRules.push_back(rule);
  }

  inline void lbRegionGraph::computeDenominatorRule(arcIndexVecVec & denomRules, arcIndex curArc) const {
    arcIndexVec rule;

    nodeIndex P = getArc(curArc).first;
    nodeIndex R = getArc(curArc).second;

    nodeIndexVec EP = getDescendants(P, true);
    nodeIndexVec ER = getDescendants(R, true);

    for (uint i = 0; i < EP.size(); i++) {
      nodeIndex ni = EP[i];

      if (!containsIndex(ER, ni)) {
	arcIndexVec childArcs = getOutgoingArcIndices(ni);
	for (uint j = 0; j < childArcs.size(); j++) {
	  if (childArcs[j] != curArc) {
	    nodeIndex nj = getArc(childArcs[j]).second;
	    assert(getArc(childArcs[j]).first == ni);

	    if (containsIndex(ER, nj)) {
	      rule.push_back(childArcs[j]);
	    }
	  }
	}
      }
    }

    denomRules.push_back(rule);
  }

  /*
   * Returns <numerator rules, denominator rules>
   */
  inline pair<arcIndexVecVec, arcIndexVecVec> lbRegionGraph::computeUpdateRules() const {
    pair<arcIndexVecVec, arcIndexVecVec> updateRules;

    for (arcIndex ai = 0; ai < getNumArcs(); ai++) {
      computeNumeratorRule(updateRules.first, ai);
      computeDenominatorRule(updateRules.second, ai);
    }

    return updateRules;
  }

  inline vector<double> lbRegionGraph::computeCountingNums() const {
    vector<double> countingNumVec(_numNodes);

    if (getCountingNumsFile().empty()) {
      countingNumVec = computeBetheCountingNums() ;
    }
    else { // read counting numbers from a file
      ifstream countingNumsFstrm (getCountingNumsFile().c_str()) ;
      assert (countingNumsFstrm) ;
    
      vector<double> countNumsFromFile ;
      while (countingNumsFstrm.good()) {
        double curCountNum ;
        countingNumsFstrm >> curCountNum ;
        countNumsFromFile.push_back (curCountNum) ;
      }
      //cerr << "[DEBUG] Counting nums from file: " ;
      //printVector (countNumsFromFile , cerr) ;
      //cerr << "[DEBUG] Num of nodes: " << countingNumVec.size() << endl ;

      if (countNumsFromFile.size() != countingNumVec.size()) {
        cerr << "[ERROR] Expecting " << countingNumVec.size() << " counting numbers, got "
             << countNumsFromFile.size() << " in file " << getCountingNumsFile() << endl ;
        assert (false) ;
      }
      for (uint i=0 ; i<countingNumVec.size() ; ++i) {
        countingNumVec[i] = countNumsFromFile[i] ;
      }
    }

    return countingNumVec;
  }

  inline vector<double> lbRegionGraph::computePowerNums() const {
    vector<double> countingNums = computeCountingNums();
    vector<double> powerNums(countingNums.size());

    for (uint i = 0; i < powerNums.size(); i++) {
      nodeIndexVec parents = getIncomingNodeIndices(i);
      
      if (parents.size() == 0) {
	powerNums[i] = 1;
      }
      else {
	double qR = (1 - countingNums[i]) / parents.size();
	if (qR != 2.0) {
	  powerNums[i] = 1/(2.0 - qR);
	}
	else {
	  // Region graph needs to be rewired.
	  NOT_REACHED;
	}
      }
    }

    return powerNums;
  }

  inline vector<double> lbRegionGraph::computeBetheCountingNums() const {
    vector<double> countingNumVec(_numNodes);

    nodeIndexVec sortedNodes = getTopologicalNodeSort();
    
    for (uint i = 0; i < sortedNodes.size(); i++) {
      nodeIndexVec ancestors = getAncestors(sortedNodes[i]);
      double total = 0;
      for (uint j = 0; j < ancestors.size(); j++) {
        total += countingNumVec[ancestors[j]];
      }
      countingNumVec[sortedNodes[i]] = 1 - total;
    }

    return countingNumVec ;
  }

  inline bool lbRegionGraph::isEmpty() const { 

    return (lbDAG::isEmpty() && _regions.empty());
  }

  inline lbRegion const& lbRegionGraph::getRegion(nodeIndex ni) const {
    assert(nodeExists(ni));
    assert((uint) ni < _regions.size());
    return _regions[ni];
  }

  inline const string & lbRegionGraph::getCountingNumsFile() const {
    return _countingNumsFile ;
  }

  inline bool lbRegionGraph::varIndicesCountCorrectly(vector<double> countingNumVec) const {
    vector<double> varIndicesMask;
    
    assert(_regions.size() == (uint) getNumNodes());
    uint i;
    for (i= 0; i < _regions.size(); i++) {
      for (uint j = 0; j < _regions[i]._varIndices.size(); j++) {
	rVarIndex cur = _regions[i]._varIndices[j];

	if ((uint) cur >= varIndicesMask.size()) {
	  varIndicesMask.resize((int) cur + 1, 0);
	}

	varIndicesMask[cur] += countingNumVec[i];
      }
    }

    for (i = 0; i < varIndicesMask.size(); i++) {
      if (varIndicesMask[i] != 1) {
	return false;
      }
    }

    return true;
  }

  inline bool lbRegionGraph::facIndicesCountCorrectly(vector<double> countingNumVec) const {
    vector<double> facIndicesMask;

    assert(countingNumVec.size() == (uint) getNumNodes());
    assert(_regions.size() == (uint) getNumNodes());
uint i;
    for (i = 0; i < _regions.size(); i++) {
      for (uint j = 0; j < _regions[i]._facIndices.size(); j++) {
	measIndex cur = _regions[i]._facIndices[j];

	if ((uint) cur >= facIndicesMask.size()) {
	  facIndicesMask.resize((int) cur + 1, 0);
	}

	facIndicesMask[cur] += countingNumVec[i];
      }
    }

    for (i = 0; i < facIndicesMask.size(); i++) {
      if (facIndicesMask[i] != 1) {
	return false;
      }
    }

    return true;
  }

  inline measIndex lbRegionGraph::getHighestFacIndex() const {
   assert(_regions.size() > 0 && _regions[0]._facIndices.size() > 0);
        measIndex highest = _regions[0]._facIndices[0];
uint i;
    for (i = 0; i < _regions.size(); i++) {
      for (uint j = 0; j < _regions[i]._facIndices.size(); j++) {
	if (highest < _regions[i]._facIndices[j]) {
	  highest = _regions[i]._facIndices[j];
	}
      }
    }

    return highest;
  }

  inline rVarIndex lbRegionGraph::getHighestVarIndex() const {
    assert(_regions.size() > 0 && _regions[0]._varIndices.size() > 0);
    
    rVarIndex highest = _regions[0]._varIndices[0];
uint i;
    for (i = 0; i < _regions.size(); i++) {
      for (uint j = 0; j < _regions[i]._varIndices.size(); j++) {
	if (highest < _regions[i]._varIndices[j]) {
	  highest = _regions[i]._varIndices[j];
	}
      }
    }

    return highest;
  }

  inline bool lbRegionGraph::countingNumsSumToOne() const {
    double sum = 0; 
    
    vector<double> countingNumVec = computeCountingNums();
    for (uint i = 0; i < countingNumVec.size(); i++) {
      sum += countingNumVec[i];
    }

    return (sum == 1);
  }
  

  inline void lbRegionGraph::print(ostream & out) const {
    out << "Printing RegionGraph, thus... " << endl;
    lbDAG::print(out);

    vector<double> countingNumVec = computeCountingNums();
    out << "Counting nums: " << endl;
    uint i;
    for (i = 0; i < countingNumVec.size(); i++) {
      out << "c(" << i << ") = " << countingNumVec[i] << endl;
    }

    vector<double> powerNumVec = computePowerNums();
    out << "Power nums: " << endl;
    for (i = 0; i < powerNumVec.size(); i++) {
      out << "p(" << i << ") = " << powerNumVec[i] << endl;
    }

    pair<arcIndexVecVec, arcIndexVecVec> rules;
    rules = computeUpdateRules();
    
    arcIndexVecVec const& updateNumerRules = rules.first;
    arcIndexVecVec const& updateDenomRules = rules.second;

    out << endl;
    assert(updateNumerRules.size() == updateDenomRules.size());
    for (i = 0; i < updateDenomRules.size(); i++) {
      out << "Rules for arc " << i << "... " << endl;
     
      out << "Numerator: ";
	  uint  j;
      for (j = 0; j < updateNumerRules[i].size(); j++) {
	out << updateNumerRules[i][j] << " ";
      }
      out << endl;
      
      out << "Denominator: ";
      for (j = 0; j < updateDenomRules[i].size(); j++) {
	out << updateDenomRules[i][j] << " ";
      }
      out << endl << endl;
    }

    out << "Regions: " << endl;
    for (nodeIndex ni = 0; (uint) ni < _regions.size(); ni++) {
      _regions[ni].print(out);
    }
  }

  inline void lbRegionGraph::printFormatted(ostream & out) const {
    lbDAG::printFormatted(out);

    out << "# Format: num vars \\t vars \\t num factors \\t factors" << endl;
    out << "@Regions"<<endl;
    for (nodeIndex ni = 0 ; ni < getNumNodes(); ni++) {
      out << "rgn" << ni << "\t";
      _regions[ni].printFormatted(out);
    }
    out << "@End" << endl <<endl;
  }

  /* 
   * We ensure a few things here.  First the region graph is a
   * valid graph with reasonable counting numbers.  Next we ensure
   * that the children are subsets of their parents.
   * Family preservation is verified in the region model.
   */
  inline bool lbRegionGraph::isValid() const {

    //cerr << "[DEBUG] Printing region graph:" << endl ;
    //print (cerr) ;

    if (!lbDAG::isValid()) {
      NOT_REACHED;
      return false;
    }

    if (!isConnected()) {
      NOT_REACHED;
      return false;
    }

    if (!isAcyclic()) {
      NOT_REACHED;
      return false;
    }

    if ((uint) getNumNodes() != _regions.size()) {
      NOT_REACHED;
      return false;
    }

    vector<double> countingNumVec = computeCountingNums();
    if (!countingNumVec.size() == getNumNodes()) {
      NOT_REACHED;
      return false;
    }

    if (getCountingNumsFile().empty()) {
      for (nodeIndex ni = 0; ni < getNumNodes(); ni++) {
        nodeIndexVec ancestors = getAncestors(ni);
        double total = 0;
        for (uint j = 0; j < ancestors.size(); j++) {
          total += countingNumVec[ancestors[j]];
        }
        
        if (countingNumVec[ni] != (1 - total)) {
          NOT_REACHED;
          return false;
        }
      }
    }

    // Ensure proper subset relationship
    for (arcIndex ai = 0; ai < getNumArcs(); ai++) {
      arc a = getArc(ai);
      assert(nodeExists(a.first));
      assert(nodeExists(a.second));
      
      lbRegion const& rgnParent = _regions[a.first];
      lbRegion const& rgnChild = _regions[a.second];
      //cerr<<"DEBUG parent "<<a.first<<" vars"<<endl;
      //printVector(rgnParent._varIndices,cerr);
      //cerr<<"DEBUG child "<<a.second<<" vars "<<endl;
      //printVector(rgnChild._varIndices,cerr);
      if (!rgnChild.isSubsetOf(rgnParent,getCountingNumsFile().empty())) {
        cerr<<"DEBUG parent: " ;
        printVector(rgnParent._varIndices,cerr);
        cerr<<"DEBUG child: " ;
        printVector(rgnChild._varIndices,cerr);
	NOT_REACHED;
	return false;
      } 
    }

    if (getCountingNumsFile().empty()) {    
      if (!varIndicesCountCorrectly(countingNumVec)) {
        NOT_REACHED;
        return false;
      }
      
      if (!facIndicesCountCorrectly(countingNumVec)) {
        NOT_REACHED;
        return false;
      }
    }

    return true;
  }


  /*
   * Cluster region graph generation helper methods.
   */

  inline void lbRegionGraph::installFirstClusterGeneration(varIndicesVecVec const& genvivv, 
							   facIndicesVecVec const& genfivv) {
    lbRegionVec rgns;
    assert(genvivv.size() == genfivv.size());
    assert(getNumNodes() == 0);  // Assume empty graph

    for (nodeIndex ni = 0; (uint) ni < genvivv.size(); ni++) {
      addNode();
      _regions.push_back(lbRegion(genvivv[ni], genfivv[ni]));
    }

    // Make sure this generation was valid!
    assert(ensureValidClusterGeneration(rgns, true));
  }

  inline void lbRegionGraph::installRemainingClusterGenerations(bool removeSubregions) {
    nodeIndex prevGenBegin = 0;
    nodeIndex prevGenEnd = getNumNodes();
    nodeIndex nextGenBegin = prevGenBegin;
    nodeIndex nextGenEnd = prevGenEnd;

    while (installSingleClusterGeneration(nextGenBegin, nextGenEnd, removeSubregions)) {
      // If here we just installed a generation
      assert(prevGenEnd == nextGenBegin);
      addClusterGenerationArcs(prevGenBegin, prevGenEnd, nextGenBegin, nextGenEnd);
      prevGenBegin = nextGenBegin;
      prevGenEnd = nextGenEnd;
    }
  }

  // genBegin and genEnd are initially the previous generation's indices and then
  // we update them to be the newest installed generation indices.
  inline bool lbRegionGraph::installSingleClusterGeneration(nodeIndex & genBegin, nodeIndex & genEnd, bool removeSubregions) {
    lbRegionVec nextGen;

    for (nodeIndex ni = 0; ni < genEnd; ni++) {
      for (nodeIndex nj = ((int) ni) + 1; nj < genEnd; nj++) {
	if (ni != nj) {
	  assert(nodeExists(ni));
	  assert(nodeExists(nj));
	  assert((uint) ni < _regions.size());
	  assert((uint) nj < _regions.size());

	  lbRegion tmp = _regions[ni].getIntersection(_regions[nj]);

	  if (!tmp.isEmpty()) {
	    bool found = false;

	    for (nodeIndex k = 0; k < genEnd ; k++) {
	      if (tmp.isEqual(_regions[k])) {
		found = true;
		break;
	      }
	    }

	    if (!found) {
	      nextGen.push_back(lbRegion(tmp._varIndices, tmp._facIndices));
	    }
	  }
	}
      }
    }

    ensureValidClusterGeneration(nextGen, removeSubregions);

    for (uint i = 0; i < nextGen.size(); i++) {
      addNode();
      addRegion(nextGen[i]);
    }

    genBegin = genEnd;
    genEnd = getNumNodes();

    return nextGen.size() > 0;
  }

  //Create a generation of only single variable regions:
  inline void lbRegionGraph::installSingleVariableClusterGeneration(varIndicesVecVec const& vivv) {
    nodeIndex topGenBegin = 0;
    nodeIndex topGenEnd = getNumNodes();

    //Calculate the variables to be added to the bottom generation:
    typedef set<rVarIndex> VarSet;
    VarSet* varsToAddAsRegions = new VarSet();
    for (nodeIndex ni = 0; (uint) ni < vivv.size(); ni++) {
      const varsVec& vars = vivv[ni];
      if (vars.size() > 1) {//otherwise, unnecessary to include in bottom generation
	for (varsVec::const_iterator vIt = vars.begin(); vIt != vars.end(); ++vIt)
	  varsToAddAsRegions->insert(*vIt);
      }
    }

    //Create the bottom generation:
    for (VarSet::const_iterator varIt = varsToAddAsRegions->begin();
	 varIt != varsToAddAsRegions->end(); ++varIt) {
      varsVec vv;
      vv.push_back(*varIt);
      facIndicesVec fv;
      
      addNode();
      _regions.push_back(lbRegion(vv, fv));
    }
    delete varsToAddAsRegions;


    nodeIndex bottomGenBegin = topGenEnd;
    nodeIndex bottomGenEnd = getNumNodes();

    //Connect the top and bottom generations:
    addClusterGenerationArcs(topGenBegin, topGenEnd, bottomGenBegin, bottomGenEnd);
  }

  inline void lbRegionGraph::addClusterGenerationArcs(nodeIndex prevGenBegin, nodeIndex prevGenEnd,
						      nodeIndex nextGenBegin, nodeIndex nextGenEnd) {
    for (nodeIndex np = 0; np < prevGenEnd; np++) {
      for (nodeIndex nc = nextGenBegin; nc < nextGenEnd; nc++) {
	assert(nodeExists(np));
	assert(nodeExists(nc));
	assert((uint) np < _regions.size());
	assert((uint) nc < _regions.size());

	lbRegion const& rgnp = _regions[np];
	lbRegion const& rgnc = _regions[nc];

	if (rgnc.isSubsetOf(rgnp)) {
	  if (np>= prevGenBegin) {
	    addArc(np, nc);
	  }
	  else {
	    bool found = false;
	    nodeIndexVec const& children = getOutgoingNodeIndices(np);
	    int numOfChildren = children.size();
	    for (int ind = 0 ; ind < numOfChildren ; ind++) {
	      lbRegion const& middleRgn = _regions[children[ind]];
	      if (rgnc.isSubsetOf(middleRgn)) {
		found = true;
	      }
	    }
	    if (!found) {
	      addArc(np, nc);
	    }

	  }
	}
      }
    }
  }
  
  // Ensures the we have a valid kikuchi geration (valid depends on
  // what we say for removeSubregions.  If remove subregions is true
  // then a valid generation will not have any regions that are
  // subsets of eachother.  If it is false it will only require that
  // two regions in the same generation be not equal.  Returns true if
  // the generation was already valid.
  inline bool lbRegionGraph::ensureValidClusterGeneration(lbRegionVec & rgns, bool removeSubregions) const {
    bool wasValid = true;

    for (nodeIndex ni = 0; (uint) ni < rgns.size(); ni++) {
      for (nodeIndex nj = 0; (uint) nj < rgns.size(); nj++) {
	if (ni != nj) {
	  bool isSubregion = rgns[ni].isSubsetOf(rgns[nj]);
	  bool isEqual = rgns[nj].isSubsetOf(rgns[ni]) && isSubregion;
	  bool remove = (isSubregion && removeSubregions) || isEqual;
	  
	  if (remove) {
	    rgns.erase(rgns.begin() + ni);
	    wasValid = false;
	    ni = ni - 1; // I now have to recheck the ni index.
	    break;
	  }
	}
      }
    }

    return wasValid;
  }

  // Cleans the region graph by merging equal nodes, killing nodes
  // with a counting number 0, and removing extra links between
  // ancestors and non-direct descendants.
  inline void lbRegionGraph::clean(bool killUselessRegions) {
    removeRedundantNodes(killUselessRegions);
    removeRedundantArcs();
    assert(isValid());
  }

  inline void lbRegionGraph::removeRedundantNodes(bool killUselessRegions) {
    mergeAllEqualRegions();

    if (killUselessRegions) {
      removeAllUselessRegions();
    }
  }

  inline void lbRegionGraph::removeRedundantArcs() {
    arcIndexVec removeCandidates;

    for (nodeIndex ni = 0; ni < getNumNodes(); ni++) {
      arcIndexVec children = getOutgoingArcIndices(ni);

      for (uint i = 0; i < children.size(); i++) {
	nodeIndexVec const& childsDesc = getDescendants(getArc(children[i]).second);

	for (uint j = 0; j < children.size(); j++) {
	  for (uint k = 0; k < childsDesc.size(); k++) {
	    if (getArc(children[j]).second == childsDesc[k]) {
	      removeCandidates.push_back(children[j]);
	    }
	  }
	}
      }
    }

    arcIndexVec toRemove = vecUnique(removeCandidates);
    removeSpecifiedArcs(toRemove);
  }

  inline void lbRegionGraph::mergeAllEqualRegions() {
    nodeIndexVec toRemove;

    for (nodeIndex ni = 0; ni < getNumNodes(); ni++) {
      nodeIndexVec const& parents = getIncomingNodeIndices(ni);
     
      for (uint j = 0; j < parents.size(); j++) {
	if (getRegion(parents[j]).isEqual(getRegion(ni))) {
	  mergeEqualRegions(ni, parents[j]);
	  toRemove.push_back(parents[j]);
	}
      }
    }

    removeSpecifiedNodes(toRemove);
  }

  inline void lbRegionGraph::mergeEqualRegions(nodeIndex mergeTo, nodeIndex mergeFrom) {
    arcIndexVec const& incomingArcs = getIncomingArcIndices(mergeFrom);
    
    for (uint i = 0; i < incomingArcs.size(); i++) {
      arc a = getArc(incomingArcs[i]);
      assert(a.second == mergeFrom);
      addArc(a.first, mergeTo);
    }
  }

  inline void lbRegionGraph::removeAllUselessRegions() {
    NOT_IMPLEMENTED_YET;
  }

  inline void lbRegionGraph::removeUselessRegions(nodeIndex ni) {
    NOT_IMPLEMENTED_YET;
  }
  
};

#endif
