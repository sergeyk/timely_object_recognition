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
#include <lbDefinitions.h>
#include <lbRandomVar.h>
#include <lbMeasure.h>
#include <lbGraphStruct.h>
#include <lbModel.h>
#include <lbDriver.h>
#include <lbBeliefPropagation.h>
#include <lbRandomProb.h>
#include <iomanip>
#include <lbMathFunctions.h>
using namespace std;

using namespace lbLib;


cliquesVecVector cacheCliqBelongings(lbModel const& model,lbGraphStruct const& graph) {
  int numOfVars = graph.getNumOfVars();
  cliquesVecVector cacheVec = cliquesVecVector(numOfVars);
  for (rVarIndex vi = 0 ; vi < numOfVars ; vi++) {
    varsVec thisV = varsVec(1);
    thisV[0]=vi;
    cacheVec[vi] = cliquesVec();
    for (cliqIndex ci = 0 ; ci < graph.getNumOfCliques() ; ci++) {
      varsVec cVars = graph.getVarsVecForClique(ci);
      if (vecSubset(thisV,cVars)) {
	cacheVec[vi].push_back(ci);
      }
    }
  }
  return cacheVec;
}

lbAssignedMeasure_ptr probFull(rVarIndex var,double temp,
			       lbModel const& model,
			       lbFullAssignment & assign,
			       cliquesVecVector const& cacheVec,
			       lbMeasureDispatcher const& disp,
                               bool metropolis)
{
  varsVec thisV = varsVec(1);
  thisV[0]=var;
  cardVec thisCards = model.getCardForVars(thisV);
  lbMeasure_Sptr resMeas = disp.getNewMeasure(thisCards,false);
  lbAssignedMeasure_ptr result = new lbAssignedMeasure(resMeas,thisV);
  varValue prevValue = assign.getValueForVar(var) ;  // for metropolis
  assign.setValueForVar(var,0);
  do {
    probType entryProb=1;
    for (cliqIndex ciInd=0;
	 ciInd<(int)cacheVec[var].size();
	 ciInd++)
    {
      cliqIndex ci = cacheVec[var][ciInd];
      lbAssignedMeasure const& cliqueAM = model.getAssignedMeasureForClique(ci);
      probType thisProb = cliqueAM.valueOfFull(assign);
      entryProb*=thisProb;
    } 
    result->setValueOfFull(assign,entryProb);
  } while (assign.advanceOne(thisCards,thisV));
  if (metropolis) {
    assign.setValueForVar(var,prevValue) ;  // restore previous value (undo set to 0)
  }
  result->raiseToThePower((1.0)/temp);
  result->normalize();
  return result;
}

void stepAssignment (lbAssignment& assign,
		     lbModel const& model,
		     lbMeasureDispatcher const& disp,
		     int numOfVars,
		     cliquesVecVector const& cacheVec,
		     double temp,
                     bool metropolis)
{
  rVarIndex randRVar = (rVarIndex)_lbRandomProbGenerator.RandomInt(numOfVars);
  if (randRVar>=numOfVars) {
    cerr<<"ERROR, var : "<<randRVar<<" num of vars: "<<numOfVars<<endl;
  }
  lbAssignedMeasure_ptr probResult = probFull(randRVar,temp,model,assign,cacheVec,disp,metropolis);
  varValue resultVal = probResult->randomValFromTable();
  
  if (!metropolis) {
    assign.setValueForVar(randRVar,resultVal);  // always accept
  }
  else { // metropolis
    // acceptance probability is:  min {1, [1-p(xi|X\Xi)]/[1-p(x'i|X\Xi)]}
    probType curAssignProb = probResult->valueOfFull (assign) ;  // = p(xi|X\Xi)
    
    lbAssignedMeasure_ptr newValResult = probResult->duplicate() ;
    newValResult->setValueOfFull (assign, 0) ;  // only consider new values (normalized 4 lines down)
    newValResult->normalize() ;
    resultVal = newValResult->randomValFromTable() ;  // this is guaranteed to be different from the current value
    delete newValResult ;
    varValue prevVal = assign.getValueForVar (randRVar) ;
    assign.setValueForVar (randRVar, resultVal) ; // Xi = x'i
    probType nextAssignProb = probResult->valueOfFull (assign) ; // = p(x'i|X\Xi)

    double acceptanceProb = Min (1, (1-curAssignProb)/(1-nextAssignProb)) ;
    /*cout << "[DEBUG] var=" << randRVar
      << " prevVal=" << prevVal
      << " prevProb=" << curAssignProb
      << " proposedVal=" << resultVal
      << " proposedProb=" << nextAssignProb
      << " acceptance prob: " << acceptanceProb << endl ;*/
    double randNum = _lbRandomProbGenerator.RandomDouble(1) ;
    if (randNum > acceptanceProb) { // don't accept:
      assign.setValueForVar (randRVar, prevVal) ;  // restore previous value
    }
  }

  delete probResult;
}

