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


#include <inferUtils.h>


using namespace lbLib ;

// TRW:
bool lbLib::TRWcheckConvergence (const doubleVec & prevCountingNums,
                                 const doubleVec & curCountingNums,
                                 double tol)
{
  /*
  // DEBUG
  cerr << "[DEBUG] Prev cns: " ;
  for (uint i=0 ; i<prevCountingNums.size() ; ++i) { cerr << prevCountingNums[i] << " " ; }
  cerr << endl ;
  cerr << "[DEBUG] Cur cns: " ;
  for (uint i=0 ; i<curCountingNums.size() ; ++i) { cerr << curCountingNums[i] << " " ; }
  cerr << endl ;
  // END DEBUG
  */

  // make sure L1 is not too large:
  double diff_L1 = 0.0 ;
  assert (prevCountingNums.size() == curCountingNums.size()) ;
  for (uint i=0 ; i<prevCountingNums.size() ; i++) {
    diff_L1 += abs (prevCountingNums[i]-curCountingNums[i]) ;
  }
  //cerr << "[DEBUG] Diff L1: " << diff_L1 << endl ;

  if (diff_L1 <= tol) {
    return true ;
  } else {
    return false ;
  }
}

void lbLib::updateCountNums (lbRegionBP * inf,
                             const doubleVec & newCountNums)
{
  inf->updateCountingNums(newCountNums) ; // dirty listeners!!!
  inf->makeVarValidCountingNums() ;
}

void lbLib::TRWupdateCountingNums (double alpha,
                                   lbRegionBP * inf,
                                   const doubleVec & prevCountNums,
                                   const doubleVec & gradientTree)
{
  doubleVec newCountNums ;
  for (uint i=0 ; i<prevCountNums.size() ; ++i) {
    newCountNums.push_back ((1-alpha)*prevCountNums[i] + alpha*gradientTree[i]) ;
  }
  updateCountNums (inf, newCountNums) ;

  /*
  // DEBUG
  const doubleVec & updatedCountNums = inf->getModel().getCountingNums() ;
  cerr << "[ERROR] Setting model counting numbers to: " ;
  for (uint i=0 ; i<updatedCountNums.size() ; ++i) {
    cerr << updatedCountNums[i] << " " ;
  }
  cerr << endl ;
  // END DEBUG
  */
}

double lbLib::TRWupdateAlpha (double alpha0,
                              lbRegionBP * inf,
                              const doubleVec & prevCountNums,
                              const doubleVec & gradientTree)
{
  // TODO: sometime in the future we want to implement Armijo's rule here
  //cerr << "[DEBUG] Finding alpha..." << endl ;

  double initPartition = inf->partitionFunction() ;

  static const double MIN_GAIN = 1e-2 ;
  static const double MIN_ALPHA = 1e-3 ;
  double alpha = 1 ;
  for (int m=0 ; alpha>MIN_ALPHA ; m++) {
    alpha *= alpha0 ;
    TRWupdateCountingNums (alpha, inf, prevCountNums, gradientTree) ;
    double curPartition = inf->partitionFunction() ;
    double gainPartition = initPartition - curPartition ;
    // DEBUG
    //cerr << "[DEBUG] alpha=" << alpha << " Partition gain: " << gainPartition << endl ;
    // END DEBUG
    if (gainPartition >= MIN_GAIN) {
      return alpha ;
    }
  }
  return -1 ; // no gain
}

