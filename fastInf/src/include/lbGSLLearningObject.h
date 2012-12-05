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

#ifndef _GSL__Learning__Object_
#define _GSL__Learning__Object_

#include <GSLOptimizer.h>
#include <lbInferenceObject.h>
#include <lbSuffStat.h>
#include <ObjectiveFunction.h>

namespace lbLib {

  class lbModel;
  class lbBeliefPropagation;

  enum RegularizationType {REG_NONE,REG_L1,REG_L2} ;
  
  class GSLFuncWrapper : public ObjectiveFunction {
  public:

    inline GSLFuncWrapper(lbSuffStat& suff,
                          lbModel& model,
                          int size,
			  set<measIndex> const& measSet,
                          RegularizationType regType = REG_NONE,
                          double regParam = 0.0);
    inline virtual ~GSLFuncWrapper();
    inline virtual void setMeasureSet(set<measIndex> const& measSet);
    virtual double f(long double const* p) ;
    virtual void df (long double const* p, long double* res);
    virtual long double fdf(long double const* p, long double* res) ;
    virtual void endIteration(int iter, probType * sum = NULL, probType * cnt = NULL, string dumpprefix = string(""));
    virtual void ddf(long double const* p, long double** res) ;
    inline virtual int paramNum() const;

  protected:
    virtual int calcDerivForMeasure(measIndex meas,
                                    long double* res,
                                    int index) const ;
    virtual double fNoRegularization() { return _suff.getLogLikelihood(); }

  protected:
    lbModel& _model;
    lbSuffStat& _suff;

  private:
    int _vecSize;
    set<measIndex> _measSet;
    lbModel* _tmpModel;
  };  
  
  class lbGSLLearningObject {

  public:

    lbGSLLearningObject(string evidenceFileName,
			lbModel& model,
			lbMeasureDispatcher const& disp,
			double thresh = 1e-5,
			lbMessageCompareType cType = C_MAX,
			double smooth = 0.5,
			lbMessageQueueType qType = MQT_WEIGHTED,
			set<measIndex> const* measSet = NULL);

    lbGSLLearningObject(lbModel& model,
			lbSuffStat* suffStats,
                        ObjectiveFunction * objFunc,
			lbBeliefPropagation* infObj,
			lbMeasureDispatcher const& disp,
			set<measIndex> const* measSet = NULL);

    lbGSLLearningObject(lbModel& model,
			lbSuffStat* suffStats,
			lbBeliefPropagation* infObj,
			lbMeasureDispatcher const& disp,
			set<measIndex> const* measSet = NULL);

    virtual ~lbGSLLearningObject() ;

    // EMMode: true: normal EM. false: incremental EM
    virtual probType learnEM(tGSLOptimizer::tProcType method = tGSLOptimizer::FR,probType LEARN_EPS = 1e-3,probType step = 0.01, int maxIter = 100, int maxEMIter = 15);

    void testLikeAndDeriv(probType LOC_EPS = 0.00001);
    void testLikeAndDerivByMeasure(probType LOC_EPS = 0.00001);
    void testDerivAndHess(probType LOC_EPS = 0.00001);

    void setRegularizeParamL1(double beta) { _objFunc->setRegularizeParamL1(beta); }
    void setRegularizeParamL2(double sigmaSq) { _objFunc->setRegularizeParamL2(sigmaSq); }

  protected: //functions
    probType learnDirected() ;
    probType learnUndirected(tGSLOptimizer::tProcType method,probType LEARN_EPS,probType step,int maxIter);
    
    ObjectiveFunction * getObjFunc() {
      return _objFunc ;
    }

    virtual double f() ;

  private:
    probType learnConjugate() ;
    probType learnGradientAscent() ;
    probType learnParamsIPF() ;

  protected: //data members
    lbInferenceObject* _infObj;

    lbSuffStat_ptr _suffStat;

    lbModel& _model;

    set<measIndex> _measSet;

    ObjectiveFunction * _objFunc ;

    bool _ownInfObj;    // does learning object own the inference object?
    bool _ownSuffStat;  // does learning object own the suff stats?
    bool _ownObjFunc;

  };

  inline void GSLFuncWrapper::setMeasureSet(set<measIndex> const& measSet) {
    _measSet=measSet;
  };

  inline int GSLFuncWrapper::paramNum() const{
    return _vecSize;
  }
  
  //ctor
  inline GSLFuncWrapper::GSLFuncWrapper(lbSuffStat& suff,
                                        lbModel& model,
                                        int size,
                                        set<measIndex> const& measSet,
                                        RegularizationType regType,
                                        double regParam)
    :
    _model(model),
    _suff(suff),
    _vecSize(size),
    _measSet(measSet)
  {
    // regularization
    if (regType==REG_L1)
      setRegularizeParamL1(regParam);
    else if (regType==REG_L2)
      setRegularizeParamL2(regParam);
  }

  inline GSLFuncWrapper::~GSLFuncWrapper() {
  }

} 


#endif //_Loopy__Learning__Object_
