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

#include <lbInferenceMonitor.h>
#include <lbBeliefPropagation.h>
#include <iomanip>

using namespace lbLib;

lbInferenceMonitor::lbInferenceMonitor(lbBeliefPropagation const * bp) :
  _bp(bp) {
  _lastUpdateT = 0;

  _statGapMessages = 100000000;
  _statGapTime = 100000000;
  _ticksSpentOnStats = 0;

  _outputMarg = "";
  _outputConvT = "";
  _outputConvM = "";
}

lbInferenceMonitor::~lbInferenceMonitor() {
  for (uint i = 0; i < _exactVec.size(); i++) {
    delete _exactVec[i];
  }
}

void lbInferenceMonitor::setOptions(lbOptions & opt, int argc, char *argv[]) {
  opt.addIntOption("Igm", &_statGapMessages, "messages between statistics collection");
  opt.addDoubleOption("Igt", &_statGapTime, "time between statistic collection");
  opt.addStringOption("Om", &_outputMarg, "output file for marginals over variables");
  opt.addStringOption("Oct", &_outputConvT, "output file for convergence as a function of time");
  opt.addStringOption("Ocm", &_outputConvM, "output file for convergence as a function of messages");
  opt.setOptions(argc, argv);
}

void lbInferenceMonitor::finishStatistics() {
  string suffix;

  //cerr << "Finishing stats." << endl;
  if (_bp->_propConverged) {
    suffix = ".success";
  }
  else {
    suffix = ".failure";
  }

  if (_outputMarg != "") {
    _outputMarg += suffix;
    ofstream outMarg(_outputMarg.c_str());
    printMarginals(outMarg);    
  }

  if (_outputConvM != "") {
    _outputConvM += suffix;
    ofstream outConvM(_outputConvM.c_str());
    printConvergenceM(outConvM);    
  }

  if (_outputConvT != "") {
    _outputConvT += suffix;
    ofstream outConvT(_outputConvT.c_str());
    printConvergenceT(outConvT);    
  }
}

void lbInferenceMonitor::updateStatistics() {
  clock_t startUpdate = clock();

  if (_bp->_messageCount % _statGapMessages == 0) {
    updateStatistics(false);
  }

  clock_t nowT = clock();
  if (((double) (nowT - _lastUpdateT))/CLOCKS_PER_SEC > _statGapTime) {
    updateStatistics(true);
    _lastUpdateT = nowT;  // Refresh last time.
  }

  clock_t endUpdate = clock();

  _ticksSpentOnStats += (endUpdate - startUpdate);
}

void lbInferenceMonitor::updateStatistics(bool forTime) {
  if (lbOptions::isVerbose(V_PROPAGATION)) {
    cerr << "Messages computed(" << _bp->_messageCount << "), "; 
    cerr << "updated(" << _bp->_messageBank->getTotalUpdated() << "), ";
    cerr << "dirty(" << _bp->_messageBank->getTotalDirty() << ")" << endl;
  }

  if (_exactVec.size() > 0) {
    probType err = averageKL();

    if (forTime) {
      if (_convergenceT.size() < 10000) {
	_convergenceT.push_back(err);
	double duration = ((double) (clock() - _bp->_timeStart) / CLOCKS_PER_SEC);
	_secondsCounts.push_back(duration);
      }
    }
    else {
      if (_convergenceM.size() < 10000) {
	_convergenceM.push_back(err);
	_messageCounts.push_back(std::make_pair(_bp->_messageCount,_bp->_messageBank->getTotalUpdated()));
      }
    }
  }
}

probType lbInferenceMonitor::averageKL() {
  probType totalError = 0;
  int count = 0;

  assert(_exactVec.size() > 0);

  for (rVarIndex var = 0; var < _bp->getModel().getGraph().getNumOfVars(); var++) {
    lbAssignedMeasure_ptr margbel = getBeliefMarginal(var);
    lbAssignedMeasure_ptr margexa = getExact(varsVec(1, var));
    
    probType error = margexa->getKL(*margbel);
    totalError += error;

    delete margbel;
    delete margexa;
    count++;
  }

  return (totalError / count);
}

