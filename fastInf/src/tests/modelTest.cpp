#include <lbDriver.h>
#include <lbModel.h>
#include <lbJunctionTree.h>
using namespace lbLib;


int main (int argc,char** argv) {
  if (argc != 2) {
    cout << "USAGE : modelTest <network file>\n";
    exit(1);
  }


  long seed = 0;
  //  srand(seed);
  _lbRandomProbGenerator.Initialize(seed);
  
  lbMeasureDispatcher MD;
  lbDriver driver(MD);

  vector<string> tempVec;
  for (int i=0;i<7;i++) 
    {
      std::ostringstream stream;
      stream << i;
      std::string iStr = stream.str();
      std::string varPref("var");
      std::string varName = varPref+iStr;
      tempVec.push_back(varName);
    }

  cardVec varsCards = cardVec(7,2);
  lbVarsList_ptr varsPtr = new lbVarsList();
  varsPtr->addVarList(tempVec);
  lbCardsList_ptr cardsPtr = lbCardsList_ptr(new lbCardsList(*varsPtr));
  cardsPtr->setCardsForAllVars(varsCards);


  lbGraphStruct_ptr graphPtr = lbGraphStruct_ptr(new lbGraphStruct(*varsPtr));
  varsVec vars0 = varsVec(3);
  varsVec vars1 = varsVec(2);
  varsVec vars2 = varsVec(2);
  varsVec vars3 = varsVec(1);
  varsVec vars4 = varsVec(3);
  vars0[0]=0;
  vars0[1]=1;
  vars0[2]=2;

  vars1[0]=1;
  vars1[1]=3;

  vars2[0]=1;
  vars2[1]=4;

  vars3[0]=4;
  
  vars4[0]=4;
  vars4[1]=5;
  vars4[2]=6;

  int index = graphPtr->addClique(vars0);
  index = graphPtr->addClique(vars1);
  index = graphPtr->addClique(vars2);
  index = graphPtr->addClique(vars3);
  index = graphPtr->addClique(vars4);
  graphPtr->addCliqueNeighbor(0,2);
  graphPtr->addCliqueNeighbor(1,2);
  graphPtr->addCliqueNeighbor(2,3);
  graphPtr->addCliqueNeighbor(2,4);

  

  lbModel* model1 = new lbModel(*graphPtr,*cardsPtr,MD);
  
  model1->addMeasure(MD.getNewMeasure(cardsPtr->getCardsVecForVarsVec(vars0),true));
  model1->addMeasure(MD.getNewMeasure(cardsPtr->getCardsVecForVarsVec(vars1),true));
  model1->addMeasure(MD.getNewMeasure(cardsPtr->getCardsVecForVarsVec(vars2),false));
  model1->addMeasure(MD.getNewMeasure(cardsPtr->getCardsVecForVarsVec(vars3),false));
  model1->addMeasure(MD.getNewMeasure(cardsPtr->getCardsVecForVarsVec(vars4),true));

  bool checkBool = model1->isMeasureAssignedForClique(0);
  assert(!checkBool);
  
  for (int i = 0  ; i<5 ; i++)
    model1->setMeasureForClique(i,i);

  checkBool = model1->isMeasureAssignedForClique(0);
  assert(checkBool);

  cerr<<"**** Originial model built****"<<endl;
  model1->getGraph().printGraphToFastInfFormat(cout);
  model1->printModelToFastInfFormat(cout);

  probType* probVec = new probType[26];
  for (int i = 0  ; i<26 ; i++)
    probVec[i] = _lbRandomProbGenerator.RandomProb();

  cerr<<"**** Changing params from log vector ****"<<endl;
  model1->updateLogParamsFromVector(probVec,true);
  model1->printModelToFastInfFormat(cout);

  cerr<<"****  Replacing measure 3****"<<endl;
  model1->replaceMeasure(MD.getNewMeasure(cardsPtr->getCardsVecForVarsVec(vars3),false),3);
  model1->printModelToFastInfFormat(cout);

  cerr<<"**** printing assigned measure 1 ****"<<endl;
  lbAssignedMeasure const& assMeas(model1->getAssignedMeasureForClique(1));
  assMeas.print(cout);

  cerr<<"****  print cardinalities for clique 2 ****"<<endl;
  cardVec printCardVec = model1->getCardForVars(vars2);
  printVector(printCardVec,cout);

  cerr<<"Getting cliques using measure 1*****"<<endl;
  cliquesVec const& printCliqVec(model1->getCliquesUsingMeasure(0));
  printVector(printCliqVec,cout);

  cerr<<"Check num of measures: "<<model1->getNumOfMeasures()<<endl;
  cerr<<"Check num of params: "<<model1->getSize()<<endl;
  cerr<<"Check num of params(2): "<<model1->getParamNum()<<endl;
  cerr<<"Check num of params(3): "<<model1->getParamNum()<<endl;

  int size;
  probType* probVec2 = model1->getLogParamVector(size,true);
  assert (size == model1->getParamNum());
  for (int j=0;j<model1->getParamNum();j++)
    cerr<<"Old vec: "<<probVec[j]<<" new vec: "<<probVec2[j]<<endl;
  delete model1;

  delete[] probVec;
  delete[] probVec2;
  
  cout << "*** Reading network...\n";
  driver.readUniverse(argv[1]);
  cout << "*** Network read, model is:\n";
  lbModel& LBModel = driver.getModel();
  LBModel.printVarsAndCardToFile(cout);
  LBModel.getGraph().printGraphToFastInfFormat(cout);
  LBModel.printModelToFastInfFormat(cout);

  cout << "Check triangulation"<<endl;
  lbModel * emodel = lbJunctionTree::CalcJunctionTreeGraphicalModel(&LBModel);
  emodel->printVarsAndCardToFile(cout);
  emodel->getGraph().printGraphToFastInfFormat(cout);
  emodel->printModelToFastInfFormat(cout);
  delete emodel;

  delete graphPtr;
  delete cardsPtr;
  delete varsPtr;
  

  cout << "***\n";
}
