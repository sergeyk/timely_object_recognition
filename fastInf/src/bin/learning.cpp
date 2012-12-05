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

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <lbDefinitions.h>
#include <lbRandomVar.h>
#include <lbMeasure.h>
#include <lbGraphStruct.h>
#include <lbModel.h>
#include <lbDriver.h>
#include <lbBeliefPropagation.h>
#include <GSLOptimizer.h>
#include <lbGSLLearningObject.h>
#include <lbFeatureTableMeasure.h>
#include <ObjectiveFunction.h>

/*!  This File is an example to how to learn model parameters from evidence
 */

using namespace std;
using namespace lbLib;

/*!
  Parameters:
 */

//inference threshold (when to decide the messages have converged)
double _infObjectThreshold = 1e-5;

//max em iter for chain graph (when we iterate between dir and undir learning)
int _emMaxIter = 15;

//Conjugate gradient type (see numerical recipes)
tGSLOptimizer::tProcType _gradAscendtMethod = tGSLOptimizer::FR;
int _gradAscendtMethodInt = 0;

//use smoothing on inference (when updating messages)
double _infSmooth = 0.5;

//Tables comparison type (Again, to determine convergnece)
//Can be 
// 0 - C_MAX - l_inifinity norm
// 1 - C_KL  - KL norm
// 2 - C_AVG - l_2 norm
lbMessageCompareType _compType = C_AVG ;
int _compTypeInt = 1;

//Queue type (0-unweighted, 1-weighted, 2-bars, 3-crisscross, 4-snakes)
//last three options are only for grids
lbMessageQueueType _queueType = MQT_WEIGHTED;
int _queueTypeInt =0;

//for converging the gradient ascent
double _learnEps = 1e-5 ;  //0.001;
double _learnStep = 0.05 ; //0.01;

//max gradient ascent iterations
int _learnIter = 100;

//input files:
string _inputFile = "none.net";
string _evidenceFile = "none.assign";
string _outputFile = "none.net";

//test derivatives
bool _runDTesting = false;
bool _runHTesting = false;

//print beliefs after learning
bool _printBeliefs = 0;

bool _changeToFeatureTable = false;


RegularizationType _regType = REG_NONE ;
double _regParam ;


//print tunning options
void printUsage(bool running = false) {

  char const* str = (running ? "" : "default ");

  if (running) {
    cerr << "--------------------------------------" << endl;
    cerr << "--------------------------------------" << endl;
    cerr << "--------------------------------------" << endl;
    cerr << "Running Learning With These Parameters" << endl;
    cerr << "--------------------------------------" << endl;
    cerr << "--------------------------------------" << endl;
    cerr << "--------------------------------------" << endl;
  }
  else {
    cerr << "Usage:" << endl;
    cerr << "Options: " << endl;
  }

  cerr << "-i [input file (" << str << _inputFile << ")]" << endl;
  cerr << "-e [evidence file (" << str << _evidenceFile << ")]" << endl;
  cerr << "-o [output net file (" << str << _outputFile << ")]" << endl;
  cerr << "-tD [run derivative test (" << str << _runDTesting << ")]" << endl;
  cerr << "-tH [run hessian test (" << str << _runHTesting << ")]" << endl;
  cerr << "-b [print the final beliefs (" << str << _printBeliefs << ")]" << endl;
  cerr << "-f [+|-] change to feature tables" << endl;
  cerr << "-g [+|-] optimize based on gradient or not" << endl;
  cerr << "-r1 [L1] parameter of L1 regularization" << endl;
  cerr << "-r2 [L2] parameter of L2 regularization" << endl;
  cerr << "-pT [inference threshold (" <<_infObjectThreshold<<")]"<<endl;
  cerr << "-pS [inference smoothing (" <<_infSmooth<<")]"<<endl;
  cerr << "-pC [inference compare 0-MAX, 1-KL 2-AVG (" <<_compTypeInt<<")]"<<endl;
  cerr << "-pQ [inference queue  0-Unweighted, 1-Weighted (" <<_queueTypeInt<<")]"<<endl;
  cerr << "-m [optimization method 0-FR, 1-PR, 2-BFGS, 3-STEEP, 4-NEWTON, 5-GRADIENT ("<<_gradAscendtMethodInt<<")]"<<endl;
  cerr << "-E [EM iterations (" << _emMaxIter << ")]" << endl;
  cerr << "-Le [Learning epsilon ("<<_learnEps<<")]"<<endl;
  cerr << "-Ls [Learning step ("<<_learnStep<<")]"<<endl;
  cerr << "-Li [Learning max iterations ("<<_learnIter<<")]"<<endl;
  cerr << "-v [verbosity type]" << endl << endl;
  cerr << "Verbosities: " << endl;
  printVerbosities();

  if (!running) {
    exit(1);
  }

  cerr << endl << endl;
}

