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

#include <lbGSLLearningObject.h>
#include <lbBeliefPropagation.h>
#include <iomanip>
#include <time.h>
#include <sstream>
#include <lbDefinitions.h>
#include <lbNewtonOptimizer.h>
#include <lbGradientAscent.h>

using namespace std;
using namespace lbLib ;
/**
   This is the wrapper function for the conjugate gradient
   it can calculate likelihood (as f) and the d-log Likelihood as (d)
*/

/*
  Building learning object
*/
lbGSLLearningObject::lbGSLLearningObject(string evidenceFileName,
					 lbModel& model,
					 lbMeasureDispatcher const& disp,
					 double thresh,
					 lbMessageCompareType cType,
					 double smooth,
					 lbMessageQueueType qType,
					 set<measIndex> const* measSet)
  : _model(model),
    _ownInfObj(true),
    _ownSuffStat(true),
    _ownObjFunc(true)
{  
  if ( measSet != NULL )
    _measSet = *measSet;

  // build default inference object
  _infObj = new lbBeliefPropagation(model, disp);
  //  ((lbBeliefPropagation *)_infObj)->createInferenceMonitor();
  ((lbBeliefPropagation *)_infObj)->setThreshold(thresh);
  ((lbBeliefPropagation *)_infObj)->setCompareType(cType);
  ((lbBeliefPropagation *)_infObj)->setSmoothing(smooth);
  ((lbBeliefPropagation *)_infObj)->setQueueType(qType);
  _suffStat = new lbSuffStat(*_infObj,
			     evidenceFileName,
			     _measSet);

  _objFunc = new GSLFuncWrapper(*_suffStat,_model,_model.getParamNum(),_measSet,REG_NONE,0.0);

  if (isVerbose(V_LEARNING)) {
    cerr << "After suff stat, f=" << f() << endl; // no regularizatin yet
  }

}
    
lbGSLLearningObject::lbGSLLearningObject(lbModel& model,
					 lbSuffStat* suffStats,
                                         ObjectiveFunction * objFunc,
					 lbBeliefPropagation* infObj,
					 lbMeasureDispatcher const& disp,
					 set<measIndex> const* measSet)
  : _infObj(infObj),
    _suffStat(suffStats),
    _model(model),
    _objFunc(objFunc),
    _ownInfObj(false),
    _ownSuffStat(false),
    _ownObjFunc(false)
{
  if ( measSet != NULL )
    _measSet = *measSet;
}

lbGSLLearningObject::lbGSLLearningObject(lbModel& model,
					 lbSuffStat* suffStats,
					 lbBeliefPropagation* infObj,
					 lbMeasureDispatcher const& disp,
					 set<measIndex> const* measSet)
  : _infObj(infObj),
    _suffStat(suffStats),
    _model(model),
    _ownInfObj(false),
    _ownSuffStat(false),
    _ownObjFunc(true)
{
  if ( measSet != NULL )
    _measSet = *measSet;

  _objFunc = new GSLFuncWrapper(*_suffStat,_model,_model.getParamNum(),_measSet,REG_NONE,0.0);
}


lbGSLLearningObject::~lbGSLLearningObject() {
  if (_ownInfObj)
    delete _infObj;
  if (_ownSuffStat)
    delete _suffStat;
  if (_ownObjFunc)
    delete _objFunc;
}
    
probType lbGSLLearningObject::learnDirected() {
  //  const int MAX_ITER=10;
  measIndicesVec changedMeasures ;  
  for (measIndex meas=0;meas<_model.getNumOfMeasures();meas++) {
    if ( _measSet.size() == 0 || _measSet.find(meas)!=_measSet.end() )
      if (_model.getMeasure(meas).isDirected()) {
	lbMeasure_Sptr temp = (_suffStat->getEmpiricalExpectation(meas)).duplicate();
	temp->makeDirected();
	temp->normalizeDirected();
	int paramNum = temp->getParamNum();
	probType* paramVec = new probType[paramNum];
	for (int i =0 ;i<paramNum;i++)
	  paramVec[i]=0;
	temp->extractLogParamsAddToVector(paramVec,0);
	bool changed;
	_model.setLogParamsToMeasure(meas,paramVec,changed);
	if (changed) {
	  changedMeasures.push_back(meas);
	}
	delete[] paramVec;
	  }
  }
  _model.updateListeners(changedMeasures);
  return 0;
}