void runAssignments (double period,
                     bool periodInMins,
		     lbAssignment& assign,
		     lbModel const& model,
		     lbMeasureDispatcher const& disp,
		     int numOfVars,
		     cliquesVecVector const& cacheVec,
		     double temp,
                     bool metropolis)
{
  if (!periodInMins) {
    for (int time=0;time<period;time++) {
      stepAssignment (assign,model,disp,numOfVars,cacheVec,temp,metropolis) ;
    }
  } // if period not in minutes
  else {
    time_t startTime = time(NULL) ;
    time_t curTime = time(NULL) ;
    int CHECK_INTERVAL = 100 ;
    int iterations = 0 ;
    while (true) {
      stepAssignment (assign,model,disp,numOfVars,cacheVec,temp,metropolis) ;
      iterations++ ;
      if (iterations % CHECK_INTERVAL == 0) {
        curTime = time(NULL) ;
        /*
        cerr << "[DEBUG] iterations=" << iterations
             << " curTime-startTime = " << curTime << "-" << startTime << "="
             << curTime-startTime << endl ;
        */
        if (curTime-startTime > period*60) {
          cerr << "[INFO] Time limit exceeded, ran " << iterations << " iterations in " << period << " minutes." << endl ;
          break ;
        }
      }
    } // while time not passed
  }
}


class gibbsConvergeStatistic {

  typedef vector<double>    doubleVec ;
  typedef vector<doubleVec> doubleMat ;

public:
  gibbsConvergeStatistic (safeVec<lbAssignment_ptr> & assignsVec,
                          lbModel const& model,
                          lbMeasureDispatcher const& disp,
                          double convergeThresh = 1.05)
    : _model_(model),
      _numOfSamples_(0),
      _convergenceThreshold_(convergeThresh)
  {
    _numOfChains_ = assignsVec.size() ;
    _numOfStatistics_ = model.getSize() ;

    _totalMu  = doubleVec (_numOfStatistics_,0.0) ;
    _totalSum = doubleVec (_numOfStatistics_,0.0) ;
    _mu       = doubleMat (_numOfStatistics_) ;
    _sum      = doubleMat (_numOfStatistics_) ;
    _sum2     = doubleMat (_numOfStatistics_) ;
    for (int s=0 ; s<_numOfStatistics_ ; ++s) {
      _mu[s]   = doubleVec (_numOfChains_,0.0) ;
      _sum[s]  = doubleVec (_numOfChains_,0.0) ;
      _sum2[s] = doubleVec (_numOfChains_,0.0) ;
    }

    updateStats (assignsVec) ; // read initial assignments
  }