doubleVec lbLib::TRWfindMinSpanTree (const E * modelEdges,
                                     int numEdges,
                                     int modelNumNodes,
                                     const probVector & pvWeights)
{
  double * weights = new double [pvWeights.size()] ;
  for (uint i=0 ; i<pvWeights.size() ; ++i) {
    weights[i] = pvWeights[i] ;
  }

  //cerr << "[DEBUG] Number of edges = " << numEdges << endl ;
  std::map<E,int> edge2cliq ;
  for (int i=0 ; i<numEdges ; ++i) {
    edge2cliq.insert (std::make_pair (modelEdges[i],i)) ;
  }

  Graph g (modelEdges, modelEdges + numEdges, weights, modelNumNodes) ;

  property_map < Graph, edge_weight_t >::type weight = get (edge_weight, g) ;
  std::vector < Edge > spanning_tree;

  kruskal_minimum_spanning_tree (g, std::back_inserter(spanning_tree)) ;

  delete[] weights ;

  doubleVec res (numEdges, 0) ; // no edges
  
  // TBD: create res from spanning_tree
  //std::cout << "Print the edges in the MST:" << std::endl;
  for (std::vector < Edge >::iterator ei = spanning_tree.begin();
       ei != spanning_tree.end();
       ++ei)
  {
    /*
    std::cout << source(*ei, g) << " <--> " << target(*ei, g)
              << " with weight of " << weight[*ei]
              << std::endl;
    */

    std::map<E,int>::const_iterator findCliq = edge2cliq.find (make_pair (source(*ei,g),
                                                                          target(*ei, g))) ;
    assert (findCliq != edge2cliq.end()) ;

    res[findCliq->second] = 1 ; // mark edge in tree
  }

  return res ;
}

void lbLib::TRWbuildGraphEdges (const lbRegionBP * inf,
                                E *& modelEdges,
                                int & modelNumNodes,
                                cliquesVec & edgeCliqs)
{
  // Go over all cliques, make sure they are either univar or
  // pairwise factors. For factors create edges and update nodes.

  vector<E> allEdges ;
  set<int> allVars ;

  for (cliqIndex cliq=0; cliq<inf->getGraph().getNumOfCliques(); cliq++) {
    varsVec cliqVars = inf->getGraph().getVarsVecForClique(cliq);

    if (cliqVars.size() == 1) { // variables
      //cerr << "[DEBUG] Univar cliq " << cliq << endl ;
      // do we want to do something with univars?
    }
    else if (cliqVars.size() == 2) { // pairwise
      //cerr << "[DEBUG] Pairwise cliq " << cliq << endl ;
      edgeCliqs.push_back (cliq) ;
      E curEdge ((int)cliqVars[0], (int)cliqVars[1]) ;
      allEdges.push_back (curEdge) ;
      allVars.insert ((int)cliqVars[0]) ;
      allVars.insert ((int)cliqVars[1]) ;
    }
    else {
      cerr << "[ERROR] Cannot call TRW with non-pairwise potentials (cliq " << cliq << ")." << endl ;
      exit (-1) ;
    }
  }

  modelEdges = new E[allEdges.size()] ;
  for (uint i=0 ; i<allEdges.size() ; ++i) {
    modelEdges[i] = allEdges[i] ;
    //cerr << "[DEBUG] Edge " << i << ": (" << modelEdges[i].first << "," << modelEdges[i].second << ")" << endl ;
  }

  modelNumNodes = (int)allVars.size() ;
  //cerr << "[DEBUG] Number of nodes: " << modelNumNodes << endl ;
}

