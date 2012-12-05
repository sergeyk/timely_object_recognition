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

#include <lbDefinitions.h>
#include <lbRegionBP.h>
#include <lbGridQueue.h>
#include <lbDriver.h>
#include <lbTableMeasure.h>
#include <lbBeliefPropagation.h>
#include <sstream>
#include <sys/stat.h>
#include <lbJunctionTree.h>
#include <inferUtils.h>
#include <timer.h>

/*!  This File is an example to how to create and run inference with
  belief propagation It reads a model from a file creates an infrence
  object and calculate all marginals and the partition function.
 */
using namespace std;
using namespace lbLib;
using namespace boost;

lbDriver * _driver = NULL;
lbMeasureDispatcher * _disp = NULL;

/*!
  Default values of parameters
 */
//Max or sum product on the bp
bool _useMaxProduct = false;

//Use log smoothing on messages instead of regular smoothing
bool _useLogSmoothing = false;

//Queue type (0-unweighted, 1-weighted, 2-bars, 3-crisscross, 4-snakes)
//last three options are only for grids
int _queueType = 0;

//Whether to print a specific belief or all (or none)
int _printBeliefs = -1;

//Whether to print a specific marginal or all (or none)
int _printMarginals = -1;

//Whether to measure the time it took to run inference
bool _printTime = false;
string _printTime2File ;

//Seed for random numbers 
int _seed = -1 ;

//default names for input and output files
string _inputFile = "none.net";
string _clusterFile = "none.net";
string _evidenceFile = "none.assign";
fullAssignmentPtrVec _evidence;
string _countingNumsFile ;

//Sparsitiy 
double _makeSparseHack = 0;
double _considerSparse = 1.0;

//Run exact inference on triangulated model (to compare and validate results on small models)
bool _exactInf = false ;

/*!
 * This helper function reads full evidence (optionaly many instances, each in one line) from a file 
 * n is the number of variables in the model
 * missing values are assigned by '?'
 */
void readEvidenceFromFile(string evidenceFileName, int n) {

  ifstream_ptr in;
  try {
    in = ifstream_ptr(new ifstream(evidenceFileName.c_str()));
  }
  catch (...) {
    cerr << "error while reading file: " << evidenceFileName << endl;
  }
  _evidence = fullAssignmentPtrVec();
  while (!in->eof()) {
    lbFullAssignment_ptr assign(new lbFullAssignment());
    if (assign->readAssignmentFromFile(*in, n)) {
      _evidence.push_back(assign);
    }
  }
  cerr<<"num of evidences "<<_evidence.size()<<endl;
  in->close();
}

/*!
  Set all the parameters that can be set by the user
 */
void setOptions(lbOptions & opt, int argc, char *argv[]) {
  opt.addStringOption("i", &_inputFile, "input .net file");
  opt.addStringOption("e", &_evidenceFile, "input .assign file");
  opt.addStringOption("c", &_clusterFile, "input cluster file");
  opt.addIntOption("b", &_printBeliefs, "print the first N clique beliefs (0 for all)");
  opt.addIntOption("m", &_printMarginals, "print the first N singleton beliefs (0 for all)");
  opt.addBoolOption("t", &_printTime, "print the time");
  opt.addStringOption("tF", &_printTime2File, "print the time in seconds into a file") ;
  opt.addBoolOption("x", &_useMaxProduct, "do max product");
  opt.addBoolOption("g", &_useLogSmoothing, "use log space smoothing");
  opt.addIntOption("q", &_queueType, "type of queue to use (0-unweighted, 1-weighted, 2-bars, 3-crisscross, 4-snakes)");
  opt.addDoubleOption("pS", &_makeSparseHack, "add zeroes to make nonunivariate potentials sparse");
  opt.addDoubleOption("pC", &_considerSparse, "set threshold at which potentials are sparse");
  opt.addIntOption("s",&_seed,"initialize the random seed (-1 for time)");
  opt.addStringOption("k",&_countingNumsFile, "use GBP with counting nums from file");
  opt.addStringOption("trwopt",&_countingNumsFile, "Run TRW algorithm and find optimal tree weights");
  opt.addStringOption("valopt",&_countingNumsFile, "Try to minimize the energy under variable-valid and convexity constraints");
  opt.addBoolOption("exact", &_exactInf, "run exact inference using junction tree");

  for (int i = 0; i < V_MAX; i++) {
    opt.addVerboseOption(i, lbDefinitions::verbose_descriptions[i]);
  }

  opt.setOptions(argc, argv);
  
  if (!opt.isOptionSetByUser("i")) {
    opt.usageError("Must give a .net file");
  }
  if ((opt.isOptionSetByUser("k") && opt.isOptionSetByUser("trwopt"))  ||
      (opt.isOptionSetByUser("k") && opt.isOptionSetByUser("valopt"))  ||
      (opt.isOptionSetByUser("trwopt") && opt.isOptionSetByUser("valopt")))
  {
    opt.usageError("\"k\", \"trwopt\", and \"valopt\" are exclusive.") ;
  }
}


