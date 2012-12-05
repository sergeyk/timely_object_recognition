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

#include <lbPropagationInference.h> 
#include <lbSubgraph.h>
#include <iomanip>
using namespace std;

using namespace lbLib;

/*
 * CONSTRUCTORS / DESTRUCTORS
 */
lbPropagationInference::lbPropagationInference(lbModel & model, lbMeasureDispatcher const & disp) :
  lbInferenceObject(model,disp) {

  _induceSpanningTrees = true;
  _built = false;
  _factors = NULL;
  _factorsUpdated = false;
}

lbPropagationInference::~lbPropagationInference() {
  if (_factors != NULL)
    delete _factors;
}

void lbPropagationInference::setOptions(lbOptions & opt, int argc, char *argv[]) {
  opt.setOptions(argc, argv);
}

lbPropagationInference::lbPropagationInference(lbPropagationInference const& otherPropagation) 
  : lbInferenceObject(otherPropagation) {
  NOT_IMPLEMENTED_YET;
}

void lbPropagationInference::initialize(bool allocate,bool useOldInfo) {
  if (allocate) {
    buildSpanningTree();
    initFactors();
    _evidence = lbAssignment();
    _built = true;
  }

  assert(_built);
  _calculatedInitialPartition = false; 
  _calculatedPartition = false;

  // We calculate it only when necessary
  _initialPartition = 0;
  _currentPartition = 0;
  _currentEntropy = 0;
  
  resetEvidence();

  if (!allocate) {
    measIndicesVec vec;
    for(measIndex m = 0; m < _model.getNumOfMeasures(); m++) {
      vec.push_back(m);
    }
    resetFactors(vec);
  }
}

void lbPropagationInference::initFactors() {

  if (_factors != NULL)
    delete _factors;
  
  int numOfCliques = _model.getGraph().getNumOfCliques();
  _factors = new lbAssignedMeasureCofwPtrVec(numOfCliques);
  for (cliqIndex cliq = 0; cliq < numOfCliques; cliq++){
    this->updateFactorFromModel(cliq);
  }
}

void lbPropagationInference::updateFactorFromModel(cliqIndex cliq) {

  if (_modifyAndDeleteModel) //any changes to _factors will CHANGE _model's measures [but won't delete the measure]:
    (*_factors)[cliq] = lbAssignedMeasureCofwPtr(&(_model.getAssignedMeasureForCliqueNonConst(cliq)), true);
  else //don't allow for modifications of _model:
    (*_factors)[cliq] = lbAssignedMeasureCofwPtr(&(_model.getAssignedMeasureForClique(cliq)));
}

void lbPropagationInference::buildSpanningTree(){
  int numOfCliques = getNumCliques();

  _localMessagesAdjList = adjListVec(numOfCliques);
  _localMessagesScopes = neighborSeparatorVec(numOfCliques);

  neighborSeparatorVec const& originalGraphSeparators = _graph.getGraphSeparators();

  // Go over all vars and build a spanning tree for each
  for (rVarIndex var = 0; var < _graph.getNumOfVars(); var++) {
    edgeSet edges = edgeSet();

    // make a nodes list out of all cliques that have this var
    nodesSet const& nodes = _graph.getAllCliquesForVar(var);

    //for (cliqIndex toCliq = 0; toCliq < (numOfCliques); toCliq++) {
    for (uint i = 0; i < nodes.size(); i++) {
      cliqIndex toCliq = nodes[i];
      separatorsVec cliqSep = originalGraphSeparators[toCliq];
      

      for (uint neighborInd=0;neighborInd<cliqSep.size();neighborInd++){
	cliqIndex neighborCliq = _graph.cliqueNeighbors(toCliq)[neighborInd];
	varsVec const& sepVars = cliqSep[neighborInd];
	for(vecIndex ind = 0;((uint) ind) < sepVars.size(); ind++) {
	  if (sepVars[ind]==var){
	    // if it does, add this edge to the edge list
	    if (toCliq < neighborCliq) {  // no sense in doing it twice
	      edges.push_back(edge(toCliq,neighborCliq));
	    }
	  }
	}
      }
    }

    
    // For each edge, add this var to the separator scope of
    // the two "factor vertices" it connects.
    if (_induceSpanningTrees) {
      // Now build a spanning tree given the edges and vertices   
      lbSpanningTree_ptr tree(new lbSpanningTree(edges, nodes, _graph.getNumOfCliques()));
      tree->buildSpanningTree();
      addEdgesToLocalGraph(tree->getSpanningTree(), var);
      delete tree;
    }
    else {
      addEdgesToLocalGraph(edges, var);
    }
      

  }
}