//set running options (need to move to lbOption interface..)
void setOptions(int argc, char* argv[]) {
  bool infilefound = false; 

  for(int i = 1; i < argc; i++) {
    switch (argv[i][1]) {
    case 'f':
      if ( argv[i+1][0] == '+' )
	_changeToFeatureTable = true;
      else if ( argv[i+1][0] == '-' )
	_changeToFeatureTable = false;
      else
	assert(false);
      i++;
      break;
    case 'g':
      if ( argv[i+1][0] == '+' )
	ObjectiveFunction::setGradientBased(true);
      else if ( argv[i+1][0] == '-' )
	ObjectiveFunction::setGradientBased(false);
      else
	assert(false);
	  i++;
      break;
    case 'r':
      if ( argv[i][2] == '1' ) {
        _regType = REG_L1 ;
        _regParam = atof(argv[i+1]) ;
      }
      else if ( argv[i][2] == '2' ) {
        _regType = REG_L2 ;
        _regParam = atof(argv[i+1]) ;
      }
      else 
	assert(false);
      i++;
      break;
    case 'i':
      infilefound = true;
      _inputFile = argv[i+1];
      i++;
      break;
    case 'e':
      _evidenceFile = argv[i+1];
      i++;
      break;
    case 'o':
      _outputFile = argv[i+1];
      i++;
      break;
    case 'b':
      _printBeliefs = true;
      break;
    case 't':
      if ( argv[i][2] == 'H' )
	_runHTesting = true;
      else
	_runDTesting = true;
      break;
    case 'v':
      addVerbose((verbosity_type) atoi(argv[i+1]));
      i++;
      break;
    case 'p':
      if(argv[i][2]=='T'){
	_infObjectThreshold = atof(argv[i+1]);
	i++;
      }
      else if(argv[i][2]=='S'){
	_infSmooth = atof(argv[i+1]);
	i++;
      }
      else if(argv[i][2]=='Q'){
	_queueTypeInt = atoi(argv[i+1]);
	i++;
	if (_queueTypeInt==0){
	  _queueType = MQT_UNWEIGHTED;
	}
	else if (_queueTypeInt==1){
	  _queueType = MQT_WEIGHTED;
	}
	else
	  assert(false);
      }
      else if(argv[i][2]=='C'){
	_compTypeInt = atoi(argv[i+1]);
	i++;
	if (_compTypeInt==0) {
	  _compType=C_MAX;
	}
	else if(_compTypeInt==1) {
	  _compType=C_KL;
	}
	else if(_compTypeInt==2) {
	  _compType=C_AVG;
	}
	else {
	  assert(false);
	}
      }
      else {
	assert(false);
      }
      break;
    case 'E':
      _emMaxIter = atoi(argv[i+1]);
      assert(_emMaxIter >= 1);
      i++;
      break;
    case 'm':
      _gradAscendtMethodInt = atoi(argv[i+1]);
      if (_gradAscendtMethodInt==0)
	_gradAscendtMethod = tGSLOptimizer::FR;
      else if (_gradAscendtMethodInt==1)
	_gradAscendtMethod = tGSLOptimizer::PR;
      else if (_gradAscendtMethodInt==2)
	_gradAscendtMethod = tGSLOptimizer::BFGS;
      else if (_gradAscendtMethodInt==3)
	_gradAscendtMethod = tGSLOptimizer::STEEP;
      else if (_gradAscendtMethodInt==4)
	_gradAscendtMethod = tGSLOptimizer::NEWTON;
      else if (_gradAscendtMethodInt==5)
	_gradAscendtMethod = tGSLOptimizer::GRADIENT;
      else
	assert(false);
      i++;
      break;
    case 'L':
      if (argv[i][2]=='e') {
	_learnEps=atof(argv[i+1]);
	i++;
      }
      else if (argv[i][2]=='s') {
	_learnStep=atof(argv[i+1]);
	i++;
      }
      else if (argv[i][2]=='i') {
	_learnIter=atoi(argv[i+1]);
	i++;
      }
      else {
	cerr << endl << "Invalid option: " << argv[i][2] << endl << endl;
	printUsage();
      }
      break;
    default:
      cerr << endl << "Invalid option: " << argv[i][1] << endl << endl;
      printUsage();
    }
  }

  if (!infilefound) {
    cerr << endl << "Infile not found." << endl << endl;
    printUsage();
  }
}