/*!
  Read the user input, and create the inference object.
 */
lbBeliefPropagation * getInferenceObject(int argc, char * argv[]) {
  lbModel_ptr model = NULL;

  // Read options from the user
  lbOptions opt;
  setOptions(opt, argc, argv);

  //initialize random seed
  if ( _seed == -1 ) 
    _seed = (int)time(NULL);
  _lbRandomProbGenerator.Initialize((long)_seed);

  //initialize the measure factory with log representation 
  //to work with non-log change this to MT_TABLE_NOLOG
  _disp = new lbMeasureDispatcher(MT_TABLE);

  //create the driver that reads the model
  _driver = new lbDriver(*_disp);
 
  //read the model from file
  cerr << "Reading network... ";
  _driver->readUniverse(_inputFile.c_str());
  cerr << "done." << endl;

  model = &_driver->getModel();

  /*
  // Uncommet to print the model:
  model->printAllNetToFile(_clusterFile.c_str()) ;
  */

  if (model == NULL) {
    cerr << "Error reading network: " << _inputFile << endl;
    exit(1);
  }

  if (_makeSparseHack) {
    model->makeSparseHack(_makeSparseHack);
  }

  //Set inference parameters
  lbMeasure::setMaxProduct(_useMaxProduct);
  lbMeasure::setLogSmooth(_useLogSmoothing);

  lbBeliefPropagation *inf = NULL;

  //Uncomment to print numbers with high precision
  //cerr << setprecision(15);

  //There are three possibilities for creating models
  // 1) Run standard loopy belief propagation on the model
  // 2) Run GBP with Kikuchi's Cluster Variation Method
  // 3) Run Generlized BP with predefined counting numbers
  // 4) Run Generlized BP when the program determines counting numbers (e.g., TRW optimal (Wainright et al) or ValOpt (Meshi et al) 
  if (_clusterFile == "none.net") {
    if (opt.isOptionSetByUser("k")  ||  opt.isOptionSetByUser("trwopt")  ||  opt.isOptionSetByUser("valopt")) {
      //Options (3) and (4)
      //Get vivv (var indices) and fivv (factor indices) from model
      pair <varsVecVector,vector<measIndicesVec> > vecPair =
        lbRegionBP::getVecsForRegionGraph(*model, model->getGraph()) ;

      // build a Bethe region graph (factor graph)
      lbRegionGraph * rg = new lbRegionGraph();
      rg->setCountingNumsFile (_countingNumsFile) ;
      rg->setBetheRegions (vecPair.first) ;
      //rg->print(cerr);
      inf = new lbRegionBP(*rg, *model, *_disp);
      delete rg ;
    }
    else {
      //Option (1) from above
      inf = new lbBeliefPropagation(*model, *_disp);
    }
  }
  else {
    //option (2) from above
    lbRegionGraph *rg = lbRegionBP::createRegionGraph(*model, _clusterFile);
    inf = new lbRegionBP(*rg, *model, *_disp);
    delete rg;
  }

  //Create monitors
  inf->createInferenceMonitor();
  inf->getInferenceMonitor()->setExact(model->getExactBeliefs());
  inf->getInferenceMonitor()->setOptions(opt, argc, argv);
  
  //Set queue type
  //0 is the unweighted queue (standard asynq queue)
  //1 is a weighted queue (see Elidan, McGraw & Koller UAI 06)
  //Other options are for grid models only
  if (opt.isOptionSetByUser("q")) {
    if (_queueType < 2) {
      inf->setQueueType((lbMessageQueueType) _queueType);
    }
    else {
      inf->setQueueType(MQT_MANUAL);
      lbGridQueueOrder gqo(model->getGraph());
      inf->setManualQueueOrder(gqo.getOrdering((gridQueueType) (_queueType-2)));
    }
  }
  
  inf->setOptions(opt, argc, argv);

  //read evidence file
  if (opt.isOptionSetByUser("e")) {
    readEvidenceFromFile(_evidenceFile, inf->getModel().getGraph().getNumOfVars());
  }

  //made sure we read all user input
  opt.ensureArgsHandled(argc, argv);

  //if user asked for printing all marginals: get number of variables (for each var we will print a marginal)
  if (_printMarginals == 0) {
    _printMarginals = inf->getModel().getGraph().getNumOfVars();
  }

  //if user asked for printing all cliques: get number of cliques (for each clique we will print a marginal belief)
  if (_printBeliefs == 0) {
    _printBeliefs = inf->getModel().getGraph().getNumOfCliques();
  }
    
  //print options that are being used
  opt.printOptions();

  //In case we want to compare with exact beliefs
  if (opt.isOptionSetByUser("exact")) {
    lbModel * emodel = NULL ;
    //Build junction tree (triangulate the model)
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

    //Create BP object on the resulting tree
    cerr << "Running exact inference..." << endl;
    lbBeliefPropagation* einf = new lbBeliefPropagation(*emodel,*_disp);
    cerr << "Finished running exact inference..." << endl;
    
    //Get exact beliefs for each clique
    int numCliques = model->getGraph().getNumOfCliques() ;
    assignedMesVec exactBeliefs (numCliques) ;
    for (int i = 0; i < numCliques; i++) {
      varsVec const& vars = model->getGraph().getVarsVecForClique(i);
      lbAssignedMeasure_ptr exact = einf->prob(vars);
      exactBeliefs[i] = exact ;
    }

    //save exact marginals on our original inference object (inf and not einf)
    inf->getInferenceMonitor()->setExact(exactBeliefs);    
    
    // Cleanup:
    for (int i = 0; i < numCliques; i++) {
      delete exactBeliefs[i] ;
    }

    delete einf;
    if (emodel!=NULL)
      delete emodel ;
  
  } // if exact inference

  //run BP
  inf->calcProbs();

  if (opt.isOptionSetByUser("trwopt")) {
    lbRegionBP * rbpInf = static_cast<lbRegionBP*> (inf) ;
    assert (rbpInf) ;
    trbp (rbpInf) ;
  }
  else if (opt.isOptionSetByUser("valopt")) {
    lbRegionBP * rbpInf = static_cast<lbRegionBP*> (inf) ;
    assert (rbpInf) ;
    varvalOpt (rbpInf) ;
  }

  return inf;
}