void lbPropagationInference::addEdgesToLocalGraph(edgeSet const& edges,rVarIndex var) {
  int numOfCliques = getNumCliques();
 
  for (vecIndex edgeInd = 0; ((uint) edgeInd) < edges.size(); edgeInd++) {
    edge ed = edges[edgeInd];
    cliqIndex fromCliq = ed.first();
    cliqIndex toCliq = ed.second();

    //If an edge has been added we add it in
    //both directions

    // First check if a separator already exists between two cliques
    bool found = false;
    vecIndex index;
    for (vecIndex neighborInd = 0; ((uint)neighborInd) < _localMessagesAdjList[fromCliq].size(); neighborInd++) {
      if (_localMessagesAdjList[fromCliq][neighborInd]==toCliq){
	found=true;
	index = neighborInd;
	break;
      }
    }

    if (!found){
      // Edge has not been found.  We must add it.

      // Add edge
      index = _localMessagesAdjList[fromCliq].size();
      _localMessagesAdjList[fromCliq].push_back(toCliq);
      vecIndex toIndex = _localMessagesAdjList[toCliq].size();
      _localMessagesAdjList[toCliq].push_back(fromCliq);

      // Create new separator with var.
      _localMessagesScopes[fromCliq].push_back(varsVec());
      _localMessagesScopes[fromCliq][index].push_back(var);

      // Create the same scope on the other side.
      _localMessagesScopes[toCliq].push_back(varsVec());
      _localMessagesScopes[toCliq][toIndex].push_back(var);
    }
    else {
      // Edge has been found.
      _localMessagesScopes[fromCliq][index].push_back(var);
      for (vecIndex neighborInd=0;((uint)neighborInd)<_localMessagesAdjList[toCliq].size();neighborInd++) {
	if (_localMessagesAdjList[toCliq][neighborInd]==fromCliq) {
	    _localMessagesScopes[toCliq][neighborInd].push_back(var);
	}
      }
    }
  }

  // Cache all sizes.
  _localSizes = vecSizeVec(numOfCliques);
  for (cliqIndex cliq = 0; cliq < numOfCliques; cliq++) {
    _localSizes[cliq] = _localMessagesAdjList[cliq].size();
  }      
}

void lbPropagationInference::setEvidence(lbAssignment const& assign) {
  changeEvidence(assign,true);
}

probType lbPropagationInference::evidenceLogProb() {

  double ipf = initialPartitionFunction();
  double pf = partitionFunction();
  double elp = pf - ipf;
  
  // p(x,e) = 1/Z(mult over all cliqs(p(cliq))
  // p(x|e) = Ze / Z ==> log p(x,e) = log Ze - log Z

  if ( exp2(elp) > 1.0 ) {
    probType diff  = 1.0 - (probType) exp2(elp);
    cerr << ">1: " << diff;
    cerr << " Curr partition: " << pf;
    cerr << " Init part: " << ipf << endl;
    _evidence.print(cerr, getModel().getGraph().getNumOfVars());
  }
  else if (isnan(exp(elp))) {
    cerr << "nan: " << (exp(elp)) << endl;
    _evidence.print(cerr, getModel().getGraph().getNumOfVars());
  }

  return elp;
}

bool lbPropagationInference::factorsUpdated(measIndicesVec const& vec) {
  if (!_built) {
    reset();
  }

  resetFactors(vec);

  if (vec.size() > 0) {
    if (_evidence.getSize() != 0) {
      setEvidence(_evidence);
    }
    setFactorsUpdated(true);
    setCalculatedBeliefs(false);
    setCalculatedPartition(false);
    setCalculatedInitPartition(false);
    return true;
  }

  return false;  
}

void lbPropagationInference::resetFactors(measIndicesVec const& vec) {
  uint numOfChangedMeasures = vec.size();
  for (uint i = 0; i < numOfChangedMeasures; i++) {
    measIndex meas = vec[i];
    cliquesVec changedCliques = _model.getCliquesUsingMeasure(meas);

    for (uint ind=0; ind < changedCliques.size(); ind++) {
      this->updateFactorFromModel( changedCliques[ind] );
    }
  }
} 