lbAssignedMeasure_ptr lbInferenceMonitor::getBeliefMarginal(rVarIndex var) {
  lbAssignedMeasure_ptr bel = NULL;
  for (cliqIndex cliq = 0; cliq < _bp->getModel().getGraph().getNumOfCliques(); cliq++){
    varsVec vars = _bp->getModel().getGraph().getVarsVecForClique(cliq);

    bool found = false;
    for (int i = 0; i < (int) vars.size(); i++) {
      if (vars[i] == var) {
	bel = _bp->computeBelief(cliq);
	found = true;
	break;
      }
    }

    if (found) {
      break;
    }
  }

  assert(bel != NULL);
  varsVec margVar(1, var);

  lbAssignedMeasure_ptr marginal = bel->marginalize(margVar, _bp->getModel().measDispatcher());
  delete bel;
  return marginal;
}

lbAssignedMeasure_ptr lbInferenceMonitor::getExact(varsVec const & vars) {
  lbAssignedMeasure_ptr exact = NULL;

  for (int i = 0; i < (int) _exactVec.size(); i++) {
    if (vecSubset(vars, _exactVec[i]->getVars())) {
      exact = _exactVec[i];
      break;
    }
  }

  lbAssignedMeasure_ptr marginal = NULL;

  if (exact != NULL) {
    marginal = exact->marginalize(vars, _bp->getModel().measDispatcher());
  }

  return marginal;
}

void lbInferenceMonitor::printBeliefs(ostream & out) {
  printBeliefs(out, _bp->getModel().getGraph().getNumOfCliques());
}

// Print the first k beliefs.
void lbInferenceMonitor::printBeliefs(ostream & out, int k,bool withExact) {
  int numOfCliques = _bp->getModel().getGraph().getNumOfCliques();

  for (cliqIndex cliq=0; cliq < numOfCliques && cliq < k; cliq++) {
    cerr << "Belief:" << endl;
    lbAssignedMeasure_ptr bel =_bp->computeBelief(cliq);
    bel->print(cerr);

    lbAssignedMeasure_ptr exact = getExact(bel->getVars());

    if (withExact && exact != NULL) {
      cerr << "Exact:" << endl;
      exact->print(cerr);
      cerr << endl;
      cerr << "KL of belief " << cliq << " from exact: " << exact->getKL(*bel) << endl;
    }

    cerr << endl;
    delete bel;
    delete exact;
  }
}

void lbInferenceMonitor::printMarginals(ostream & out) {
  printMarginals(out, _bp->getModel().getGraph().getNumOfCliques());
}

void lbInferenceMonitor::printInfo(ostream & out) const {

}

void lbInferenceMonitor::printMessage(ostream & out, cliqIndex toCliq, cliqIndex fromCliq) const {

}

void lbInferenceMonitor::printAllMessages(ostream & out) const {

}

void lbInferenceMonitor::printIncomingMessages(ostream & out, cliqIndex toClique) const {

}

void lbInferenceMonitor::printAdjacentCliques(ostream & out, cliqIndex ofClique) const {

}

void lbInferenceMonitor::printAllAdjacentCliques(ostream & out) const {

}

void lbInferenceMonitor::printScope(ostream & out, cliqIndex fromClique, cliqIndex toClique) const {

}

void lbInferenceMonitor::printAllScopes(ostream & out) const {

}

void lbInferenceMonitor::printRelevancy(ostream & out) const {

}

void lbInferenceMonitor::printFrustration(ostream & out) const {

}

void lbInferenceMonitor::printMessageQueue(ostream & out) const {

}

void lbInferenceMonitor::printAssignedMeasure(ostream & out, cliqIndex cliq) const {

}

void lbInferenceMonitor::printAllAssignedMeasures(ostream & out) const {

}

void lbInferenceMonitor::printEvidence(ostream & out) const {

}

// Over single vars
void lbInferenceMonitor::printCurrentStatus() {

}

