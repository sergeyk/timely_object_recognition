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
#include "lbDefinitions.h"
#include "lbRandomVar.h"
#include "lbMeasure.h"
#include "lbGraphStruct.h"
#include "lbModel.h"
#include "lbDriver.h"
#include "lbLoopyInf.h"
using namespace std;

using namespace lbLib;


void printMarginalsfromModel(lbModel& model) {
  lbInferenceObject_ptr infObj = new lbLoopyInf(model);
  varsVec vec = varsVec(1);
  for (rVarIndex var=0;var<model.getGraph().getNumOfVars();var++) {
    vec[0]=var;
    lbAssignedMeasure_ptr tmp = infObj->prob(vec);
    cout<<"marginal for var "<<var<<" is: "<<endl;
    tmp->print(cout);
  }
  delete infObj;
  
}

void printBeliefsFromModel(lbModel& model) {
  lbInferenceObject_ptr infObj = new lbLoopyInf(model);
  int numOfCliques = model.getNumOfCliques();
  for (cliqIndex cliq=0;cliq<numOfCliques;cliq++) {
    varsVec vec = model.getVarsVecForClique(cliq);
    lbAssignedMeasure_ptr tmp = infObj->prob(vec,cliq);
    tmp->print(cout);
  }
  delete infObj;
}

/*
Main method 
*/
int main(int argc, char* argv[]) {

  cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ BEGIN TEST"<<endl;
  lbDriver_ptr driver(new lbDriver());

  if (argc < 2)
    driver->exitDriver(string("USAGE: lbDriver inputFileName \n"));


  string fileName =string(argv[1]);
  //  string assignFileName = string(argv[2]);
  driver->readUniverse(fileName);
  printMarginalsfromModel(driver->getModel());
  delete driver;
  cout<<"after driver dtor"<<endl;

  return 0;
 
}      
