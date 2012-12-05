#include <lbGraphStruct.h>
#include <string>
//#include <lbTableMeasure.h>

using namespace lbLib;

void Test(int argc,char** argv) {

  vector<string> tempVec;
  for (int i=0;i<10;i++) 
    {
      std::ostringstream str;
      str << "var" << i;
      tempVec.push_back(str.str());
    }

  lbVarsList_ptr varsPtr = new lbVarsList();
  varsPtr ->addVarList(tempVec);
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
  assert(index==0);

  cout<<"adding cliques"<<endl;
  index = graphPtr->addClique(vars1);
  assert(index==1);

  index = graphPtr->addClique(vars2);
  assert(index==2);

  index = graphPtr->addClique(vars3);
  assert(index==3);

  index = graphPtr->addClique(vars4);
  assert(index==4);

  cout<<"first print"<<endl;
  graphPtr->printGraph(cout);

  cout<<"check isVarInClique"<<endl;
  bool checkBool = graphPtr->isVarInClique(0,1);
  assert(checkBool==1);

  checkBool = graphPtr->isVarInClique(0,4);
  assert(checkBool==0);

  cout<<"check copy ctor (1)"<<endl;
  lbGraphStruct_ptr graphPtr2 = new lbGraphStruct(*graphPtr);
  
  graphPtr2->printGraph(cout);

  cout<<"check add all neighbors"<<endl;
  graphPtr2->addNeighboursToAllCliques();

  graphPtr2->printGraph(cout);

  delete graphPtr2;
  
  cout<<"check add var to clique"<<endl; 
  graphPtr->addRandomVarToClique(0,7);
  graphPtr->addRandomVarToClique(1,7);

  graphPtr->printGraph(cout);

  cout<<"check remove var to clique "<<endl;
  graphPtr->removeRandomVarFromClique(0,7);
  graphPtr->printGraph(cout);

  cout<<"add neighbors manually:"<<endl;
  graphPtr->addCliqueNeighbor(0,2);
  graphPtr->addCliqueNeighbor(1,2);
  graphPtr->addCliqueNeighbor(2,3);
  graphPtr->addCliqueNeighbor(2,4);

  graphPtr->printGraph(cout);

  cout<<"check remove neighbor"<<endl;
  bool retVal = graphPtr->removeCliqueNeighbor(0,2);
  assert(retVal);
  graphPtr->printGraph(cout);
 
  graphPtr->addCliqueNeighbor(0,2);

  int numOfCls = graphPtr->getNumOfCliques();
  assert(numOfCls==5);

  cliquesVec const& clsVec1 = graphPtr->getAllCliquesForVar(0);
  cliquesVec const& clsVec2 = graphPtr->getAllCliquesForVar(4);
  
  cout <<"Cliques for var 0: "<<endl;
  for (uint i=0;i<clsVec1.size();i++)
    cout<<clsVec1[i]<<" ";
  cout<<endl;

  cout <<"Cliques for var 4: "<<endl;
  for (uint i=0;i<clsVec2.size();i++)
    cout<<clsVec2[i]<<" ";
  cout<<endl;


  cout <<"check running intersection property:"<<endl;
  bool checkRIP = graphPtr->runningIntersectionSatisfied(4);
  assert(checkRIP);
  graphPtr->removeRandomVarFromClique(2,4);
  checkRIP = graphPtr->runningIntersectionSatisfied(4);
  assert(!checkRIP);
  delete graphPtr;
  //  delete varsPtr;
  cerr<<"Test OK"<<endl;
}


int main (int argc,char** argv) {
  if (argc != 1) {
    cout<<"USAGE : graphTest"<<endl;
    exit(1);
  }

  Test(argc, argv);

}