// local potentials and messages
void lbInferenceMonitor::printSpanningTreeToDotFile(ofstream& out) const {
      out << "graph G {" << endl;

    // print out all the nodes (cliques)
    for (uint i = 0; i < _bp->_localMessagesAdjList.size(); i++)
    {
	int card  = _bp->getModel().getGraph().getVarsVecForClique(i).size();
	out << "cliq" << i << "[ label = \"" << i << " (";
	for (int k = 0; k < card; k++)
	{
	    if (k > 0)
		out << " ";
	    out << _bp->getModel().getGraph().getVarsVecForClique(i)[k];
	}
	
	out << ")\"]" << endl;
    }

    // print out edges between cliques
    for (uint i = 0; i < _bp->_localMessagesAdjList.size(); i++)
    {
	for (uint j = 0; j < _bp->_localMessagesAdjList[i].size(); j++)
	{
          if ((int)i < _bp->_localMessagesAdjList[i][j])
	    {
		out << "cliq" << i<< "--cliq" << _bp->_localMessagesAdjList[i][j] <<"[ label = \"";

		for (uint k = 0; k < _bp->_localMessagesScopes[i][j].size(); k++)
		{
		    out << " " << _bp->_localMessagesScopes[i][j][k];
		}

		out << "\" ] ;" << endl;
	    }
	}
    }

    out << "}" << endl;
}

int lbInferenceMonitor::getMessageCount() const {
  return _bp->getMessageCount();
}

double lbInferenceMonitor::getTimeLapse() const  {
  return ((double) (_bp->_timeEnd - _bp->_timeStart)/CLOCKS_PER_SEC);
}

void lbInferenceMonitor::printExact(cliqIndex index) {

}

void lbInferenceMonitor::setExact(lbAssignedMeasurePtrVec const & exact) {
  for (uint i = 0; i < exact.size(); i++) {
    _exactVec.push_back(exact[i]->duplicateValues());
  }
}

void lbInferenceMonitor::printMarginals(ostream & out, int k) {
  out << setprecision(5);

  if (k > 0) {
    out << "# belief marginals / exact marginals / KL Divergence" << endl;
  }
    
  for (rVarIndex var = 0; var < _bp->getModel().getGraph().getNumOfVars() && var < k; var++) {
    lbAssignedMeasure_ptr margbel = getBeliefMarginal(var);

    out << var << "\t";

    int margsize = margbel->getMeasure().getSize();

    probType * margvals = new probType[margsize];

    int i;
    for (i = 0; i < margsize; i++) {
      margvals[i] = 0;
    }

    margbel->getMeasure().extractValuesAddToVector(margvals,0,false);

    for (i = 0; i < margsize; i++) {
      out << setw(15) << margvals[i] << "\t";
    }

    delete[] margvals;

    if (_exactVec.size() > 0) {
      lbAssignedMeasure_ptr margexa = getExact(varsVec(1, var));

      int exactsize = margexa->getMeasure().getSize();
      probType * exactvals = new probType[margsize];
      
      for (i = 0; i < exactsize; i++) {
	exactvals[i] = 0;
      }

      margexa->getMeasure().extractValuesAddToVector(exactvals,0,false);

      for (i = 0; i < exactsize; i++) {
	out << setw(15) << exactvals[i] << "\t";
      }

      out << setw(15) << margexa->getKL(*margbel);

      delete [] exactvals;
      delete margexa;
    }

    out << endl;

    delete margbel;
  }
}

void lbInferenceMonitor::printConvergenceM(ostream & out) const {
  out << "# Convergence in messages" << endl;
  out << "# ";

  if (!_bp->_propConverged) {
    out << "failure after ";
  }
  else {
    out << "success after ";
  }

  out << getTimeLapse() << " seconds and " << _bp->_messageCount << " messages " << endl << endl; 

  for (uint i = 0; i < _convergenceM.size(); i++) {
    assert(i < _messageCounts.size());
    out << _messageCounts[i].first << "\t";
    out << _messageCounts[i].second << "\t";
    out << setw(15) << _convergenceM[i] << endl;
  }
}

void lbInferenceMonitor::printConvergenceT(ostream & out) const {
  out << "# Convergence in time" << endl;
  out << "# ";

  if (!_bp->_propConverged) {
    out << "failure after ";
  }
  else {
    out << "success after ";
  }

  out << getTimeLapse() << " seconds and " << _bp->_messageCount << " messages " << endl << endl; 

  for (uint i = 0; i < _convergenceT.size(); i++) {
    assert(i < _messageCounts.size());
    out << _secondsCounts[i] << endl;
    out << setw(15) << _convergenceT[i] << "\t";
  }
}