probType lbGSLLearningObject::learnUndirected(tGSLOptimizer::tProcType method,
					      probType LEARN_EPS,
					      probType step,
					      int MaxIter) {
  
  int pSize;
  probType* p = _model.getLogParamVector(pSize,true,_measSet);
  probType* res = new probType[pSize];
  for ( int r=0 ; r<pSize ; r++ )
    res[r] = 0.0;
  probType fret;
  time_t tid;
  tid = time(NULL);

  string mname;
  switch ( method ) {
  case tGSLOptimizer::FR: mname = "FR"; break;
  case tGSLOptimizer::PR: mname = "PR"; break;
  case tGSLOptimizer::BFGS: mname = "BFGS"; break;
  case tGSLOptimizer::STEEP: mname = "STEEP"; break;
  case tGSLOptimizer::NEWTON: mname = "NEWTON"; break;
  case tGSLOptimizer::GRADIENT: mname = "GRADIENT"; break;
    
  default: assert(false); break;
  }
  //    cerr<<"PPP START " << mname << " AT TIME "<<ctime( &tid )<<endl;
  
  // actual optimization
  switch ( method ) {
  case tGSLOptimizer::NEWTON:
    {
      // NOTE: shouldn't we call here
      // wrapper = new lbNRFuncWrapper...
      // ???
      lbNewtonDiagonalOptimizer opt(*(getObjFunc()));
      fret = opt.Optimize(p,res,LEARN_EPS,step,MaxIter);
    }
    break;
  case tGSLOptimizer::GRADIENT: 
    {
      lbGradientAscent opt(*(getObjFunc()));
      fret = opt.Optimize(p,res,LEARN_EPS,step,MaxIter);
    }
    break;
  default:
    tGSLOptimizer opt(*(getObjFunc()),method);
    fret = opt.Optimize(p,res,LEARN_EPS,step,MaxIter);
    break;
  }
  
  tid = time(NULL);
  //    cerr<<"PPP ENDED " << mname << " AT TIME "<<ctime( &tid )<<endl;
  cerr<<"PPP---------------------"<<endl;

  _model.updateLogParamsFromVector(res,true,_measSet);
  
  if (isVerbose(V_LEARNING)) {
    cerr << "*** Final Log Parameters in _model: ";
    probType* q = _model.getLogParamVector(pSize,true,_measSet);
    for ( int pi=0 ; pi<pSize ; pi++ )
      cerr << " " << q[pi];
    cerr << endl;
    delete[] q;
  }

  if (p)
    delete[] p;
  if (res)
    delete[] res;
  return 0;
}

probType lbGSLLearningObject::learnEM(tGSLOptimizer::tProcType method,probType LEARN_EPS,probType step,int maxIter,int maxEMIter) {
  //  repeat untill convergrnce \ max loops
  const probType CONVERGENCE_THRESHOLD = (0.001);
  _suffStat->setEMMode(true);
  probType oldLike = -HUGE_VAL;
  oldLike = -f() ; // "-" since we now maximize
  probType newLike;
  bool finished = false;
  int iter = 0;
  do {

    // learn parmeters according to direction
    learnDirected();
    learnUndirected(method,LEARN_EPS,step,maxIter);

    // reset counts after complete data learning step
    _suffStat->resetCounts();

    newLike = -f() ; // "-" since we now maximize
    probType logDiffRatio  = fabs((oldLike-newLike)/oldLike);
    iter++;
    cerr<<"Old LL: "<<oldLike<<"\tNew LL: " << newLike << endl;
    if ( (logDiffRatio<CONVERGENCE_THRESHOLD) ) {
        cerr << "Learning converged with logDiffRatio = " << logDiffRatio << endl;
        finished = true;
    }
    if ( iter>=maxEMIter )
      finished=true;
    oldLike=newLike;

    cerr<<"EM : Finished iter num : "<<iter<<" new like is : "<<newLike<<endl;
  } while (!finished );

  cerr<<"Finished EM Learning EM : total num of iter : "<<iter<<" like is : "<<newLike<<endl;
  return newLike;
}