int main(int argc, char* argv[]) 
{
  clock_t startClock = 0 ;
  if (!_printTime2File.empty()) {
    startClock = clock() ;
  }
  timer t;
  t.start();

  //get inference object

  lbBeliefPropagation * inf = getInferenceObject(argc, argv);
  t.check("Right after getInferenceObject (which includes calcProbs())");

  //check whether bp converged 
  bool success = inf->calcProbs();
  t.check("Right after inf->calcProbs()");

  //In case the user asked for beliefs, print a belief for each clique (or one of them)
  if (_printBeliefs > 0) {
    cerr << endl << endl << "The first " << _printBeliefs << " beliefs: " << endl;
    inf->getInferenceMonitor()->printBeliefs(cerr, _printBeliefs);
  }

  //In case the user asked for marginals, print a marginal for each var (or one of them)
  if (_printMarginals > 0) {
    cerr << "The first " << _printMarginals << " marginals: " << endl;
    inf->getInferenceMonitor()->printMarginals(cerr, _printMarginals);
  }

  //print initial partition as approximated by the Free Energy approximation
  cerr << "Initial Partition: " << inf->initialPartitionFunction() << endl;

  //In case we have evidence, for each instnace:
  // - assign the evidence to the model
  // - print log likelihood and marginals after assigning the evidence
  timer t2;
  t.restart();
  for (int i = 0; i < (int) _evidence.size(); i++) {
    t2.restart();
    inf->changeEvidence(*(_evidence[i]));
    cerr << "Evidence: ";
    _evidence[i]->print(cerr, inf->getModel().getGraph().getNumOfVars());    
    cerr << "Current Partition: " << inf->partitionFunction() << endl;
    cerr << "Probability: " << inf->evidenceProb() << " (" << inf->evidenceLog2Prob() << ")" << endl << endl;
    if (_printMarginals > 0) {
      cerr << "The first " << _printMarginals << " marginals after evidence is set: " << endl;
      inf->getInferenceMonitor()->printMarginals(cerr, _printMarginals);
    }
    if (_printBeliefs > 0) {
      cerr << endl << endl << "The first " << _printBeliefs << " beliefs after evidence is set: " << endl;
      inf->getInferenceMonitor()->printBeliefs(cerr, _printBeliefs);
    }
    t2.check("After a piece of evidence");
  }
  t.check("Right after all the evidence has been gone through");

  if (success) {
    cerr << "SUCCEEDED" << endl;
  }
  else {
    cerr << "FAILED" << endl;
  }

  cerr << "messages(" << inf->getInferenceMonitor()->getMessageCount() << ") ";
  cerr << "update(" << inf->MessageBank().getTotalUpdated() << ") ";

  if (_printTime) {
    cerr << "time(" << inf->getInferenceMonitor()->getTimeLapse() << ")" << endl;
  }
  if (!_printTime2File.empty()) {
    ofstream timeFile (_printTime2File.c_str()) ;
    assert (timeFile) ;
    timeFile << ((double) (clock() - startClock) / CLOCKS_PER_SEC) ;
    timeFile.close() ;
  }

  lbTableMeasure<lbLogValue>::printStats();

  cerr << "DONE!" << endl ;
  delete inf;
  delete _driver;
  delete _disp;
  return 0;
}
