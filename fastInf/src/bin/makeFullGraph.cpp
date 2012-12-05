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

/*
  Construct Grid (output to .net file)
  Perform loopy using continuation
  Create grid spin glass model
*/

#include <lbDefinitions.h>
#include <lbRandomVar.h>
#include <lbMeasure.h>
#include <lbGraphStruct.h>
#include <lbModel.h>
#include <lbDriver.h>
#include <lbRandomProb.h>
#include <lbTableMeasure.h>
#include <lbMathUtils.h>
#include <lbBeliefPropagation.h>
#include <lbJunctionTree.h>

using namespace lbLib;

lbRandomGenerator _RandomProbGenerator = lbRandomGenerator() ;

static string _modelFile = "model.net";
static string _clusterFile = "clusters.out";

const char* usage = "makeFullGraph <-g graphSize> [options]\n"
" Options:\n"
" -n <name>       - output name of model\n"
" -g <size>       - size of complete graph (default==3)\n"
" -f <factor>     - factor of pairwise potentials (default=1)\n"
" -l <field>      - factor of single node potentials (default=0.05)\n"
" -s <seed>       - random seed\n"
" -e              - do exact inference\n"
" -p <A|M>        - attractive or mixed potentials";


map< int , lbMeasure_Sptr > cliqToMeas;
probType factor = 1.0;
probType singleFactor = 0.05 ;
enum tCoupling { C_ATTR, C_MIX };
tCoupling couplingType = C_MIX ;
int graphSize = 0;
bool exact = false;

cliqIndex createSingleClique(lbGraphStruct& Graph,int i)
{
  varsVec vec;
  vec.push_back(i);
  cliqIndex cind = Graph.addClique(vec);
  return cind;
}

lbMeasure_Sptr createSingleMeasure(lbGraphStruct& Graph,cliqIndex cind)
{
  cardVec cvec(1,2);
  lbMeasure_Sptr meas(new lbTableMeasure<lbValue>(cvec));
  // sample uniformly in [-singleFac,singleFac]
  probType lam = singleFactor - _RandomProbGenerator.RandomDouble(2*singleFactor);
  probType vals[2];
  /*
  vals[0] = log2(_RandomProbGenerator.RandomProb()); 
  vals[1] = log2(_RandomProbGenerator.RandomProb());
  */
  vals[0] = lam ;
  vals[1] = -lam ;
  bool changed;
  assert( meas->setLogParams(&vals[0],0,changed) == 2 );
  cliqToMeas.insert(std::make_pair(cind,meas));
  return meas;
}

cliqIndex createPairwiseClique(lbGraphStruct& Graph,
			       int from,int to)
{
  // create clique
  varsVec vec;
  vec.push_back(from);
  vec.push_back(to);
  cliqIndex  cind = Graph.addClique(vec);
  Graph.addCliqueNeighbor(cind,from);
  Graph.addCliqueNeighbor(cind,to);
  return cind;
}

lbMeasure_Sptr createPairwiseMeasure(lbGraphStruct& Graph,cliqIndex cind)
{
  probType lam = 0 ;
  if (couplingType == C_ATTR) {
    lam = _RandomProbGenerator.RandomDouble(factor); // in the range [0,factor]
  } else if (couplingType == C_MIX) {
    lam = factor - _RandomProbGenerator.RandomDouble(2*factor);  // in [-factor,factor]
  }
  cardVec cvec(2,2);
  lbMeasure_Sptr meas(new lbTableMeasure<lbValue>(cvec));
  probType vals[4];
  /*
  vals[0] = lam/2;
  vals[1] = -lam/2;
  vals[2] = -lam/2;
  vals[3] = lam/2;
  */
  vals[0] = lam;
  vals[1] = -lam;
  vals[2] = -lam;
  vals[3] = lam;
  bool changed;
  assert ( meas->setLogParams(&vals[0],0,changed) == 4 );
  cliqToMeas.insert(std::make_pair(cind,meas));
  return meas;
}


void computeAndPrintExact(lbModel *model, lbMeasureDispatcher &disp) {
  lbModel * emodel = NULL ;
  try {
    cerr << "Converting model into junction tree..." << endl;
    // This next line creates a new model. This call to the constructor
    // is set such that the model remembers it allocated new graph and cards.
    // Thus, in deleting this model, it will also delete the objects it allocated.
    emodel = lbJunctionTree::CalcJunctionTreeGraphicalModel(model);
  }
  catch(string* s) {
    throw s;
  }

  cerr << "Running exact inference..." << endl;
  lbBeliefPropagation* einf = new lbBeliefPropagation(*emodel,disp);
  cerr << "Finished running exact inference..." << endl;

  //emodel->printAllNetToFile("exact.net");

  ofstream out(_modelFile.c_str(), ios_base::app);
  out << endl << endl << "@Exact" << endl;
  for (int i = 0; i < model->getGraph().getNumOfCliques(); i++) {
    varsVec const& vars = model->getGraph().getVarsVecForClique(i);
    lbAssignedMeasure_ptr exact = einf->prob(vars);
    exact->getMeasure().printToFastInfFormat(out);
  }
  out << "@End" << endl;

  // Partition:
  out << "# Exact Partitition function:" << endl;
  out << "# "<<einf->partitionFunction() << endl;

  out.flush();
  out.close();

  delete emodel;
  delete einf;
}