/*
 * We can change only part of the evidence if the 
 * other nodes are marked -1.
 */
void lbPropagationInference::changeEvidence(lbAssignment const& assign, bool forceUpdate) {
  if (!_built) {
    reset();
  }

  varsVec allVars = getModel().getGraph().getVars().getVarsVec();
  if (getEvidence().equals(assign, allVars) && !forceUpdate) {
    return;
  }

  setCalculatedPartition(false);
  setCalculatedBeliefs(false);

  /* If the old and new evidences are both empty, then
     (*_factors)[cliq].get() == &(_model.getAssignedMeasureForClique(cliq)),
     since the old evidence did not zero any of (*_factors)[cliq]
     (and this is what the new evidence also requires). */
  if (getEvidence().isEmpty() && assign.isEmpty()) {
    return;
  }
  
  _evidence = assign;

  // Go over all clique measures and messages:
  cliqIndex cliq;
  for (cliq = 0; cliq < getNumCliques(); cliq++){
    (*_factors)[cliq]->updateAssign(_evidence, _model.getAssignedMeasureForClique(cliq));
  } // over cliques
}

void lbPropagationInference::resetEvidence() {
  setEvidence(lbAssignment());
}


lbAssignedMeasure_ptr lbPropagationInference::prob(varsVec const& vars) {
  cliqIndex bestCliq = -1;
  varsVec bestVars = varsVec();

  // Find best cliq (contains most of vars)
  for (cliqIndex cliq=0;cliq<_graph.getNumOfCliques();cliq++){
    varsVec tempVars=varsVec();
    varsVec cliqVars = _graph.getVarsVecForClique(cliq);

    // TO DO: make match function for sets of vars
    for (rVarIndex var=0;var<(rVarIndex)vars.size();var++){
      for (rVarIndex cliqVar=0;cliqVar<(rVarIndex)cliqVars.size();cliqVar++){
	if (cliqVars[cliqVar]==vars[var])
	  tempVars.push_back(vars[var]);
      }
    }

    if (tempVars.size() > bestVars.size()){
      if (tempVars.size() == vars.size()){
	return prob(vars,cliq);
      }

      bestCliq=cliq;
      bestVars=tempVars;
    }
  }

#ifdef DEBUG
  if (bestCliq == -1){
    cerr<<"in loopy evidenceprob, Var not found\n";
    exit(1);
  }
#endif
  
  // Did not find a perfect cliq find the 'not found' vars and calc prob
  varsVec notFoundVars = varsVec();
  for (rVarIndex var = 0; var < (rVarIndex) vars.size(); var++) {
    bool found = false;

    for (rVarIndex cliqVar=0; cliqVar < (rVarIndex) bestVars.size(); cliqVar++)
      if(vars[var] == bestVars[cliqVar])
	found=true;

    if (!found)
      notFoundVars.push_back(vars[var]);
  }
    
  return prob(vars,bestVars,notFoundVars,bestCliq);
}

/*
 * Get the distribution over a set of variables which are not
 * necessarily in the same clique (bestCliq contains as many
 * of them as possible though).  We do this by assigning the 
 * variables not in bestCliq and iterating over each assignment
 * summing up the probabilities as we go.
 */
