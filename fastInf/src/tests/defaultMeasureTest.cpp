//#include <fstream>
//#include <sstream>
#include <lbTableMeasure.h>
#include <lbDebugging.h>

using namespace lbLib;

#define NUM_ENTRIES 1000

void prelimTest() {
  lbMeasureDispatcher * disp = new lbMeasureDispatcher(MT_TABLE);

  cardVec card = cardVec(3);
  card[0]=2;
  card[1]=3;
  card[2]=2;

  varsVec vars = varsVec(3);
  vars[0]=0;
  vars[1]=1;
  vars[2]=2;
  int numOfVarsForPrintNZ = 3;
  lbMeasure_Sptr sparse = disp->getNewMeasure(card);
  lbSmallAssignment assign(vars);

  assign.advanceOne(card, vars);
  sparse->setValueOfFull(assign, vars, 0);
  assign.advanceOne(card, vars);
  assign.advanceOne(card, vars);
  sparse->setValueOfFull(assign, vars, 0);
  assign.advanceOne(card, vars);
  sparse->setValueOfFull(assign, vars, 0);
  assign.advanceOne(card, vars);
  assign.advanceOne(card, vars);
  sparse->setValueOfFull(assign, vars, 0);
  assign.advanceOne(card, vars);
  sparse->setValueOfFull(assign, vars, 0);
  assign.advanceOne(card, vars);
  assign.advanceOne(card, vars);
  sparse->setValueOfFull(assign, vars, 0);
  assign.advanceOne(card, vars);
  sparse->setValueOfFull(assign, vars, 0);
  assign.advanceOne(card, vars);
  assign.advanceOne(card, vars);
  sparse->setValueOfFull(assign, vars, 0);
  
  lbMeasure_Sptr meas = sparse->duplicate();

  ((lbTableMeasure<lbLogValue> &) *sparse).printNZEs(numOfVarsForPrintNZ);
  
  cardVec card2 = cardVec(2);
  card2[0] = 2;
  card2[1] = 2;

  varsVec vars2 = varsVec(2);
  vars2[0] = 0;
  vars2[1] = 2;
  
  lbMeasure_Sptr smallMessage = disp->getNewMeasure(card2);
 
  cerr << "MESSAGE: " << endl;
  smallMessage->print(cerr);

  cerr << "SPARSE: " << endl;
  sparse->print(cerr);

  cerr << "MEAS: " << endl;
  meas->print(cerr);

  cerr << "margiplyOntoSparse: " << endl;
  smallMessage->marginalize(*sparse, vars, vars2, true);
  sparse->print(cerr);

  cerr << "standard marginalize and multiply: " << endl;
  smallMessage->marginalize(*meas, vars, vars2, true);
  meas->print(cerr);
  
  
  cardVec card3 = cardVec(4);
  card3[0] = 2;
  card3[1] = 3;
  card3[2] = 2;
  card3[3] = 3;

  varsVec vars3 = varsVec(4);
  vars3[0] = 2;
  vars3[1] = 100;
  vars3[2] = 0;
  vars3[3] = 1;

  lbMeasure_Sptr bigMessage = disp->getNewMeasure(card3);
  
  
  cerr << "MARGINALIZE TO SPARSE" << endl << endl;

  cerr << "margiplyOntoSparse: " << endl;
  bigMessage->marginalize(*sparse, vars, vars3, true);
  sparse->print(cerr);

  cerr << "standard marginalize and multiply: " << endl;
  bigMessage->marginalize(*meas, vars, vars3, true);
  meas->print(cerr);

  cerr << endl << endl << endl << endl;
  


  lbMeasure_Sptr small1 = smallMessage->duplicate();
  lbMeasure_Sptr small2 = smallMessage->duplicate();

  lbMeasure_Sptr big1 = bigMessage->duplicate();
  lbMeasure_Sptr big2 = bigMessage->duplicate();




  cerr << "MARGINALIZE ONLY FROM SPARSE" << endl << endl;
  
  cerr << "margiplyFromSparse: " << endl;
  sparse->marginalize(*small1, vars2, vars, false);
  small1->print(cerr);

  cerr << "standard marginalize and multiply: " << endl;
  meas->marginalize(*small2, vars2, vars, false);
  small2->print(cerr);
  
  
  cerr << "SPARSE: " << endl;
  sparse->print(cerr);
  cerr << endl;
  cerr << "BIG: " << endl;
  big1->print(cerr);
  cerr << endl;

  cerr << "sparseMargInto BIG : " << endl;
  sparse->marginalize(*big1, vars3, vars, false);
  big1->print(cerr);
  cerr << endl;

  cerr << "standardMargeInto BIG: " << endl;
  meas->marginalize(*big2, vars3, vars, false);
  big2->print(cerr);

  cerr << endl << endl << endl << endl;



 
  cerr << "MARGINALIZE AND MULTIPLY FROM SPARSE" << endl << endl;


  cerr << "margiplyFromSparse: " << endl;
  sparse->marginalize(*small1, vars2, vars, true);
  small1->print(cerr);

  cerr << "standard marginalize and multiply: " << endl;
  meas->marginalize(*small2, vars2, vars, true);
  small2->print(cerr);

  cerr << "margiplyFromSparse: " << endl;
  sparse->marginalize(*big1, vars3, vars, true);
  big1->print(cerr);


  cerr << "standard marginalize and multiply: " << endl;
  meas->marginalize(*big2, vars3, vars, true);
  big2->print(cerr);  

  cerr << endl << endl << endl << endl;

}

void bigTest() {
  cerr << "---------------------------------------------" << endl;
  cerr << "---------------------------------------------" << endl;
  cerr << "---------------------------------------------" << endl;
  cerr << "---------------------------------------------" << endl;

  lbMeasureDispatcher * disp = new lbMeasureDispatcher(MT_TABLE);

  cardVec card = cardVec(2);
  card[0] = NUM_ENTRIES;
  card[1] = NUM_ENTRIES;

  varsVec vars = varsVec(2);
  vars[0] = 0;
  vars[1] = 1;
 
  lbMeasure_Sptr sparse = disp->getNewMeasure(card);

  cardVec card3 = cardVec(1);
  card3[0] = NUM_ENTRIES;

  varsVec vars3 = varsVec(1);
  vars3[0] = 1;

  lbMeasure_Sptr message = disp->getNewMeasure(card3);
  


  cerr << "MARGINALIZE FROM SPARSE" << endl << endl << endl << endl << endl;
  cerr << "marginalizeFromSparse: " << endl;
  sparse->marginalize(*message, vars3, vars, false);
  cerr << "done" << endl;

  cerr << "marginalizeFromNormal: " << endl;
  sparse->marginalize(*message, vars3, vars, false);
  cerr << "done" << endl;
  
}

int main (int argc,char** argv) {
  InitDebugging();
  addVerbose(V_TEMPORARY);
  if (argc != 1) {
    cout<<"USAGE : defaultTestMeasure"<<endl;
    exit(1);
  }

  long seed = 0;
  _lbRandomProbGenerator.Initialize(seed);

  prelimTest();
  bigTest();
}