  void updateStats (safeVec<lbAssignment_ptr> & assignsVec) {

    assert (_numOfChains_ == (int)assignsVec.size()) ;

    //cout << "[DEBUG] In updateStats\n" ;

    _numOfSamples_++ ;

    int runningInd = 0 ;
    for (measIndex meas=0 ; meas<_model_.getNumOfMeasures() ; ++meas) {

      //cout << "[DEBUG] meas = " << meas << endl ;

      measurePtrVec measCounts (_numOfChains_) ;
      for (int i=0 ; i<_numOfChains_ ; ++i) {
        measCounts[i] = _model_.getMeasure(meas).duplicate() ;
        measCounts[i]->makeZeroes() ;
      }

      cliquesVec const & cliques4meas = _model_.getCliquesUsingMeasure(meas) ;
      for (uint i=0 ; i<cliques4meas.size() ; ++i) {

        cliqIndex cliq = cliques4meas[i] ;
        lbAssignedMeasure const & cliqueAM = _model_.getAssignedMeasureForClique(cliq);
        
        //cout << "[DEBUG] Clique = " << cliq << endl ;

        for (int j=0 ; j<_numOfChains_ ; ++j) {
          lbAssignedMeasure_ptr dupCliqAM = cliqueAM.duplicate() ;
          dupCliqAM->updateAssign (*(assignsVec[j])) ;
          //cout << "[DEBUG] measure counts:\n" ; dupCliqAM->getMeasure().print (cout) ;
          measCounts[j]->addMeasure (dupCliqAM->getMeasure()) ;
          delete dupCliqAM ;
        } // for chain
      } // for cliq

      int measSize = 0 ;
      for (int j=0 ; j<_numOfChains_ ; ++j) {

        probVector flatCounts = measCounts[j]->measure2Vec() ;
        //cout << "[DEBUG] Chain=" << j << " counts=(" << vec2str(flatCounts,",") << ")\n" ;
        int curMeasSize = flatCounts.size() ;
        if (j==0) {
          measSize = curMeasSize ;
        } else {
          assert (curMeasSize == measSize) ;
        }

        for (int k=0 ; k<measSize ; ++k) {

          // update sum_i and sum2_i:
          double statCount = flatCounts[k] ;
          int stat = runningInd + k ;

          _sum[stat][j]   += statCount ;
          _sum2[stat][j]  += pow(statCount,2.0) ;
          _mu[stat][j]     = _sum[stat][j] / _numOfSamples_ ;
          _totalSum[stat] += statCount ;

        } // for measure entry
      } // for chain

      runningInd += measSize ;

      // update _totalMu:
      for (int stat=0 ; stat<_numOfStatistics_ ; ++stat) {
        _totalMu[stat] = 1.0/(_numOfChains_*_numOfSamples_) * _totalSum[stat] ;
      }
      
      // DEBUG:
      //cout << "[DEBUG] After measure " << meas << ", stats:\n" ; printStats(cout) ;

    } // for meas
    
    // cout << "[DEBUG] After update, stats:\n" ; printStats(cout) ;
  }