lbAssignedMeasure_ptr lbPropagationInference::prob(varsVec const& allVars,
						    varsVec const& bestVars,
						    varsVec const& notFoundVec,
						    cliqIndex bestCliq){

  // build new evidence and remember old (if exists)
  lbAssignment oldEvidence = _evidence;
  lbAssignment assign = _evidence;
  assign.zeroise(notFoundVec);

  // Build cards
  cardVec const& bestCard = _model.getCardForVars(bestVars);
  cardVec const& notFoundCard = _model.getCardForVars(notFoundVec);
  cardVec const& allVarsCard = _model.getCardForVars(allVars);

  // Build new Assigned measures with the correct scope
  lbMeasure_Sptr measPtr = _measDispatcher.getNewMeasure(allVarsCard);
  lbAssignedMeasure_ptr result = new lbAssignedMeasure(measPtr,allVars); 

  lbMeasure_Sptr localMeasPtr;
  lbAssignedMeasure_ptr localAssignMeas;

  probType probOfFull;
  probType conditionalProb;

  // Go over all assignments for y
  // p(x,y|E)/Z = p(x|y,E)*p(y|E) / Z 
  //            = p(x|y,E)*p(y,E) / Z'
  // and fill the measure
  lbFullAssignment movingAssign = lbFullAssignment(assign);
  movingAssign.zeroise(bestVars);

  do {
    changeEvidence(assign);
    localAssignMeas = prob(bestVars, bestCliq);

    //calculate p(y,E)
    probOfFull = evidenceProb();

    // now go over all X's and assign in the right place
    do {
      conditionalProb = localAssignMeas->valueOfFull(movingAssign);
      probType mult = probOfFull * conditionalProb;
      result->setValueOfFull(movingAssign, mult);
    } while (movingAssign.advanceOne(bestCard,bestVars));

    movingAssign.advanceOne(notFoundCard,notFoundVec);
    delete localAssignMeas;

  } while (assign.advanceOne(notFoundCard,notFoundVec));

  // revert back
  changeEvidence(oldEvidence);
  return  result;  
}

// Assuming all vars are in this cliq!
lbAssignedMeasure_ptr lbPropagationInference::prob(varsVec const& vars,cliqIndex cliq,bool computeProbs) {
  if ( computeProbs )
    calcProbs();

  lbAssignedMeasure_ptr belief = NULL;
  belief = getBelief(cliq);

  if (vars.size() != _graph.getVarsVecForClique(cliq).size()) {
    lbAssignedMeasure_ptr old = belief;
    belief = belief->marginalize(vars, _measDispatcher);
    delete old;
  }

  belief->normalize();
  return belief;    
}

void lbPropagationInference::revertFactorsFromModel() {
  // read measure of the model

  measIndicesVec vec;
  for(measIndex m = 0; m < _model.getNumOfMeasures(); m++) {
    vec.push_back(m);
  }

  factorsUpdated(vec);
}

cliquesVec const& lbPropagationInference::getNeighbors(cliqIndex cliq) const {
  return _localMessagesAdjList[cliq];
}

int lbPropagationInference::getIndexOfNeighbor(cliqIndex wrtCliq, cliqIndex ofCliq) const {
  cliquesVec const& neighborsList = _localMessagesAdjList[wrtCliq];
  uint ofCliqIndex;

  for (ofCliqIndex = 0; ofCliqIndex < _localSizes[wrtCliq]; ofCliqIndex++){
    if (neighborsList[ofCliqIndex] == ofCliq)
      break;
  }

  return (ofCliqIndex < _localSizes[wrtCliq] ? ofCliqIndex : NOT_CLIQ);
  //  return ofCliqIndex;
}

varsVec const& lbPropagationInference::getScope(messageIndex messIndex) const {
  int fromCliqIndex = getIndexOfNeighbor(messIndex.second, messIndex.first);
  if (fromCliqIndex != NOT_CLIQ) {
    return _localMessagesScopes[messIndex.second][fromCliqIndex];
  }
  return _emptyScope;
}

lbAssignedMeasure const* lbPropagationInference::getFactor(cliqIndex index) const {
  lbAssignedMeasureCofwPtr const& amCp = (*_factors)[index];
  return amCp.get();
}


probType lbPropagationInference::fullEvidenceScore() {
  return exp(fullEvidenceLogScore());
}

probType lbPropagationInference::fullEvidenceLogScore() {
  if (!_model.getGraph().getNumOfVars() == _evidence.getSize()) {
    cerr << "Asked for evidence probability of an assignment to "
	 << "variables that was not full.\n";
    return -HUGE_VAL;
  }
  probType result = 0;
  for (uint i = 0; i < _factors->size(); ++i) {
    // For each factor, calculate its value with the evidence.
    const varsVec & vvec ((*_factors)[i]->getVars());
    lbSmallAssignment assign(vvec);
    for (uint j = 0; j < vvec.size(); ++j) {
      assign.setValueForVar(vvec[j], _evidence.getValueForVar(vvec[j]));
    }
    result += (*_factors)[i]->logValueOfFull(assign);
  }
  cerr << endl;
  return result;
}
