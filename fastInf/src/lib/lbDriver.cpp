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


#include <lbDriver.h>

using namespace lbLib;


const int lbDriver::ERROR_MODE = 1;

const string lbDriver::VARS_STR = "@Variables";

const string lbDriver::MEASURE_STR = "@Measures";
  
const string lbDriver::CLIQUES_STR = "@Cliques";
      
const string lbDriver::CLIQUE_TO_MEASURE_STR = "@CliqueToMeasure";

const string lbDriver::IDLE_PARAMS = "@IdleParams";

const string lbDriver::SHARED_PARAMS = "@SharedParams";

const string lbDriver::DIRECTED_MEASURES_LIST = "@DirectedMeasures";

const string lbDriver::EXACT_STR = "@Exact";

lbDriver::lbDriver(lbMeasureDispatcher const& disp) :
  _measDisp(disp)
{
  _model=NULL;
  _cards=NULL;
  _graph=NULL;
  _vars = NULL;
  //  _measDisp = new lbMeasureDispatcher(MT_TABLE);
}

lbDriver::~lbDriver() {
  
  if(_model)
    delete _model;

  if(_graph)
    delete _graph;

  if (_cards)
    delete _cards;

  if (_vars)
    delete _vars;

  //  delete _measDisp;
}

bool lbDriver::readVariables(ifstream_ptr in) {
  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
  //cout<<"reading vars"<<endl;
  (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
  string str(buffer.get());
  //rVarIndex varInd=0 ;
  int card;
  //lbRandomVar_ptr rVarPtr;
  vector<string> varsVector = vector<string>();
  cardVec varsCards = cardVec();
  while(!(str==lbDefinitions::END_STR)) {
    (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
    str = string(buffer.get());
    istringstream iss(str);
    iss >> str;
    if (str[0]!='@') {
      iss >> card;
      varsCards.push_back(card);
      varsVector.push_back(str);
      //varInd++;
    }
  }
  _vars = lbVarsList_ptr(new lbVarsList());
  _vars->addVarList(varsVector);
  _cards= lbCardsList_ptr(new lbCardsList(*_vars));
  _cards->setCardsForAllVars(varsCards);
  //  _cards->print(cout);
  //cout<<"finished reading"<<endl;
  //  delete[] buffer;
  return true;
}

bool lbDriver::readExact(ifstream_ptr in, lbModel *model) {
  assignedMesVec amv;
  paramPtrMap sharedParams;
  paramPtrMap idleParams;

  for (int i = 0; i < model->getGraph().getNumOfCliques(); i++) {
    varsVec const& vars = model->getGraph().getVarsVecForClique(i);
    lbMeasure_Sptr measPtr = model->measDispatcher().getNewMeasure();
    measPtr->readOneMeasure(in,sharedParams,idleParams);
    lbAssignedMeasure_ptr assnmes = new lbAssignedMeasure(measPtr, vars);
    amv.push_back(assnmes);
  }

  string str;
  (*in) >> str;
  model->setExactBeliefs(amv);
  return true;
}

//this method will read all the vars-measures-cliques from an input
// file (look at example input file)
bool lbDriver::readUniverse(string inputFileName, bool readLogValues) {
  //  cout<<"reading univ"<<endl;
  //open in stream
  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
  ifstream_ptr in(new ifstream(inputFileName.c_str()));

  if (!*in)
    return false;
 
  while (!(*in).eof() && (*in).good()) {
    
    string str;
    (*in) >> str;
    if (str == "") {
      break;
    }

    if (str[0]=='#') {
      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
    }
    else {
      
      if (str==VARS_STR) {
	//cout<<"reading variables"<<endl;	
	if (!readVariables(in)) {
	  exitDriver(string("illegal input - failed reading vars"));
	  break;
	}
	_graph = lbGraphStruct_ptr(new lbGraphStruct(*_vars));
	_model = lbModel_ptr(new lbModel(*_graph,*_cards,_measDisp));
      }
      else if (str==MEASURE_STR) {
	//cout<<"reading measures"<<endl;
	//	_model = lbModel_ptr(new lbModel(*_graph,*_cards,_measDisp));
	if (!_model->readMeasures(in,readLogValues)) {
	  exitDriver(string("illegal input - failed reading measures"));
	  break;
       	}
      }
      else if (str==CLIQUES_STR) {
	//cout<<"reading cliques"<<endl;
	if (!_graph->readCliques(in)) { 
	  exitDriver(string("illegal input - failed reading cliques")); 
	  break;
       	}
	//_graph->printGraph(cerr);
      }
      else if (str==CLIQUE_TO_MEASURE_STR) {
	//cout<<"reading measure to cliques"<<endl;
	if (!_model->assignMeasureForCliques(in)) {
	  exitDriver(string("illegal input - failed reading cliques measures"));
	  break;
       	}
      }
      else if (str==IDLE_PARAMS) {
	if (!_model->readSharedAndIdleParams(in,MP_IDLE)) {
	  exitDriver(string("illegal input - failed reading idle params"));
	  break;
       	}
      }
      else if (str==SHARED_PARAMS) {
	if (!_model->readSharedAndIdleParams(in,MP_SHARED)) {
	  exitDriver(string("illegal input - failed reading idle params"));
	  break;
       	}
      }
      else if (str==DIRECTED_MEASURES_LIST) {
	if (!_model->readDirectedMeasuresList(in)) {
	  exitDriver(string("illegal input - failed reading idle params"));
	  break;
       	}
      }
      else if (str==EXACT_STR) {
	if (!readExact(in, _model)) {
	  exitDriver(string("illegal input - failed reading regions"));
	  break;
	}
      }
      else {
	exitDriver(string("illegal input file"));
	break;
      }
    }
  }

  //  _graph->printGraph(cerr);
  //  ofstream* ofp = new ofstream("graph.dot");
  //_graph->printToDotFile(*ofp);
  //ofp->close;
  //delete ofp;
  //  _model->printAllNetToFile("NET12");
  in->close();
  //delete in;
  //delete[] buffer;
  return true;
}

void lbDriver::exitDriver(string msg) {
  cerr << msg << endl;
  
  if (_vars)
    delete _vars;
  
  if (_cards)
    delete _cards;
  
  if(_model)
    delete _model;
  
  if(_graph)
    delete _graph;
  
  //exit(ERROR_MODE);
}

lbModel & lbDriver::getModel(){
  //  _model->printModel(cerr);
  //lbModel* m(new lbModel(*_model));
  //return *m;
  return *_model;
}

lbGraphStruct & lbDriver::getGraph(){
  return *_graph;
}
/*
Main method 
*/
//int main(int argc, char* argv[]) {
/*
//} 

  lbDriver* driver = new lbDriver();

  if (argc!=2)
    driver->exitDriver(string("USAGE: lbDriver inputFileName \n"));
  
  string fileName =string(argv[1]);

  driver->readUniverse(fileName);

  delete driver;
  
  return 0;
 
}      
*/

void lbDriver::writeUniverse(lbModel const& model, string outputFileName,
			     bool writeLogValues) {
  ofstream ofs(outputFileName.c_str());
  lbDriver::writeUniverse(model,ofs,writeLogValues);
}

void lbDriver::writeUniverse(lbModel const& model, ostream& os,
			     bool writeLogValues) {
  model.printModelToFastInfFormat(os,false,5,writeLogValues);
}

lbVarsList & lbDriver::getVars() {
  return *_vars;
}

lbCardsList & lbDriver::getCards() {
  return *_cards;
}