void lbLib::trbp (lbRegionBP * inf) {

  // For the algorithm see:
  // A New Class of Upper Bounds on the Log Partition Function, Wainwright et al. 2005
  // (p. 2322)

  //cout << "[DEBUG] In TRW opt" << endl ;

  E * modelEdges = NULL ;
  int modelNumNodes ;
  cliquesVec edgeCliqs ;
  TRWbuildGraphEdges (inf, modelEdges, modelNumNodes, edgeCliqs) ;
  //for (uint i=0 ; i<edgeCliqs.size() ; ++i) {
  //  cerr << "[DEBUG] Edge " << i << ": (" << modelEdges[i].first << "," << modelEdges[i].second << ")" << endl ;
  //}

  bool converged = false ;
  static const int maxTRWIter = 1000; //200 ;
  int iterations = 0 ;
  static const double TOL = 1e-2 ;
  double alpha0 = 0.5 ;  // initial step size
  while (!converged && iterations<maxTRWIter) {
    // calculate marginals for current weights
    inf->calcProbs() ;

    doubleVec prevCountNums = inf->getModel().getCountingNums() ; 
    /*
    // DEBUG
    cerr << "[DEBUG] Model cns: " ;
    for (uint i=0 ; i<prevCountNums.size() ; ++i) { cerr << prevCountNums[i] << " " ; }
    cerr << endl ;
    // END DEBUG
    */

    // calculate new edge weights
    // ==========================
    // get edge gradient (mutual information of edges)
    probVector edgeGradient = inf->getNegMutualInfos (edgeCliqs) ;
    /*
    // DEBUG:
    cerr << "[DEBUG] Gradient vector: " ;
    for (uint i=0 ; i<edgeGradient.size() ; ++i) { cerr << edgeGradient[i] << " " ; }
    cerr << endl ;
    // END DEBUG
    */

    // get descent direction (argmax_rho)
    assert (edgeGradient.size() == edgeCliqs.size()) ;
    doubleVec treeEdges = TRWfindMinSpanTree (modelEdges,
                                              edgeCliqs.size(),
                                              modelNumNodes,
                                              edgeGradient) ;
    // returns indicator vector over factors
    assert (treeEdges.size() == edgeCliqs.size()) ;
    /*
    // DEBUG:
    cerr << "[DEBUG] MST: " ;
    for (uint i=0 ; i<treeEdges.size() ; ++i) { cerr << treeEdges[i] << " " ; }
    cerr << endl ;
    // END DEBUG
    */

    doubleVec newCountNums (prevCountNums.size(), 0) ;
    for (uint i=0 ; i<edgeCliqs.size() ; ++i) {
      newCountNums[edgeCliqs[i]] = treeEdges[i] ;
    }

    // Update alpha
    double alpha = TRWupdateAlpha (alpha0, inf, prevCountNums, newCountNums) ;
    if (alpha < 0) {
      alpha = 0 ;  // keep previous model and converge
    }
    TRWupdateCountingNums (alpha, inf, prevCountNums, newCountNums) ;

    converged = TRWcheckConvergence (prevCountNums,
                                     inf->getModel().getCountingNums(),
                                     TOL) ;
    iterations++ ;
  } // while not converged

  delete[] modelEdges ;
}