  bool didConverge() {
    // Taken from "Markov Chain Monte Carlo In Practice"/ Gilks, Richardson and Spiegelhalter (1996), p. 137.
    // Gelman and Rubin 1992.

    // if we had a single statistic to compute over m chains and n samples in each chain
    // with average mu, chain average mu_i, chain sum S_i, and chain square sum S2_i:
    //
    // B = n/(m-1) * sum_i (mu_i - mu)^2
    // W = 1/(m*(n-1)) * sum_i [S2_i - 2*mu_i*S_i + n*mu_i^2]
    // R = (n-1)/n * W + 1/n * B
    //
    // and we check whether R is close to 1.0

    bool converged = true ;
    for (int stat=0 ; stat<_numOfStatistics_ ; ++stat) {
      double B = 0 ;
      for (int i=0 ; i<_numOfChains_ ; ++i) {
        B += pow (_mu[stat][i]-_totalMu[stat],2.0) ;
      }
      B *= (double) _numOfSamples_/(_numOfChains_-1) ;
      
      double W = 0 ;
      for (int i=0 ; i<_numOfChains_ ; ++i) {
        W += (_sum2[stat][i] - 2*_mu[stat][i]*_sum[stat][i] + _numOfSamples_*pow(_mu[stat][i],2.0)) ;
      }
      W *= 1.0/(_numOfChains_*(_numOfSamples_-1)) ;
      
      double estimVar = (double)(_numOfSamples_-1)/_numOfSamples_ * W + 1.0/_numOfSamples_ * B ;

      double R = estimVar/W ;

      //cout << "[DEBUG] Checking convergence for statistic " << stat
      //     << ": estimVar=" << estimVar << ", W=" << W << ", R=" << R << endl ;

      if (R > _convergenceThreshold_) {
        converged = false ;
        break ; // no need to check other statistics
      }
    }

    return converged ;
  }

private:
  void printStats (ostream & out) {
    out << "Printing statistics of convergence tester:\n" ;
    out << " Number of chains: " << _numOfChains_ << "\n" ;
    out << " Number of samples: " << _numOfSamples_ << "\n" ;
    out << " Number of statistics: " << _numOfStatistics_ << "\n" ;
    out << " totalMu: (" << vec2str(_totalMu,",") << ")\n" ;
    out << " Mus:\n" ;
    for (uint s=0 ; s<_mu.size() ; ++s) {
      out << "  Mu["<<s<<"] = (" << vec2str(_mu[s],",") << ")\n" ;
    }
    out << " totalSum: (" << vec2str(_totalSum,",") << ")\n" ;
    out << " sums:\n" ;
    for (uint s=0 ; s<_sum.size() ; ++s) {
      out << "  sum["<<s<<"] = (" << vec2str(_sum[s],",") << ")\n" ;
    }
    out << " sums2:\n" ;
    for (uint s=0 ; s<_sum2.size() ; ++s) {
      out << "  sum2["<<s<<"] = (" << vec2str(_sum2[s],",") << ")\n" ;
    }
  }

private:
  const lbModel & _model_ ;

  int _numOfChains_ ;
  int _numOfSamples_ ;

  int _numOfStatistics_ ;

  double _convergenceThreshold_ ;

  // statistics:
  doubleVec _totalMu ;
  doubleMat _mu ;
  doubleVec _totalSum ;
  doubleMat _sum ;
  doubleMat _sum2 ;

} ;

void runWithConvergeTest (double maxPeriod,
                          bool periodInMins,
                          safeVec<lbAssignment_ptr> & assignsVec,
                          lbModel const& model,
                          lbMeasureDispatcher const& disp,
                          int numOfVars,
                          cliquesVecVector const& cacheVec,
                          double temp,
                          bool metropolis)
{
  gibbsConvergeStatistic * convergeStatistics = new gibbsConvergeStatistic (assignsVec, model, disp) ;
  /*
  // DEBUG:
  model.printVarsAndCardToFile (cout) ;
  model.getGraph().printGraphToFastInfFormat (cout) ;
  model.printModelToFastInfFormat (cout) ;
  */

  int CONVERGE_INTERVAL = 50 ;
  int TIME_INTERVAL = 10 ;

  time_t startTime = time(NULL) ;
  time_t curTime = startTime ;
  int iterations = 0 ;
  bool done = false ;

  while (!done) {

    for (int ind=0 ; ind<(int)assignsVec.size() ; ++ind) {
      stepAssignment (*(assignsVec[ind]),model,disp,numOfVars,cacheVec,temp,metropolis) ;
    }
    convergeStatistics->updateStats (assignsVec) ;

    iterations++ ;

    if (iterations % CONVERGE_INTERVAL == 0) {  // we might not want to check convergence in every iteration
      if (convergeStatistics->didConverge()) {
        cerr << "[INFO] Convergence test passed, ran " << iterations << " iterations before convergence." << endl ;
        done = true ;
      }
    }

    if (!done) {
      if (periodInMins) {
        if (iterations % TIME_INTERVAL == 0) {  // we might not want to check if time over in every iteration
          curTime = time(NULL) ;
          if (curTime-startTime > maxPeriod*60) {
            cerr << "[INFO] Time limit exceeded, ran " << iterations << " iterations in " << maxPeriod << " minutes." << endl ;
            done = true ;
          }
        }
      }
      else { // limited number of iterations
        if (iterations > maxPeriod) {
          done = true ;
        }
      }
    } // if not done
  } // while mixing
}

