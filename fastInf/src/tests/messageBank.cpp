#include "lbGridQueue.h"
#include <lbDriver.h>
#include <lbDefinitions.h>
#include <lbRandomProb.h>
#include <lbWeightedQueue.h>
#include <lbManualQueue.h>
#include <lbWeightedMessageBank.h>
#include <lbUnweightedMessageBank.h>
#include <map>
#include <vector>
 
using namespace lbLib;
 
typedef map<messageIndex, double, lessMessageIndex> mmap;

#define ITERATIONS 100

string _fileName;
lbMeasureDispatcher *_disp;

lbAssignedMeasure_ptr getRandomMessage() {
  varsVec vars(2);
  vars[0] = 10;
  vars[1] = 11;

  cardVec card(2);
  card[0] = 2;
  card[1] = 2;

  lbMeasure_Sptr mes = _disp->getNewMeasure(card);
  lbAssignedMeasure_ptr assignedmes = new lbAssignedMeasure(mes, vars);

  return assignedmes;
}

void iteration(lbMessageBank * bank, bool same) {
  messageIndexVec miv = bank->getIteration();
  
  cerr << "Iteration size: " << miv.size() << endl; 

  for (uint i = 0; i < miv.size(); i++) {
    messageIndex mi = miv[i];
    lbAssignedMeasure_ptr newmessage;

    if (same) {
      cerr << "same " << endl;
      newmessage = bank->getMessage(mi)->duplicateValues();
    }
    else {
      newmessage = getRandomMessage();
    }

    newmessage->normalize();
    bank->setMessage(mi, newmessage);
  }

  cerr << "Dirty messages: " << bank->getTotalDirty() << endl;
  bank->update();
}

void setup(lbMessageBank * bank, adjListVec const & neighbors) {
  for (uint i = 0; i < neighbors.size(); i++) {
    for (uint j = 0; j < neighbors[i].size(); j++) {
      messageIndex mi(i, neighbors[i][j]);
      bank->initMessage(mi, getRandomMessage());
    }
  }
}

void testBank(lbMessageBank * bank, bool weighted) {
  for (uint i = 0; i < ITERATIONS; i++) {
    iteration(bank, false);
  }

  cerr << "Starting repeat: " << endl;
  for (uint i = 0; i < 6; i++) {
    iteration(bank, true);
  }

  assert(bank->getTotalDirty() == 0);
}
 
void testQueue() {
  lbUnweightedQueue uq;

  for (int i = 0; i < 100; i++) {
    messageIndex mi = messageIndex(i, i+1);
    uq.push(mi, 0);
  }

  uq.push(messageIndex(1, 2), 0);
  assert(uq.size() == 100);

  while (!uq.empty()) {
    messageIndex mi = uq.pop();
    cerr << mi.first << "," << mi.second << endl;
  }

  lbWeightedQueue wq;
  mmap weights;
  for (int i = 0; i < 100; i++) {
    messageIndex mi = messageIndex(i, i+1);
    double weight = _lbRandomProbGenerator.RandomProb();
    wq.push(mi, weight);
    weights[mi] = weight;
  }

  wq.push(messageIndex(35, 36), 0);
  assert(wq.size() == 100);

  cerr << "TOP: " << endl;
  messageIndexVec few = wq.top(10);
  for (uint i = 0; i < few.size(); i++) {
    cerr << few[i].first << "," << few[i].second << endl;
  }
  cerr << endl;

  while (!wq.empty()) {
    messageIndex mi = wq.pop();
    cerr << mi.first << "," << mi.second << ": " << weights[mi] << endl;
  }
}

void testManualQueue() {
  lbManualQueue mq;

  messageIndexVec miv;
  for (int i = 0; i < 100; i++) {
    miv.push_back(messageIndex(i, i+1));
  }

  mq.setMessageOrder(miv);
  assert(mq.size() == 0);

  for (int i = 0; i < 100; i++) {
    mq.push(messageIndex(i, i+1), 0);
  }

  for (int i = 0; i < 50; i++) {
    messageIndex mi = mq.pop();
  }

  for (int i = 0; i < 25; i++) {
    mq.push(messageIndex(i, i+1), 0);
  }
  
  mq.print(cerr);
  assert(mq.size() == 75);
}

void testGridQueue() {
  lbMeasureDispatcher * disp;
  lbDriver * driver;
  lbModel_ptr model = NULL;

  disp = new lbMeasureDispatcher(MT_TABLE);
  driver = new lbDriver(*disp);

  driver->readUniverse(_fileName.c_str());
  model = &driver->getModel();

  lbGridQueue gq(model->getGraph(), (gridQueueType) 2);
  gq.print(cerr);

  delete disp;
  delete driver;
}

void testBank(int updateSize, bool weighted) {
  adjListVec neighbors(3);

  neighbors[0].push_back(1);
  neighbors[0].push_back(2);
  neighbors[1].push_back(0);
  neighbors[1].push_back(2);
  neighbors[2].push_back(1);
  neighbors[2].push_back(0);
 
  lbMessageBank * bank;
  if (weighted) {
    bank = (lbMessageBank *) new lbWeightedMessageBank(neighbors);
  }
  else {
    bank = (lbMessageBank *) new lbUnweightedMessageBank(neighbors);
  }

  setup(bank, neighbors);
  bank->setSmoothing(10e-20);
  bank->setUpdateSize(updateSize);

  testBank(bank, weighted);

  delete bank;
}

int main(int argc, char *argv[]) {
  _disp = new lbMeasureDispatcher(MT_TABLE);
  _lbRandomProbGenerator.Initialize(0);

  bool weighted = false;
  int updateSize = 1;

  lbOptions opt;
  opt.addStringOption("i", &_fileName, "grid file name for queue test");
  opt.addBoolOption("w", &weighted, "weighted");
  opt.addIntOption("u", &updateSize, "updateSize [1-6]");

  for (int i = 0; i < V_MAX; i++) {
    opt.addVerboseOption(i, lbDefinitions::verbose_descriptions[i]);
  }
  opt.setOptions(argc, argv);
  opt.ensureArgsHandled(argc, argv);

  testGridQueue();
  //  testBank(updateSize, weighted);

  delete _disp;
}
