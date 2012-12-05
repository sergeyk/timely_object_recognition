#include <fstream>
#include <sstream>
#include <lbDebugging.h>
#include <lbWeightedTableMeasure.h>
#include <lbRandomProb.h>

using namespace lbLib;

void copyConstructionTest() {
  cerr << "Testing copy constuction. " << endl;

  cardVec card(3);
  card[0] = 2;
  card[1] = 3;
  card[2] = 2;

  lbTableMeasure<lbLogValue> ftm0(card, true);
  lbTableMeasure<lbValue> ftm1(ftm0);
  lbTableMeasure<lbValue> ftm2(ftm1);

  cerr << "The following tables should print the same: " << endl;

  ftm0.print(cerr);
  ftm1.print(cerr);
  ftm2.print(cerr);  

  lbTableMeasure<lbLogValue> ftm3(card);
  ftm3.makeZeroes();

  lbTableMeasure<lbValue> ftm4(ftm3);
  lbTableMeasure<lbValue> ftm5(ftm4);

  ftm3.print(cerr);
  ftm4.print(cerr);
  ftm5.print(cerr);  
}

void constructionTest() {
  cerr << "Testing construction." << endl;

  cardVec card(3);
  card[0] = 2;
  card[1] = 3;
  card[2] = 2;

  lbTableMeasure<lbLogValue> ftm0;
  lbTableMeasure<lbLogValue> ftm1(card, true);
  lbTableMeasure<lbLogValue> ftm2(card, false);
  // cerr << "About to copy a table...\n";
  lbTableMeasure<lbLogValue> ftm3(ftm2);
  // cerr << "Done!\n";
  
  lbTableMeasure<lbLogValue> ftm4((lbMultinomialMeasure &) ftm2);
  lbTableMeasure<lbLogValue> ftm5((lbMeasure &) ftm2);
  lbTableMeasure<lbLogValue> ftm6(ftm2);
  lbMeasure_Sptr ftmp0 = ftm2.duplicate();
  lbMeasure_Sptr ftmp1 = ((lbTableMeasure<lbLogValue> const &) ftm2).duplicate();
  

  ftm0.print(cerr);
  cerr << endl;
  ftm1.print(cerr);
  cerr << endl;
  ftm2.print(cerr);
  cerr << endl;
  ftm3.print(cerr);
  cerr << endl;
  
  ftm4.print(cerr);
  cerr << endl;
  ftm5.print(cerr);
  cerr << endl;
  ftm6.print(cerr);
  cerr << endl;
  ftmp0->print(cerr);
  cerr << endl;
  ftmp1->print(cerr);
  cerr << endl;
  
  cerr << "Finished testing construction." << endl;
}

void parameterTest() {
  cerr << "Testing parameterization." << endl;

  cardVec card(2);
  card[0] = 10;
  card[1] = 10;

  lbTableMeasure<lbValue> randomvals(card);
  randomvals.print(cerr);
  cerr << endl;
  
  safeVec<lbParameter> params;
  probType probs[10];
  probType logprobs[10];

  for (int i = 0; i < 10; i++) {
    probType value =  _lbRandomProbGenerator.RandomDouble(1.0);

    lbParameter param;
    param.setValue(0.1234);
    params.push_back(param);
    
    probs[i] = value;
    logprobs[i] = log(value);
  }

  lbFeatureTableMeasure ftm(card);

  ftm.setLogParams(params);

  varsVec vars(2);
  vars[0] = 0;
  vars[1] = 1;

  lbAssignment assign;
  assign.zeroise(vars);
  int count = 0;
  do {
    safeVec<int> pvec(1);
    pvec[0] = (count++)%10;
    ftm.setParameterIndicesOfFull(assign, vars, pvec);
  } while(assign.advanceOne(card, vars)); 
  
  ftm.setFeatures(randomvals);
  assert(ftm.isValid());
  ftm.print(cerr);

  int totalSet;
  bool changed;
  totalSet = ftm.setLogParams(probs, 0, changed);
 
  if (changed) {
    cerr << "Changed " << totalSet << " values." << endl;
  }
  else {
    cerr << "No change in table." << endl;
  }

  lbFeatureTableMeasure * ftmprint = ftm.multiply(ftm);
  ftmprint->print(cerr);
  cerr << endl;

}

void weightedTest() {

  cardVec card(2);
  card[0] = 10;
  card[1] = 10;

  varsVec vars(2);
  vars[0] = 0;
  vars[1] = 1;


  lbWeightedTableMeasure wtm(card);

  lbAssignment assign;
  assign.zeroise(vars);

  do {
    wtm.setFeatureValueOfFull(assign, vars, .1);
    assign.advanceOne(card,vars);
  } while(assign.advanceOne(card, vars));  

  wtm.setWeight(1.2);
  wtm.print(cerr);


  lbWeightedTableMeasure wtm2(card);

  assign.zeroise(vars);

  do {
    wtm2.setFeatureValueOfFull(assign, vars, .2);
    assign.advanceOne(card,vars);
  } while(assign.advanceOne(card, vars));  

  wtm2.setWeight(.7);
  wtm2.print(cerr);

  lbFeatureTableMeasure * ftmprint = wtm.multiply(wtm2);
  ftmprint->print(cerr);
}

int main (int argc,char** argv) {
  
  if (argc != 1) {
    cout<<"USAGE : featureMeasureTest"<<endl;
    exit(1);
  }
  
  int seed = 0;
  InitDebugging();
  

  _lbRandomProbGenerator.Initialize(seed);

  cerr << _lbRandomProbGenerator.RandomProb() << endl;

  cerr << "CTest...\n";
  constructionTest();
  cerr << "CCTest...\n";
  copyConstructionTest();
  cerr << "PTest...\n";
  parameterTest();
  cerr << "WTest...\n";
  weightedTest();
}
