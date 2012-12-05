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

#include <lbRegionBP.h>
#include <lbDriver.h>
#include <lbTableMeasure.h>
#include <lbBeliefPropagation.h>
#include <sstream>
#include <sys/stat.h>
#include <lbRandomProb.h>

using namespace std;
using namespace lbLib;

lbDriver * _driver = NULL;
lbMeasureDispatcher * _disp = NULL;

bool _random = false;
double _threshold = 1e-10;;
int _seed = 0;
string _inputFile = "none.net";
string _clusterFile = "none.net";
int _N = 100;

void setOptions(lbOptions & opt, int argc, char *argv[]) {
  opt.addStringOption("i", &_inputFile, "input .net file");
  opt.addStringOption("c", &_clusterFile, "input cluster file");
  opt.addIntOption("n",&_N,"set number of updates to sample");
  opt.addIntOption("s",&_seed,"initialize the random seed (0 for time)");

  for (int i = 0; i < V_MAX; i++) {
    opt.addVerboseOption(i, lbDefinitions::verbose_descriptions[i]);
  }

  opt.setOptions(argc, argv);
  
  if (!opt.isOptionSetByUser("i")) {
    opt.usageError("Must give a .net file");
  }
}

void printMessage(lbAssignedMeasure_ptr const& meas,ostream& O) {
  probType* vec = new probType[meas->getMeasure().getSize()];
  int v;
  for(v=0;v<meas->getMeasure().getSize();v++)
    vec[v] = 0.0;
  meas->getMeasure().extractValuesAddToVector(vec,0,false);
  for(v=0;v<meas->getMeasure().getSize();v++)
    O << " " << vec[v];
  delete[] vec;
}

lbBeliefPropagation * getInferenceObject(int argc, char * argv[]) {
  lbModel_ptr model = NULL;

  lbOptions opt;
  setOptions(opt, argc, argv);

  if ( _seed == 0 ) 
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

  // Testing sum product divide
  lbBeliefPropagation *inf = NULL;

  if (_clusterFile == "none.net") {
    inf = new lbBeliefPropagation(*model, *_disp);
  }
  else {
    lbRegionGraph *rg = lbRegionBP::createRegionGraph(*model, _clusterFile);
    inf = new lbRegionBP(*rg, *model, *_disp);
    delete rg;
  }

  inf->createInferenceMonitor();
  inf->getInferenceMonitor()->setExact(model->getExactBeliefs());
  inf->getInferenceMonitor()->setOptions(opt, argc, argv);

  inf->setOptions(opt, argc, argv);
  opt.ensureArgsHandled(argc, argv);
  opt.printOptions();
  inf->calcProbs();
  return inf;
}

int main(int argc, char* argv[]) 
{
  lbBeliefPropagation * inf = getInferenceObject(argc, argv);

  // determine order of messages
  messageMap::const_iterator it = inf->MessageBank().getRealMessages().begin();
  vector< messageIndex > miVec;
  for ( ; it!=inf->MessageBank().getRealMessages().end() ; it++ ) {
    cerr << "Messages[" << miVec.size() << "] " << it->first.first << " --> " << it->first.second << endl;
    miVec.push_back(it->first);
  }
  int M = miVec.size();

  for ( int n=0 ; n<_N ; n++ ) {
    inf->randomizeMessages();
    inf->MessageBank().printRealMessages(cerr);
    for ( int m=0 ; m<M ; m++ ) {
      lbAssignedMeasure_ptr meas = inf->computeMessage(miVec[m]);
      meas->normalize();
      cerr << m << " ";
      printMessage(meas,cerr);
      cerr << endl;
      delete meas;
    } // messages
  } // samples

  // clean up
  delete inf;
  delete _driver;
  delete _disp;
  return 0;
}

