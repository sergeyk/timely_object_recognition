#include <lbDriver.h>
#include <lbModel.h>
#include <lbTableMeasure.h>
#include <lbBeliefPropagation.h>
#include <lbGSLLearningObject.h>
using namespace lbLib;


int main (int argc,char** argv) {
  if (argc != 3) {
    cout << "USAGE : suffStatTest <network file> <evidence>\n";
    exit(1);
  }

  long seed = 0;
  //  srand(seed);
  _lbRandomProbGenerator.Initialize(seed);
  
  lbMeasureDispatcher MD;
  MD.setType(MT_TABLE);
  lbDriver driver(MD);

  cout << "*** Reading network...\n";
  driver.readUniverse(argv[1]);
  cout << "*** Network read, model is:\n";
  lbModel& LBModel = driver.getModel();

  cout<<"*** creating inference object***"<<endl;
  lbBeliefPropagation* infObj = new lbBeliefPropagation(LBModel, MD);
  cout <<"*** Done "<<endl;

  cout<<"*** creating suff stat with evidence file ****" <<argv[2]<<endl;
  set<measIndex> * emptyMeasSet = new set<measIndex>() ;
  lbSuffStat* suffPtr = new lbSuffStat(*infObj,string(argv[2]),*emptyMeasSet);
  cout<<"*** done creating suff stat****" <<endl;

  cout<<" Read "<<suffPtr->getNumOfEvidence()<<" instances "<<endl;

  suffPtr->print(cout);

    
  lbGSLLearningObject* learner = new lbGSLLearningObject(LBModel, suffPtr, infObj, MD, emptyMeasSet);

  learner->learnEM(tGSLOptimizer::GRADIENT);

  uint numOfMeasures = LBModel.getNumOfMeasures();
  for (uint measInd=0;measInd<numOfMeasures;measInd++) {
    lbTableMeasure_Sptr empirCountsPtr(new lbTableMeasure<lbValue>(suffPtr->getEmpiricalExpectation(measInd)));
    empirCountsPtr->normalize();

    lbTableMeasure_Sptr estimCountsPtr(new lbTableMeasure<lbValue>(suffPtr->getEstimatedExpectation(measInd)));
    estimCountsPtr->normalize();

    cout<<"Checking diff (0.01) measure number "<<measInd<<" : "<<estimCountsPtr->isDifferentAVG(*empirCountsPtr,1e-2)<<endl; 
    cout<<"Checking diff (0.001) measure number "<<measInd<<" : "<<estimCountsPtr->isDifferentAVG(*empirCountsPtr,1e-3)<<endl; 
    cout<<"Checking diff (0.0001) measure number "<<measInd<<" : "<<estimCountsPtr->isDifferentAVG(*empirCountsPtr,1e-4)<<endl; 
  }
  
  delete learner;
  delete suffPtr;
  delete emptyMeasSet;
  delete infObj;

  cout << "***\n";
}