/*
  Print beliefs from lerant model
 */
void printBeliefsFromModel(lbModel& model,lbMeasureDispatcher const& disp) {
  //create inf object, and set running options
  lbBeliefPropagation* infObj = new lbBeliefPropagation(model,disp);
  infObj->createInferenceMonitor();
  infObj->setCompareType(_compType); 
  infObj->setThreshold(_infObjectThreshold);
  infObj->setSmoothing(_infSmooth);
  infObj->setQueueType(_queueType);

  //print beliefs
  cerr << "BELIEFS:\n";
  int numOfCliques = model.getGraph().getNumOfCliques();
  for (cliqIndex cliq=0;cliq<numOfCliques;cliq++) {
    varsVec vec = model.getGraph().getVarsVecForClique(cliq);
    lbAssignedMeasure_ptr tmp = infObj->prob(vec);
    tmp->print(cout);
    delete tmp;
  }

  //print marginals
  cerr << endl << "MARGINALS:\n";
  infObj->getInferenceMonitor()->printMarginals(cerr, model.getGraph().getNumOfVars());

  delete infObj;
}

/*
Main method 
*/
int main(int argc, char* argv[]) {
  //read user preferences
  _lbRandomProbGenerator.Initialize(0); //time(NULL);
  setOptions(argc, argv);

  printUsage(true);

  //create and read model
  lbMeasureDispatcher* disp = new lbMeasureDispatcher(MT_TABLE);
  lbDriver_ptr driver(new lbDriver(*disp));
  driver->readUniverse(_inputFile);
  lbModel& model = driver->getModel();


  //use feature measures ?
  if ( _changeToFeatureTable ) {
    for ( int m=0 ; m<model.getNumOfMeasures() ; m++ ) {
      lbMeasure_Sptr meas(new lbFeatureTableMeasure((lbTableMeasure<lbLogValue> &)(model.getMeasure(m))));
      model.replaceMeasure(meas,m,model.getMeasureName(m));
    }
  }

  lbGSLLearningObject* learner;
    
  cerr << "Creating learning object" << endl;
  learner = new lbGSLLearningObject(_evidenceFile,model,*disp,
                                    _infObjectThreshold,_compType,_infSmooth,_queueType);
  // Regularization:
  if (_regType == REG_L1) {
    learner->setRegularizeParamL1 (_regParam) ;
  }
  else if (_regType == REG_L2) {
    learner->setRegularizeParamL2 (_regParam) ;
  }

  if (_runDTesting) {
    cerr << "Testing LL and derivs" << endl;
    learner->testLikeAndDeriv(1e-5);
  } else if ( _runHTesting ) {
    cerr << "Testing LL and derivs" << endl;
    learner->testDerivAndHess(1e-5);
  } else {
    cerr << "Learning...\n";
    learner->learnEM(_gradAscendtMethod,_learnEps,_learnStep,_learnIter,_emMaxIter);
  }
  
  delete learner;
  
  if (_outputFile != "none.net") {
    cerr<<"printing net to : "<<_outputFile.c_str()<<endl;
    driver->getModel().printAllNetToFile(_outputFile.c_str());
  }

  if (_printBeliefs) {
    cerr<<"----------------FINAL BELIEFS-------------------"<<endl;
    printBeliefsFromModel(driver->getModel(),*disp);
  }

  delete driver;
  delete disp;

  return 0;
 
}      
