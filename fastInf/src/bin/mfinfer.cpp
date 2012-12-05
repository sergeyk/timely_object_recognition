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

#include <lbMeasureDispatcher.h>
#include <lbDriver.h>
#include <lbTableMeasure.h>
#include <lbMeanField.h>
#include <lbOptions.h>
#include <sstream>
#include <sys/stat.h>

using namespace std;
using namespace lbLib;

lbDriver* _driver = NULL;
lbMeasureDispatcher* _disp = NULL;

int _printBeliefs = -1;
int _printMarginals = -1;
int _seed = -1;
string _inputFile = "none.net";
string _evidenceFile = "none.assign";
fullAssignmentPtrVec _evidence;
int _iterMethod = 0;
bool _testOrders = false;

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

void setOptions(lbOptions & opt, int argc, char *argv[]) {
  opt.addStringOption("i", &_inputFile, "input .net file");
  opt.addStringOption("e", &_evidenceFile, "input .assign file");
  opt.addIntOption("b", &_printBeliefs, "print the first N clique beliefs (0 for all)");
  opt.addIntOption("m", &_printMarginals, "print the first N singleton beliefs (0 for all)");
  opt.addIntOption("s",&_seed,"initialize the random seed (0 for time)");
  opt.addIntOption("M", &_iterMethod,"mean field iteration method (0-normal,1-residuals,2-MBresiduals,3-largest effect)");
  opt.addBoolOption("t", &_testOrders, "test random round-robin orders");


  opt.setOptions(argc, argv);
  
  if (!opt.isOptionSetByUser("i")) {
    opt.usageError("Must give a .net file");
  }
}

lbMeanField* getInferenceObject(int argc, char * argv[]) {
  lbModel_ptr model = NULL;
  lbOptions opt;
  setOptions(opt, argc, argv);

  if ( _seed == -1 ) 
    _seed = (int)time(NULL);
  _lbRandomProbGenerator.Initialize((long)_seed);
  _disp = new lbMeasureDispatcher(MT_TABLE);
  _driver = new lbDriver(*_disp);
  
  cerr << "Reading network... ";
  _driver->readUniverse(_inputFile.c_str());
  cerr << "done." << endl;
  
  model = &_driver->getModel();

  if (model == NULL) {
    cerr << "Error reading network: " << _inputFile << endl;
    exit(1);
  }

  lbMeanField* inf = new lbMeanField(*model,*_disp);
  inf->SetIterationMethod((lbMeanField::lbMFIterMethod)_iterMethod);

  if ( _testOrders ) {
    inf->testRandomOrderings();
    return 0;
  }

  if (opt.isOptionSetByUser("e")) {
    readEvidenceFromFile(_evidenceFile, inf->getModel().getGraph().getNumOfVars());
  }
  opt.ensureArgsHandled(argc, argv);

  if (_printMarginals == 0) {
    _printMarginals = inf->getModel().getGraph().getNumOfVars();
  }
  if (_printBeliefs == 0) {
    _printBeliefs = inf->getModel().getGraph().getNumOfCliques();
  }
  opt.printOptions();

  return inf;
}


int main(int argc, char* argv[]) 
{
  lbMeanField* inf = getInferenceObject(argc, argv);
  inf->calcProbs() ;

  if (_printBeliefs > 0) {
    cerr << endl << endl << "The first " << _printBeliefs << " beliefs: " << endl;
    for ( int i=0 ; i<_printBeliefs ; i++ )
      inf->prob(inf->getModel().getGraph().getVarsVecForClique(i))->print(cerr);
  }

  if (_printMarginals > 0) {
    cerr << "The first " << _printMarginals << " marginals: " << endl;
    inf->printMarginals(_printMarginals);
  }

  cerr << "Initial Partition: " << inf->initialPartitionFunction() << endl;

  for (int i = 0; i < (int) _evidence.size(); i++) {
    inf->changeEvidence(*(_evidence[i]));
    cerr << "Evidence: ";
    _evidence[i]->print(cerr, inf->getModel().getGraph().getNumOfVars());    
    cerr << "Probability: " << inf->evidenceProb() << " (" << inf->evidenceLog2Prob() << ")" << endl << endl;
    if (_printMarginals > 0) {
      cerr << "The first " << _printMarginals << " marginals after evidence is set: " << endl;
      inf->printMarginals(_printMarginals);
    }

  }

  cerr << "DONE!" << endl;
  delete inf;
  delete _driver;
  delete _disp;
  return 0;
}