void lbGSLLearningObject::testLikeAndDeriv(probType LOC_EPS) {
  cout << "EMMode: " << _suffStat->getEMMode() << endl; // TODO: to remove
  //print graph
  cerr<<"*** Start of test like and deriv"<<endl;
  int pSize;
  probType* p = _model.getLogParamVector(pSize,false,_measSet);
  probType* der = new probType[pSize]; 
  for ( int d=0 ; d<pSize ; d++ )
    der[d] = 0.0;
  probType newLike;
  probType oldLike;
  getObjFunc()->fdf(p,der);
  
  for (int i=0;i<pSize;i++) {
    oldLike=getObjFunc()->f(p);
    p[i]+=LOC_EPS;
    newLike = getObjFunc()->f(p);
    p[i]-=LOC_EPS;
    probType delta = newLike-oldLike;
    probType empirDer = delta/LOC_EPS;
    probType aerror = ABS(empirDer-der[i]);
    probType rerror = aerror/ABS(der[i]);
    if ( rerror > 0.01 ) {
      cerr<<"New like: " << newLike << "\toldLike: " << oldLike << "\tDelta: " << LOC_EPS << endl;
      cerr<<"i: "<<i<<" error is "<< aerror<<", Empir: "<<empirDer<<" Calculated: "<<der[i]<<endl;
    } else { // TODO: to remove
      cerr<<"New like: " << newLike << "\toldLike: " << oldLike << "\tDelta: " << LOC_EPS << endl;
      cerr << "i: " << i << " correct "<< aerror << ", Empir: " << empirDer << " Calculated: " << der[i] << endl;
    }
  }
  
  delete[] p;
  delete[] der;
  cerr<<"*** End of test like and deriv"<<endl;
  //  return 0;
}

void lbGSLLearningObject::testDerivAndHess(probType LOC_EPS) {
  //print graph
  cerr<<"*** Start of test deriv and hess (diagonal only)"<<endl;
  int pSize;
  probType* p = _model.getLogParamVector(pSize,false,_measSet);
  probType* der = new probType[pSize]; 
  probType** hes = new probType*[pSize];
  for ( int d=0 ; d<pSize ; d++ ) {
    hes[d] = new probType[pSize];
    der[d] = 0.0;
  }
  probType newDeriv;
  probType oldDeriv;
  getObjFunc()->ddf(p,hes);
  
  for (int i=0;i<pSize;i++) {
    getObjFunc()->df(p,der);
    oldDeriv = der[i];
    p[i]+=LOC_EPS;
    getObjFunc()->df(p,der);
    p[i]-=LOC_EPS;
    newDeriv = der[i];
    probType delta = newDeriv-oldDeriv;
    probType empirHes = delta/LOC_EPS;
    probType aerror = ABS(empirHes-hes[i][i]);
    probType rerror = aerror/ABS(hes[i][i]);
    if ( rerror > 0.01 ) {
      cerr<<"New deriv: " << newDeriv << "\tOld deriv: " << oldDeriv << "\tDelta: " << LOC_EPS << endl;
      cerr<<"i: "<<i<<" error is "<< aerror<<", Empir: "<<empirHes<<" Calculated: "<<hes[i][i]<<endl;
    }
  }
  
  delete[] p;
  delete[] der;
  for ( int d=0 ; d<pSize ; d++ )
    delete[] hes[d];
  delete[] hes;
  cerr<<"*** End of test deriv and hess"<<endl;
  //  return 0;
}

double lbGSLLearningObject::f()
{
  // calling with no parameters - we take them from the model
  int pSize;
  probType* p = _model.getLogParamVector(pSize,true,_measSet);
  double like = getObjFunc()->f(p) ;
  delete [] p ;
  return like ;
}

//calculates likelihood of params of the model
double GSLFuncWrapper::f(long double const* p)
{
  _model.updateLogParamsFromVector(p, true,_measSet);
  double retVal = fNoRegularization() ;

  int pSize = paramNum() ;

  double penalty = 0.0 ;
  if (regularize()) {
    penalty = regPenalty(p,pSize) ;
  }

  retVal = -(retVal-penalty) ; // "-" since we minimize!!!

  if (isVerbose(V_LEARNING)) {
    cout << "------ in f(). value:\t" << retVal << endl;
    if (penalty!=0)
      cout << "--- penalty:\t" << penalty << endl ;
    cout << "p:";
    for (int i = 0; i < pSize; i++)
      cout << ' ' << p[i];
    cout << endl;
  }

  return retVal;
}