probType calcFullAssignLikelihood(lbModel const& model,lbAssignment const& assign,int temp) {    
    probType entryProb=1;
    for (cliqIndex ci=0;
	 ci<model.getGraph().getNumOfCliques();
	 ci++) {
      lbAssignedMeasure const& cliqueAM = model.getAssignedMeasureForClique(ci);
      probType thisProb = cliqueAM.valueOfFull(assign);
      entryProb*=thisProb;
    } 
    return pow((double)entryProb,1.0/temp);
}

void checkSwitching(safeVec<lbAssignment_ptr> & assignsVec,lbModel const& model,int k) {
  probVector ratios = probVector(k);
  probType maxRatio = 0;
  int maxInd=-1;
  ratios[0]=1;
  probType p0x0 = calcFullAssignLikelihood(model,*(assignsVec[0]),1);
  for (int ind = 1; ind < k ; ind++) {
    probType pkx0 = calcFullAssignLikelihood(model,*(assignsVec[0]),(int)pow(2.0,ind));
    probType p0xk = calcFullAssignLikelihood(model,*(assignsVec[ind]),1);    
    probType pkxk = calcFullAssignLikelihood(model,*(assignsVec[ind]),(int)pow(2.0,ind));

    ratios[ind] = (p0xk*pkx0)/(p0x0*pkxk);
    
    if (ratios[ind]>maxRatio) {
      maxRatio = ratios[ind];
      maxInd = ind;
    }
  }
  bool swap = false;
  if (maxRatio<1) {
    double randNum = _lbRandomProbGenerator.RandomDouble(1);
    if (randNum>maxRatio)
      swap=true;
  }
  else
    swap = true;
  
  if (swap) {
    cerr<<"Ratio: "<<maxRatio<<" Switching to "<<maxInd<<endl;
    lbAssignment_ptr tassign = assignsVec[0];
    assignsVec[0]=assignsVec[maxInd];
    assignsVec[maxInd]=tassign;    
  }
  else {
    cerr<<"Did not swap"<<endl;
  }
}

void printAssign (const lbAssignment_ptr & assign,
                  ofstream & O,
                  int numOfVars,
                  lbModel & model,
                  lbGraphStruct & graph,
                  double & partitionSum)
{
  assign->print(O,numOfVars);
  
  // Partition estimation:
  double curEnergy = 0.0 ;
  for (cliqIndex cliq = 0; cliq < graph.getNumOfCliques(); cliq++){
    varsVec cliqVars = graph.getVarsVecForClique(cliq);
    lbAssignedMeasure const& meas = model.getAssignedMeasureForClique(cliq);
    curEnergy += meas.logValueOfFull (*assign) ;
  }
  //cout << "[DEBUG] Current energy: " << curEnergy << endl ;
  partitionSum += 1/exp(curEnergy) ;
}