glp_prob * lbLib::valoptBuildLP (lbRegionBP * inf)
{
  // create a matrix of constraints

  // we need
  // for factors: ea, and Eiia
  // for variables: eaia, and Eaia

  varsVecVector cliq2vars ;
  varCliqvecMap varInCliq ;
  intVec facStart ;
  cliqIntMap facPos ;
  
  int numVars = 0 ;
  int numFacs = 0 ;
  int numVarFac = 0 ;

  // first pass - take counts:
  for (cliqIndex cliq=0; cliq<inf->getGraph().getNumOfCliques(); cliq++) {
    varsVec cliqVars = inf->getGraph().getVarsVecForClique(cliq);
    cliq2vars.push_back (cliqVars) ;
    facStart.push_back (numVarFac) ;

    if (cliqVars.size() == 1) { // variable clique
      numVars++ ;
    } // variable
    else { // factor clique
      assert (cliqVars.size() > 1) ;
      facPos.insert (make_pair(cliq,numFacs)) ;
      numFacs++ ;
      numVarFac += cliqVars.size() ;
    }

    // map: var -> containing cliques
    for (uint i=0 ; i<cliqVars.size() ; ++i) {
      varCliqvecMapIter findVar = varInCliq.find (cliqVars[i]) ;
      if (findVar == varInCliq.end()) { // create a new list of factors
        cliquesVec empty ;
        varInCliq.insert (make_pair(cliqVars[i],empty)) ;
        findVar = varInCliq.find (cliqVars[i]) ;
      }
      findVar->second.push_back (cliq) ;  // add cliq to variable containers
    } // if factor
  }

  int numLPVars = 2*numFacs + numVarFac ;
  glp_prob * lp = glp_create_prob() ;
  assert (lp) ;

  glp_set_obj_dir (lp, GLP_MIN) ; // we solve a min problem

  glp_add_cols (lp, numLPVars) ;

  // second pass - create constraints:
  glp_add_rows (lp, cliq2vars.size()) ; // number of constraints
  intVec ia, ja ;
  doubleVec ar ;
  int numConstraints = 0 ;
  for (uint i=0 ; i<cliq2vars.size() ; ++i) {
    if (cliq2vars[i].size() == 1) { // variable
      // find all factors that contain the variable
      rVarIndex curVar = cliq2vars[i][0] ;
      varCliqvecMapIter findVar = varInCliq.find (curVar) ;
      assert (findVar != varInCliq.end()) ;
      const cliquesVec & containingCliqs = findVar->second ;
      for (uint j=0 ; j<containingCliqs.size() ; ++j) {
        cliqIndex cliqNum = containingCliqs[j] ;
        if (cliq2vars[cliqNum].size() > 1) { // handle only factors (ignore univars)
          ja.push_back (facPos[cliqNum]) ;
          ia.push_back (numConstraints) ;
          ar.push_back (1.0) ;
          varsVec varsInContainingCliq = cliq2vars[cliqNum] ;
          for (uint k=0 ; k<varsInContainingCliq.size() ; ++k) {
            if (varsInContainingCliq[k] == findVar->first) {
              ja.push_back (numFacs*2 + facStart[cliqNum]+k) ;
              ia.push_back (numConstraints) ;
              ar.push_back (-1.0) ;
              break ;
            }
          }
        } // if not singleton
      } // for containing clique
      numConstraints++ ;
      glp_set_row_bnds (lp, numConstraints, GLP_UP, 1.0, 1.0) ;
    } // if variable
    else { // factor
      ja.push_back (facPos[i]) ;
      ia.push_back (numConstraints) ;
      ar.push_back (1.0) ;
      //
      ja.push_back (numFacs + facPos[i]) ;
      ia.push_back (numConstraints) ;
      ar.push_back (-1.0) ;
      for (uint j=0 ; j<cliq2vars[i].size() ; ++j) {
        //mEiia[facStart[i]+j] = -1 ;
        ja.push_back (numFacs*2 + facStart[i]+j) ;
        ia.push_back (numConstraints) ;
        ar.push_back (-1.0) ;
      }
      numConstraints++ ;
      glp_set_row_bnds (lp, numConstraints, GLP_FX, 0.0, 0.0) ;

      // Upper-bound on Bethe:
      glp_set_col_bnds (lp, facPos[i]+1, GLP_UP, 1.0, 1.0) ;
    }
  }

  // copy to arrays and set LP matrix:
  int nonZeros = ia.size() ;
  int * iaArr = new int [nonZeros+1] ;
  int * jaArr = new int [nonZeros+1] ;
  double * arArr = new double [nonZeros+1] ;
  for (int k=0 ; k<nonZeros ; ++k) {
    iaArr[k+1] = ia[k]+1 ;
    jaArr[k+1] = ja[k]+1 ;
    arArr[k+1] = ar[k] ;
  }
  
  glp_load_matrix (lp, nonZeros, iaArr, jaArr, arArr) ;

  delete [] iaArr ;
  delete [] jaArr ;
  delete [] arArr ;

  // caa,cia \geq 0
  for (int i=0 ; i<numFacs+numVarFac ; ++i) {
    glp_set_col_bnds (lp, i+1, GLP_LO, 0.0, 0.0) ;
  }

  return lp ;
}

doubleVec lbLib::valoptSolveLP (glp_prob * lp,
                                const probVector & edgeGradient)
{
  // set objective:
  for (uint i=0 ; i<edgeGradient.size() ; ++i) {
    glp_set_obj_coef (lp, i+1, edgeGradient[i]) ;
  }
  
  // DEBUG:
  //glp_write_lp (lp, NULL, "tmp.LP") ;

  // I only want to see important messages on screen while solving
  glp_smcp parm ;
  glp_init_smcp(&parm);
  parm.msg_lev = GLP_MSG_OFF ;

  int ret = glp_simplex (lp, &parm) ;
  assert (ret == 0) ;

  doubleVec res (edgeGradient.size()) ;
  for(uint i=0; i<edgeGradient.size(); ++i) {  // read only first |factors| values
    res[i] = glp_get_col_prim (lp, i+1) ; ;
  }

  return res ;
}