void generateSingleGrid(lbMeasureDispatcher &MD) {
  cliqToMeas.clear();

  lbGraphStruct_ptr Graph = NULL;
  lbModel_ptr Model = NULL;
  lbDriver_ptr Driver = NULL;
  
  lbVarsList varsList;
  lbCardsList cardsList(varsList);

  // construct grid 
  //    cerr << "Creating vars\n";
  int varNum = graphSize;
  // create var list
  int count=0;
  varsVec vvec;
  for (int i=0 ; i<graphSize ; i++,count++) {
    ostringstream name;
    name << "V" << count;
    rVarIndex ind = varsList.addRandomVar(name.str());
    assert(ind._val == count);
    vvec.push_back(ind);
  }
  cardVec cvec(varNum,2);
  cardsList.setCardsForAllVars(cvec);
  
  /// create graph
  //    cerr << "Creating graph\n";
  Graph = new lbGraphStruct(varsList);
  
  // build clique for each var
  //    cerr << "Building single var cliques\n";
  for (int i=0 ; i<varNum ; i++ ) {
    cliqIndex cind = createSingleClique(*Graph,i);
    createSingleMeasure(*Graph,cind);
  }

  // build pairwise cliques - for all pairs of nodes
  //    cerr << "Building pairwise cliques\n";
  for (int from=0 ; from<graphSize ; from++) {
    for (int to=from+1 ; to<graphSize ; to++) {
      cliqIndex cind = createPairwiseClique(*Graph,from,to);
      createPairwiseMeasure(*Graph,cind);
    }
  }
  
  Model = new lbModel( *Graph, cardsList, MD );
  map< int , lbMeasure_Sptr >::iterator it;
  for ( it=cliqToMeas.begin() ; it!=cliqToMeas.end() ; it++ ) {
    measIndex mind = Model->addMeasure(it->second);
    assert(it->first == mind);
    Model->setMeasureForClique(it->first,mind);
  }
  
  Model->printAllNetToFile(_modelFile.c_str());

  if (exact) {
    computeAndPrintExact(Model, MD);
  }

  if ( Driver != NULL ) {
    delete Driver;
  }

  delete Model;
  delete Graph;

  cerr << "Done!\n";

}

int main(int argc, char* argv[]) {
  long seed = time(NULL);

  factor = 9;

  int i;
  for( i=1; i<argc; i++ ) {
    if( argv[i][0] == '\0' || argv[i][0] != '-' )
      continue;
    switch ( argv[i][1] ) {
    case 'n':
      _modelFile = argv[i+1];
      i++;
      break;
    case 'g':
      graphSize = atoi(argv[i+1]);
      assert(graphSize>1);
      i++;
      break;
    case 'f':
      factor = atof(argv[i+1]);
      i++;
      break;
    case 'l':
      singleFactor = atof(argv[i+1]);
      i++;
      break;
    case 'e':
      exact = true;
      break;
    case 's':
      seed = atoi(argv[i+1]);
      i++;
      break;
    case 'p':
      if (argv[i+1][0] == 'A') {
        couplingType = C_ATTR ;
      }
      else if (argv[i+1][0] == 'M') {
	couplingType = C_MIX ;
      }
      else {
	cerr << "Invalid coupling type specified (expected A or M)";
	assert(false);
      }
      i++;
      break;
    default:
      cerr << "Unknown option " << argv[i] << endl;
      cerr << "Usage: " << usage << endl;
      exit(1);
    }
  }

  if (graphSize == 0) {
    cerr << "Must give grid size\n";
    cerr << "Usage: " << usage << endl;
    exit(1);
  }

  // OPTIONS
  cerr << "Initializing random seed " << seed << endl;
  _RandomProbGenerator.Initialize(seed);

  if ( couplingType == C_MIX )
    cerr << "Building full graph (with mixed coupling) of size " << graphSize << "\n";
  else
    cerr << "Building full graph (with attractive coupling) of size " << graphSize << "\n";

  cerr << "Using factor = " << factor << endl;

  lbMeasureDispatcher MD(MT_TABLE_NOLOG);

  generateSingleGrid(MD);

  cerr << "#DONE" << endl;
  return 0;
}