void GSLFuncWrapper::endIteration(int iter, probType * sum, probType * cnt, string dumpprefix) {
  int pSizeTmp;
  probType* pTmp = _model.getLogParamVector(pSizeTmp,true,_measSet);
  cout << "Log Likelihood in end of iter " << iter << " is : " << f(pTmp) << endl;
  delete [] pTmp ;

  char c[5];
  sprintf(c, "%d", iter);
  string fileName = dumpprefix + string("dumpEndCGIter") + string(c);
  ofstream* optr = new ofstream(fileName.c_str());

  int j, pSize;
  probType * oldp = NULL, * p = NULL;
  if (cnt != NULL) {
    oldp = _model.getLogParamVector(pSize, true);
    p = new probType[pSize];
    for (j = 0; j < pSize; j++) {
      if (cnt[j] != 0) {
	p[j] = sum[j] / cnt[j];
      } else { // e.g. when a measure is not used by any clique
	p[j] = oldp[j];
      }
    }
    _model.updateLogParamsFromVector(p, true);
  }

  (*optr) << "@Measures" << endl;
  for (measIndex mInd=0;mInd<_model.getNumOfMeasures();mInd++) {
    lbMeasure const& meas = _model.getMeasure(mInd);
    string name = _model.getMeasureName(mInd);
    (*optr)<<name<<"\t";
    meas.printToFastInfFormat(*optr, false, 5, true);
  }
  (*optr) << "@End" << endl;
  optr->close();
  delete optr;

  if (cnt != NULL) {
    _model.updateLogParamsFromVector(oldp, true);
    delete [] p ;
    if (oldp) {
      delete [] oldp;
    }
  }
}
  
void GSLFuncWrapper::df(long double const* p, long double* res) {
  _model.updateLogParamsFromVector(p,true,_measSet);

  int index =0;
  /* the deriv is actually the difference between expected and observed (for log-likelihood).
     notice that this is the deriv , but the parameters actually go
     to the model so we can raise one clique param and cause the change
     in many beliefs.
  */
  for (measIndex meas=0;meas<_model.getNumOfMeasures();meas++)
    if ( _measSet.size() == 0 || _measSet.find(meas)!=_measSet.end() )
      if (!(_model.getMeasure(meas).isDirected()))
	index = calcDerivForMeasure(meas,res,index);

  if (regularize()) {
    long double* tmp = new long double[index];
    dRegPenalty(p,index,tmp);
    for ( int i=0 ; i<index ; i++ )
      res[i] += tmp[i]; // we minimize (therefore we add)!
  }

  if (isVerbose(V_LEARNING)) {
    cout << "------ in df()." << endl;
    cout << "p:";
    for (int i = 0; i < index; i++) {
      cout << ' ' << p[i];
    }
    cout << endl;
    cout << "res:";
    for (int i = 0; i < index; i++) {
      cout << ' ' << res[i];
    }
    cout << endl;
  }
}

int GSLFuncWrapper::calcDerivForMeasure(measIndex meas,
                                        long double* res,
                                        int index) const
{
  int size = _model.getMeasure(meas).calcDeriv(res,index, _suff.getEstimatedExpectation(meas), _suff.getEmpiricalExpectation(meas));

  /*  cerr << "Derivative for measure " << meas << " is:";
  for ( int i=0 ; i<size ; i++ )
    cerr << " " << res[index+i];
  cerr << endl;
  */

  index += size;
  return index;
}

long double GSLFuncWrapper::fdf(long double const* p,long double* res) {
  if (isVerbose(V_LEARNING)) {
    cout << "------ in fdf()..." << endl;
  }
  df(p, res);
  return f(p);
}

void GSLFuncWrapper::ddf(long double const* p, long double** res) 
{
  // fow now doing a diagnoal approximation so that that element ii of the Hessian
  // is - E[f^2] + E[f]^2

  _model.updateLogParamsFromVector(p,true,_measSet);

  int index =0;
  for (measIndex meas=0;meas<_model.getNumOfMeasures();meas++)
    if ( _measSet.size() == 0 || _measSet.find(meas)!=_measSet.end() )
      if (!(_model.getMeasure(meas).isDirected())) { 
	int length = _model.getMeasure(meas).getParamNum();
	probType* tmpRes = new probType[length];
	
	// compute the Hessian part of measure 'meas' at res[index][index]
	//
	// we can use the calcDeriv of the measure since the Hessian has the
	// same form as the gradient with the empirical expectation replaced
	// by the empirical expectation of the features squared and the estimated
	// expectation is simply squared
	lbMeasure_Sptr ES = _suff.getEstimatedExpectationSquared(meas);
	lbMeasure_Sptr SE = _suff.getEstimatedSquaredExpectation(meas);
	int size = _model.getMeasure(meas).calcDeriv(tmpRes,0,
						     *SE,
						     *ES);
	assert(size == length);
	
	// move to matrix
	for ( int i=0 ; i<length ; i++ )
	  res[index+i][index+i] = tmpRes[i];
	index += length;
	delete[] tmpRes;
      }
}
