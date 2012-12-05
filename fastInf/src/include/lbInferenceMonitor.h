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

#ifndef _Inference_Monitor_
#define _Inference_Monitor_

#include <lbDefinitions.h>
#include <lbAssignedMeasure.h>
#include <lbOptions.h>

namespace lbLib {
  class lbBeliefPropagation;

  class lbInferenceMonitor {
  public:
    lbInferenceMonitor(lbBeliefPropagation const * bp);
    ~lbInferenceMonitor();

    void updateStatistics();
    void finishStatistics();
    void updateStatistics(bool forTime);

    void setOptions(lbOptions & opt, int argc, char *argv[]);

    void printBeliefs(ostream & out);         // Over cliques
    void printBeliefs(ostream & out, int k,bool withExact = false);
    void compareBeliefs(ostream & out, int k);
    void printMarginals(ostream & out);       // Over vars
    void printMarginals(ostream & out, int k);

    void printInfo(ostream & out) const;
    void printMessage(ostream & out, cliqIndex toCliq, cliqIndex fromCliq) const;
    void printAllMessages(ostream & out) const;
    void printIncomingMessages(ostream & out, cliqIndex toClique) const;
    void printAdjacentCliques(ostream & out, cliqIndex ofClique) const;
    void printAllAdjacentCliques(ostream & out) const;
    void printScope(ostream & out, cliqIndex fromClique, cliqIndex toClique) const;
    void printAllScopes(ostream & out) const;
    void printRelevancy(ostream & out) const;
    void printFrustration(ostream & out) const;
    void printMessageQueue(ostream & out) const;
    void printAssignedMeasure(ostream & out, cliqIndex cliq) const;
    void printAllAssignedMeasures(ostream & out) const;
    void printEvidence(ostream & out) const;

    void printCurrentStatus(); // local potentials and messages
    void printSpanningTreeToDotFile(ofstream& out) const;

    int getMessageCount() const;
    double getTimeLapse() const;
    void printConvergenceT(ostream & out) const;
    void printConvergenceM(ostream & out) const;
    void printMarginalsClean(ostream & out) const;

    void setStatGapMessage(int gap) { _statGapMessages = gap; }
    void setStatGapTime(double gap) { _statGapTime = gap; }
    double getStatGapMessage() const { return _statGapMessages; }
    double getStatGapTime() const { return _statGapTime; }

    void printExact(cliqIndex index);
    void setExact(lbAssignedMeasurePtrVec const & exact);

  private:
    lbAssignedMeasurePtrVec _exactVec;

    probType averageKL();
    lbAssignedMeasure_ptr getBeliefMarginal(rVarIndex var);
    lbAssignedMeasure_ptr getExact(varsVec const & vars);

    lbBeliefPropagation const * _bp;

    vector<double> _convergenceM;
    vector<double> _convergenceT;
    vector< pair<double,double> > _messageCounts;
    vector<double> _secondsCounts;

    string _outputMarg;
    string _outputConvT;
    string _outputConvM;

    double _statGapTime;
    int _statGapMessages;

    clock_t _ticksSpentOnStats;
    clock_t _lastUpdateT;
  };
};

#endif
