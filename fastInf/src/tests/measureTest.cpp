//#include <fstream>
//#include <sstream>
//#include <lbDebugging.h>
#include <lbTableMeasure.h>
#include <lbMeasureDispatcher.h>
#include <lbAssignedMeasure.h>

using namespace lbLib;

static int randomness = 0;
void hugeTest(int argc,char** argv) {

  lbMeasureDispatcher * disp = new lbMeasureDispatcher(MT_TABLE_NOLOG);

  long seed = 0;
  //  srand(seed);
  _lbRandomProbGenerator.Initialize(seed);
  
  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";
  
  //try one measure :
  cardVec card = cardVec(3);
  card[0]=2;
  card[1]=3;
  card[2]=2;
  lbMeasure_Sptr mes1 = disp->getNewMeasure(card);
  mes1->print(cout);

  int len = mes1->getSize();
  probType* dVec = new probType[len];
  int i;
  for ( i=0 ; i<len ; i++ )
    dVec[i] = 0.0;
  //extracting feature values
  mes1->extractValuesAddToVector(dVec,0,false);
  cout<<"printing vec :"<<endl;

  for (i=0;i<len;i++) {
    cout<<i<<", "<<dVec[i]<<endl;
    dVec[i] = 0.0;
  }
  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";

  //extracting feature log values
  mes1->extractValuesAddToVector(dVec,0,true);
  cout<<"printing log vec :"<<endl;
  for (i=0;i<len;i++)
    cout<<i<<", "<<dVec[i]<<endl;

  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";
  cout<<"Making sure change detecion works"<<endl;
  bool changed=true;
  //updating from log because update takes exponent of vector)
  mes1->setLogParams(dVec,0,changed);
 mes1->print(cout);
  cout<<"should be 0: "<<changed<<endl;
  dVec[2]=dVec[2]*12;
  mes1->setLogParams(dVec,0,changed);
  cout<<"should be 1: "<<changed<<endl;
  cout<<"after reset: "<<endl;
  mes1->makeZeroes();
  mes1->print(cout);
  delete[] dVec;
  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";
  cerr<<"\n\n---------------------------------now assigned measres-----------"<<endl;
  int num=7;
  ifstream_ptr in(new ifstream(argv[1]));
  //(*in) >> num;
  vector< lbMeasure_Sptr> vec(num);
  paramPtrMap sharedParams = paramPtrMap();
  paramPtrMap idleParams = paramPtrMap();
  for (int i = 0;i<num;i++) {
    cerr <<"Reading measure "<<i<<endl;

    char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
    (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE,lbDefinitions::DELIM);
    string str(buffer.get());
    istringstream iss(str);
    iss >> str;
    

    lbMeasure_Sptr mes = disp->getNewMeasure();
    mes->readOneMeasure(in,sharedParams,idleParams);
    mes->normalize();
    vec[i]=mes;
    //delete buffer;
  }

  //print
  cout << "Printing measures " <<endl;
  for (int i = 0;i<num;i++) {
    vec[i]->print(cout);
    cout<<endl;
  }

  vector< lbAssignedMeasure_ptr > assignVec(num);
  //building var vectors
  varsVec vars3 = varsVec(3);
  varsVec vars2 = varsVec(2);
  varsVec vars1 = varsVec(1);

  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";
  //build assigned measures :
  vars3[0]=0;
  vars3[1]=1;
  vars3[2]=2;
  assignVec[0] = new lbAssignedMeasure(vec[0],vars3);

  vars3[0]=2;
  vars3[1]=3;
  vars3[2]=4;
  assignVec[1] = new lbAssignedMeasure(vec[1],vars3);

  vars3[0]=5;
  vars3[1]=4;
  vars3[2]=1;
  assignVec[2] = new lbAssignedMeasure(vec[2],vars3);

  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";
  vars2[0]=5;
  vars2[1]=6;
  assignVec[3] = new lbAssignedMeasure(vec[3],vars2);

  vars2[0]=6;
  vars2[1]=7;
  assignVec[4] = new lbAssignedMeasure(vec[4],vars2);

  vars2[0]=6;
  vars2[1]=4;
  assignVec[5] = new lbAssignedMeasure(vec[5],vars2);

  vars1[0]=4;
  assignVec[6] = new lbAssignedMeasure(vec[6],vars1);
 
  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";
  cout << "Printing Assigned measures " <<endl;
  for (int i = 0;i<num;i++) {
    assignVec[i]->print(cout);
    cout<<endl;
  }
  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";

  

  cerr<<"\ntest marginalize into new measure: "<<endl;
  vars2[0]=0;
  vars2[1]=1;
  //  vars3[2]=2;
  lbAssignedMeasure_ptr newAssMeas = assignVec[0]->marginalize(vars2,*disp);

  newAssMeas->print(cout);
  delete newAssMeas;
  
  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";
  vars2[0]=1;
  vars2[1]=2;
  newAssMeas = assignVec[0]->marginalize(vars2,*disp);
  newAssMeas->print(cout);
  delete newAssMeas;
  
  vars2[0]=0;
  vars2[1]=2;
  newAssMeas = assignVec[0]->marginalize(vars2,*disp);
  newAssMeas->print(cout);
  delete newAssMeas;
  
  
  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";
  cerr<<"\nmarginalize into existing measure : "<<endl;
  assignVec[1]->marginalize(*assignVec[6],vars1);
  assignVec[6]->print(cerr);

  assignVec[2]->marginalize(*assignVec[6],vars1);
  assignVec[6]->print(cerr);

  assignVec[5]->marginalize(*assignVec[6],vars1);
  assignVec[6]->print(cerr);
  
  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";
  cerr<<"\nmarginalize and multiply into existing measure : "<<endl;
  newAssMeas = assignVec[6]->duplicate();
  newAssMeas->print(cout);
  assignVec[1]->marginalizeAndMultiply(*newAssMeas,vars1);
  assignVec[2]->marginalizeAndMultiply(*newAssMeas,vars1);
  assignVec[5]->marginalizeAndMultiply(*newAssMeas,vars1);
  newAssMeas->normalize();
  newAssMeas->print(cout);
  delete newAssMeas;

  cerr<<"\n\ntest duplicate, setValue , getValue, smooth And Update"<<endl;

  cerr << "\n\nRANDOM" << randomness++ << ": " << _lbRandomProbGenerator.RandomDouble(1.0) << "\n\n";
  cardVec card2 = cardVec(2);
  card2[0]=3;
  card2[1]=2;
  
  vars2[0]=5;
  vars2[1]=6;
  lbAssignment_ptr small = new lbAssignment();
  small->zeroise(vars2);

  lbMeasure_Sptr mes = disp->getNewMeasure(card2);
  newAssMeas = new lbAssignedMeasure(mes,vars2);
  cerr<<"random"<<endl;
  newAssMeas->print(cout);
  probType counter=0.0;
  cerr<<"rising"<<endl;
  do {
    newAssMeas->setValueOfFull(*small,counter);
    counter++;
  } while (small->advanceOne(card2,vars2));
  newAssMeas->print(cout);
  lbAssignedMeasure_ptr newAssMeas1 = newAssMeas->duplicate();
  newAssMeas1->makeZeroes();
  cerr<<"zero"<<endl;
  newAssMeas1->print(cout);
  cerr<<"orig"<<endl;
  newAssMeas->print(cout);
  cerr<<"smooth param 0"<<endl;
  newAssMeas1->updateMeasureValues(newAssMeas->getMeasure(),0);
  newAssMeas1->print(cout);

  newAssMeas1->makeZeroes();
  cerr<<"smooth param 0.25"<<endl;
  newAssMeas1->updateMeasureValues(newAssMeas->getMeasure(),0.25);
  newAssMeas1->print(cout);

  
  newAssMeas1->makeZeroes();
  cerr<<"smooth param 0.5"<<endl;
  newAssMeas1->updateMeasureValues(newAssMeas->getMeasure(),0.5);
  newAssMeas1->print(cout);

  newAssMeas->normalize();
  cerr<<"\n\ntesting is different"<<endl;
  probType LOC_EPS = (4.5e-16);
  //  lbMessageCompareType type = C_MAX;
  cerr<<"Comparing max against self : "<<newAssMeas->isDifferent(*newAssMeas,C_MAX,LOC_EPS)<<endl;
  cerr<<"Comparing avg against self : "<<newAssMeas->isDifferent(*newAssMeas,C_AVG,LOC_EPS)<<endl;
  cerr<<"Comparing kl against self : "<<newAssMeas->isDifferent(*newAssMeas,C_KL,LOC_EPS)<<endl;

  delete newAssMeas1;
  newAssMeas1 = newAssMeas->duplicate();
  do {
    newAssMeas1->setLogValueOfFull(*small,newAssMeas->logValueOfFull(*small)+1e-4);
  } while (small->advanceOne(card2,vars2));
  newAssMeas1->normalize();
  cerr<<"Comparing max (should be 0) : "<<newAssMeas->isDifferent(*newAssMeas1,C_MAX,LOC_EPS)<<endl;
  cerr<<"Comparing avg (should be 0) : "<<newAssMeas->isDifferent(*newAssMeas1,C_AVG,LOC_EPS)<<endl;
  cerr<<"Comparing kl (should be 0) : "<<newAssMeas->isDifferent(*newAssMeas1,C_KL,LOC_EPS)<<endl;
  //  newAssMeas1->print(cerr);

  newAssMeas1->setValueOfFull(*small,newAssMeas->valueOfFull(*small)+ LOC_EPS*2);
  //newAssMeas1->normalize();
  //newAssMeas1->print(cerr);
  cerr<<"Comparing max (should be 1) : "<<newAssMeas->isDifferent(*newAssMeas1,C_MAX,LOC_EPS)<<endl;
  cerr<<"Comparing avg (should be 0) : "<<newAssMeas->isDifferent(*newAssMeas1,C_AVG,LOC_EPS)<<endl;
  //cerr<<"Comparing kl (should be 1) : "<<newAssMeas->isDifferent(*newAssMeas1,C_KL,6e-17)<<endl;
  
  do {
    newAssMeas1->setLogValueOfFull(*small,newAssMeas->logValueOfFull(*small)+1e-5);
  } while (small->advanceOne(card2,vars2));
  newAssMeas1->normalize();
  cerr<<"Comparing max (should be 0) : "<<newAssMeas->isDifferent(*newAssMeas1,C_MAX,LOC_EPS)<<endl;
  cerr<<"Comparing avg (should be 0) : "<<newAssMeas->isDifferent(*newAssMeas1,C_AVG,LOC_EPS)<<endl;
  cerr<<"Comparing kl (should be 0) : "<<newAssMeas->isDifferent(*newAssMeas1,C_KL,LOC_EPS)<<endl;

  // This code used to add 1000 to the log value of every possible assignment and then
  // normalize (but this actually doesn't do anything because that's just multiplying
  // by a constant and newAssMeas is normalized.  Now I just do it to the second assignment
  // The first assignment is 0 so adding 1000 to -inf is still -inf.
  small->advanceOne(card2,vars2);
  newAssMeas1->setLogValueOfFull(*small,newAssMeas->logValueOfFull(*small)+1000);

  newAssMeas1->normalize();
  cerr<<"Comparing max (should be 1) : "<<newAssMeas->isDifferent(*newAssMeas1,C_MAX,LOC_EPS)<<endl;
  cerr<<"Comparing avg (should be 1) : "<<newAssMeas->isDifferent(*newAssMeas1,C_AVG,LOC_EPS)<<endl;
  cerr<<"Comparing kl (should be 1) : "<<newAssMeas->isDifferent(*newAssMeas1,C_KL,LOC_EPS)<<endl;

  counter =1;
  cerr<<"test KL "<<endl;
  small->zeroise(vars2);

  do {
    newAssMeas->setValueOfFull(*small,counter);
    newAssMeas1->setValueOfFull(*small,counter+3);
    counter++;
  } while (small->advanceOne(card2,vars2));
  newAssMeas->normalize();
  newAssMeas1->normalize();

  newAssMeas->print(cout);
  newAssMeas1->print(cout);

  cerr<<" is diff KL "<<newAssMeas->isDifferent(*newAssMeas1,C_KL,LOC_EPS)<<endl;;

  assignVec[0]->print(cerr);
  lbAssignment_ptr small1 = new lbAssignment();
  small1->setValueForVar(0,0);
  delete newAssMeas;  
  
  cerr<<"\ntest marginalize into new measure: "<<endl;
  vars3[0]=0;
  vars3[1]=1;
  vars3[2]=2;

  newAssMeas = assignVec[0]->marginalize(vars3,*disp);
  newAssMeas->print(cout);
  delete newAssMeas;
  
  vars2[0]=0;
  vars2[1]=1;
  newAssMeas = assignVec[0]->marginalize(vars2,*small1,*disp);
  newAssMeas->print(cout);
  delete newAssMeas;
  
  vars2[0]=1;
  vars2[1]=2;
  newAssMeas = assignVec[0]->marginalize(vars2,*small1,*disp);
  newAssMeas->print(cout);
  delete newAssMeas;
  
  vars2[0]=2;
  vars2[1]=0;
  newAssMeas = assignVec[0]->marginalize(vars2,*small1,*disp);
  newAssMeas->print(cout);
  delete newAssMeas;
  delete newAssMeas1;

  //delete ass. measures
  for (int i = 0;i<num;i++) {
    delete assignVec[i];
  }
  delete small;
  delete small1;
  delete disp;
}

void normalizeTest() {

  lbMeasureDispatcher * disp = new lbMeasureDispatcher(MT_TABLE);
  cardVec card = cardVec(3);
  card[0]=2;
  card[1]=3;
  card[2]=2;
  lbMeasure_Sptr mes1 = disp->getNewMeasure(card);
  mes1->print(cout);

  mes1->normalize();
  lbTableMeasure<lbLogValue>::printStats();
  mes1->print(cout);

  mes1->normalize();
  lbTableMeasure<lbLogValue>::printStats();
  delete disp;
}

int main (int argc,char** argv) {
  if (argc != 2) {
    cout<<"USAGE : measureTest file_with_measures"<<endl;
    exit(1);
  }

  //InitDebugging();
  normalizeTest();
  hugeTest(argc, argv);
}