doubleVec lbLib::valoptUpdateCountNums (lbRegionBP * inf,
                                        const doubleVec & prevCountNums,
                                        const doubleVec & gradientNums)
{
  assert (prevCountNums.size() == gradientNums.size()) ;

  doubleVec lineStartCountNums = prevCountNums ;  // start from previous
  double lineMinPart = inf->partitionFunction() ;

  // we do a line-search
  static const double initStepSize = 5e-2 ;
  static const double TOL = 1e-3 ;
  //  bool converged = false ;
  //  cerr << "[DEBUG] Before outer loop" << endl ;
  //  while (!converged) {
    int stepIncrease = 0 ;
    for (double stepSize=initStepSize ; stepSize<0.5 ; stepSize*=2, stepIncrease++) {
      TRWupdateCountingNums (stepSize, inf, lineStartCountNums, gradientNums) ;
      double curPartition = inf->partitionFunction() ;
      //cerr << "[DEBUG] Inner loop: stepSize=" << stepSize
      //     << " curPartition=" << curPartition << endl ;
      if (curPartition + TOL < lineMinPart) {
        lineStartCountNums = inf->getModel().getCountingNums() ;
        lineMinPart = curPartition ;
      } else {
        stepIncrease++ ;
        break ;
      }
    }
    //    if (stepIncrease == 1) {
    //      converged = true ;
    //    }
    //  }

  return lineStartCountNums ;
}


void lbLib::varvalOpt (lbRegionBP * inf) {

  // We take here gradient steps (with respect to counting numbers) to minimize the energy function
  // dF/dC = -I_{\alpha}   (the mutual information)

  //cout << "[DEBUG] In varval opt" << endl ;

  // obtain factor cliques:
  cliquesVec edgeCliqs ;
  for (cliqIndex cliq=0; cliq<inf->getGraph().getNumOfCliques(); cliq++) {
    varsVec cliqVars = inf->getGraph().getVarsVecForClique(cliq);
    if (cliqVars.size() > 1) { // factor
      edgeCliqs.push_back (cliq) ;
    }
  }

  // obtain LP constraints (they don't change, only the objective):
  glp_prob * lp = valoptBuildLP (inf) ;
  assert (lp) ;

  bool converged = false ;
  int maxOptIter = 1000 ;
  int iterations = 0 ;
  static const double TOL = 1e-2 ;
  while (!converged && iterations<maxOptIter) {
    //cerr << "[DEBUG] var-val optimization, iteration=" << iterations << endl ;

    // calculate marginals for current weights
    inf->calcProbs() ;

    double prevPartition = inf->partitionFunction() ;
    //cerr << "[DEBUG] Partition = " << prevPartition << endl ;

    doubleVec prevCountNums = inf->getModel().getCountingNums() ; 
    /*
    // DEBUG
    cerr << "[DEBUG] Model cns: " ;
    for (uint i=0 ; i<prevCountNums.size() ; ++i) { cerr << prevCountNums[i] << " " ; }
    cerr << endl ;
    // END DEBUG
    */

    // calculate new edge weights
    // ==========================
    // get edge gradient (mutual information of edges)
    probVector edgeGradient = inf->getNegMutualInfos (edgeCliqs) ;
    /*
    // DEBUG:
    cerr << "[DEBUG] Gradient vector: " ;
    for (uint i=0 ; i<edgeGradient.size() ; ++i) { cerr << edgeGradient[i] << " " ; }
    cerr << endl ;
    // END DEBUG
    */

    // The descent direction is simply -gradient (we want to minimize)
    // Now we have to take the largest step in that direction
    // but we must make sure the variable-validity and convexity constraints
    // are not violated.
    // This boils down to solving a Linear Program
    doubleVec LPsolution = valoptSolveLP (lp, edgeGradient) ;
    // we need to update the model's counting numbers here:
    doubleVec newCountNums (prevCountNums.size(), 0) ;
    for (uint i=0 ; i<edgeCliqs.size() ; ++i) {
      newCountNums[edgeCliqs[i]] = LPsolution[i] ;
    }
    doubleVec smoothedSolution = valoptUpdateCountNums (inf, prevCountNums, newCountNums) ;
    updateCountNums (inf, smoothedSolution) ;
    //printVector (inf->getModel().getCountingNums(), cerr) ;

    // check improvement in partition
    double curPartition = inf->partitionFunction() ;
    if (prevPartition - curPartition < TOL) {
      converged = true ;
    }

    iterations++ ;
  }

  // cleanup:
  glp_delete_prob (lp) ;

  // done
}