/*
Main method 
*/
int main(int argc, char* argv[]) {
  //srand(0);
  _lbRandomProbGenerator.Initialize(time(NULL));
  lbMeasureDispatcher* disp = new lbMeasureDispatcher(MT_TABLE_NOLOG);
  cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ BEGIN TEST"<<endl;
  lbDriver_ptr driver(new lbDriver(*disp));
  if (argc!=6 && argc!=7){
    cerr<<"USAGE: gibbsSample netFileName burnInPeriod lagPeriod numberOfAssignments outFileResult anneal=k/metropolis"<<endl;
    exit(1);
  }

  bool metropolis = false ;
  bool anneal = false;
  bool convergenceTest = false ;
  int k=1;
  double burnTime ;
  bool burnTimeInMins = false ;

  string fileName =string(argv[1]);

  string burnTimeArg (argv[2]) ; // "10000"/"converge:10000"/"10000:min" for convergence test or "10000" for normal mode
  stringVec burnTimeSplit = splitString (burnTimeArg, ":") ;
  int burnSplitSize = burnTimeSplit.size() ;
  if (burnTimeSplit[0] == "converge") {
    convergenceTest = true ;
    k = 5 ; // this is the number of parallel chains
  }
  if (burnTimeSplit[burnSplitSize-1] == "min") {
    burnTimeInMins = true ;
    burnTime = atof (burnTimeSplit[burnSplitSize-2].c_str()) ;
  }
  else {
    burnTime = atof (burnTimeSplit[burnSplitSize-1].c_str()) ;
  }

  double lagTime ;
  bool lagTimeInMins = false ;
  string lagTimeArg (argv[3]) ;
  stringVec lagTimeSplit = splitString (lagTimeArg, ":") ;
  int lagSplitSize = lagTimeSplit.size() ;
  if (lagTimeSplit[lagSplitSize-1] == "min") {
    lagTimeInMins = true ;
    lagTime = atof (lagTimeSplit[lagSplitSize-2].c_str()) ;
  }
  else {
    lagTime = atof (lagTimeSplit[lagSplitSize-1].c_str()) ;
  }

  int numOfAssign = atoi(argv[4]);
  string oFileName = argv[5];

  if (argc==7) {
    string samplerTypeArg (argv[6]) ;
    stringVec samplerTypeSplit = splitString (samplerTypeArg, "=") ;
    string & samplerType = samplerTypeSplit[0] ;
    if (samplerType == "anneal") {
      if (convergenceTest) {
        cerr << "[WARNING] Cannot run convergence test with annealling - running without annealling." << endl ;
      }
      else {
        anneal = true ;
        k = atoi (samplerTypeSplit[1].c_str()) ;
      }
    }
    else if (samplerType == "metropolis") {
      metropolis = true ;
    }
    else {
      cerr << "[ERROR] Unexpected value for Gibbs sampler type (allowed: anneal=k/metropolis, got: "
           << samplerTypeArg << "), running plain old Gibbs sampler." << endl ;
    }
  }

  cout<<"Running gibbs on model from "<<fileName<<"\n"
      <<"Burn time: "<<burnTime ;
  if (burnTimeInMins) { cout << " minutes" ; }
  cout <<"\n"
       <<"Lag time: "<<lagTime ;
  if (lagTimeInMins) { cout << " minutes" ; }
  cout <<"\n"
       <<"producing "<<numOfAssign<<" assignments into file "
       <<oFileName<<"\n";
  if (anneal) {
    cout <<"With annealing, k= "<<k<<endl;
  }
  if (metropolis) {
    cout << "With metropolis acceptance" << endl ; 
  }
  if (convergenceTest) {
    cout << "With convergence test" << endl ;
  }

  //init basics
  driver->readUniverse(fileName);
  lbModel& model = driver->getModel();
  lbGraphStruct& graph = driver->getGraph();
  lbAssignment_ptr assign = new lbAssignment();
  safeVec <lbAssignment_ptr> assignsVec ;
  if (anneal || convergenceTest) {
    assignsVec = safeVec<lbAssignment_ptr>(k);    
  }
  int numOfVars = graph.getNumOfVars();
  cliquesVecVector cacheVec = cacheCliqBelongings(model,graph);
  double partitionSum = 0.0 ;
  
  //set initial random assignment
  varsVec vars = varsVec(1);
  
  if (anneal || convergenceTest) {
    for (int ind=0;ind<k;ind++){
      assignsVec[ind] = new lbAssignment();
      for (rVarIndex rVar = 0  ; rVar<numOfVars ; rVar++){
	vars[0]=rVar;
	cardVec cards = model.getCardForVars(vars);
	varValue randVal =(varValue)_lbRandomProbGenerator.RandomInt(cards[0]);
	assignsVec[ind]->setValueForVar(rVar,randVal);
      }
    }
  }
  else {
    for (rVarIndex rVar = 0  ; rVar<numOfVars ; rVar++){
      vars[0]=rVar;
      cardVec cards = model.getCardForVars(vars);
      varValue randVal =(varValue)_lbRandomProbGenerator.RandomInt(cards[0]);
      assign->setValueForVar(rVar,randVal);
    }
  }
  int divideFactor=3;

  //burn in
  //cerr << "Burning in..." << endl;
  if (anneal) {
    for (int ind=0;ind<k;ind++){
      double burnTimeInterval = burnTime/divideFactor ;
      for (int factor = 1;factor<divideFactor;factor++) {
        runAssignments(burnTimeInterval,
                       burnTimeInMins,
                       *(assignsVec[ind]),
                       model,
                       *disp,
                       numOfVars,
                       cacheVec,
                       pow(2.0,ind),
                       false);
        checkSwitching(assignsVec,model,k);
      }
    }
  }
  else if (convergenceTest) {
    runWithConvergeTest (burnTime,
                         burnTimeInMins,
                         assignsVec,
                         model,
                         *disp,
                         numOfVars,
                         cacheVec,
                         1.0,
                         metropolis) ;
  }
  else { // normal sampler
    runAssignments(burnTime,burnTimeInMins,*assign,model,*disp,numOfVars,cacheVec,1.0,metropolis);
  }
  //cerr << "Done burning in..." << endl;

  ofstream* O = new ofstream(oFileName.c_str());
  //lag
  for (int sampleNumber=0;sampleNumber<numOfAssign;sampleNumber++) {
    // run for lag time
    if (anneal) {
      for (int ind=0;ind<k;ind++){
        double lagTimeInterval = lagTime/divideFactor;
        for (int factor = 1;factor<divideFactor;factor++) {
          runAssignments(lagTimeInterval,
                         lagTimeInMins,
                         *(assignsVec[ind]),
                         model,
                         *disp,
                         numOfVars,
                         cacheVec,
                         pow(2.0,ind),
                         false);
          checkSwitching(assignsVec,model,k);
        }
      }
    }
    else if (convergenceTest) {
      for (int ind=0;ind<k;ind++){
        runAssignments(lagTime,
                       lagTimeInMins,
                       *(assignsVec[ind]),
                       model,
                       *disp,
                       numOfVars,
                       cacheVec,
                       1.0,
                       metropolis);
      }
    }
    else {
      runAssignments(lagTime,lagTimeInMins,*assign,model,*disp,numOfVars,cacheVec,1.0,metropolis);
    }

    //cerr << "Adding sample " << sampleNumber << endl;
    if (anneal) {
      printAssign (assignsVec[0], *O, numOfVars, model, graph, partitionSum) ;
    }
    else if (convergenceTest) {
      // add a sample from each chain
      for (int ind=0;ind<k;ind++) {
        printAssign (assignsVec[ind], *O, numOfVars, model, graph, partitionSum) ;
        sampleNumber++ ;
        if (sampleNumber >= numOfAssign) {
          break ;
        }
      }
      sampleNumber-- ; // we counted all samples (++), and the for loop also counts one sample (++)
    }
    else {
      printAssign (assign, *O, numOfVars, model, graph, partitionSum) ;
    }
  }

  O->close();
  delete O;  

  // Partition estimation:
  // Y. Ogata and M. Tanemura, "Estimation of interaction potentials of
  // spatial point patterns through the maximum likelihood procedure", Ann.
  // Inst. Statist. Math., vol. 33, pp. 315-338, 1981.
  const lbCardsList & varsCards = model.getCards() ;
  double totalCard = 0 ;
  for (int i=0 ; i<varsCards.getNumOfCards() ; ++i) {
    if (i==0 && totalCard==0) {
      totalCard = 1 ;
    }
    totalCard *= varsCards.getCardForVar(i) ;
  }

  if (convergenceTest) {
    numOfAssign = numOfAssign*k ;
  }

  cout << "Partition estimate: " << log (numOfAssign*totalCard / partitionSum) << endl ;

  // cleanup:

  if (anneal) {
    for (int ind=0 ;ind<k;ind++) {
      delete assignsVec[ind];
    }
  }
  
  delete driver;
  delete disp;

  return 0;
}
