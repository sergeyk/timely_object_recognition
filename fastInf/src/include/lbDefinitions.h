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

/*! \mainpage FastInf approximate inference help page
 * 
 * \section sec_create Library creators:
 * Ariel Jaimovich, Ofer Meshi, Ian Mcgraw  and Gal Elidan
 * 
 * This library was written in the School of Computer Science and Engineering
 * in the Hebrew University 
 * \n and in the Computer Science Dept. in Stanford Univeristy
 * \n in the labs of Prof Nir Friedman and Prof. Daphne Koller.
 * 
 * Library home page:
 * see http://compbio.cs.huji.ac.il/FastInf
 * 
 * \line 
 *
 * \section sec_intro Introduction
 * The fastInf c++ library was designed to perform memory and time efficient approximate inference in large relational undirected graphical models,
 * and to allow parameter and structure learning of template markov random fields.
 * 
 * The main idea behind the design of the library is that the inside
 * representation of variables and potentials will be very thin, so as
 * to allow efficient representation simple and efficient. In addition
 * we wished the template nature of the models to avoid duplicating
 * unnecessary copies of template potentials. 
 *
 *
 * We create a rich interface for all our basic classes (See below) in
 * order to allow implementation of various inference and learning
 * methods. Here, we implemented basic Loopy Belief Propagation,
 * Generalized belief propagation, along with various heuristics on
 * how to help these methods perform better using various smoothing
 * on messages, various queue types for the messages (see Elidan,
 * Mcgraw & Koller UAI06) and various counting numbers (see Meshi,
 * Jaimovich, Globerson & Friedman UAI09).
 *
 * 
 * \section sec_lib Directories Included in This Package
 * In general the library files reside in six main libraries:
 * include: contains all the header files
 * lib: contains the library basic source files for representing the models and the basic inference methods(.cpp)
 * learn: contains the source files for learning parameters of models
 * exe: contains files that demonstrate basic running of the library
 * test: contains tests that check if the library operates correctly (can be used after code changes to make sure nothing is broken)
 * nets: contain examples for model files
  * 
 * \line 
 * 
 * 
 * \section sec_design Code Design
 * The basic idea of the library is to hold an efficient representation of potentials to allow
 * efficient representation and fast simple actions (e.g., addition / multiplication of potentials).
 * The basic classes of the library are:
 *
 * lbMeasure.h : This model holds a template potential (i.e., it does not know on which variables it is defined)
 * in a form of a vector of vectors. The potentials hold non-negative numbers and can be represented in log/regular
 * basis. The library is implemented for discrete potentials (usually tables) but has the infrastructure to allow also continuous 
 * potentials.
 * 
 * lbGraphStruct.h : This class holds the basic structure of the graphical models. It is represented via a list of cliques.
 * We recommend, as default to build a factor graph (where there is a clique for each potential and each variable).
 * As input, the graph can be given also a list of relations between the cliques (e.g., which cliques are neighbors).
 *
 * lbModel.h : This class holds the entire graphical model, by joining the graphical structures with the corresponding template
 * measures, It creates for each clique an lbAssignedMeasure that holds an instantiation of a template measure for a specific
 * set of variables.
 * lbAssignment.h: This class holds assignment for variables in the model (either full or partial assignment).
 * 
 * lbInferenceObject.h : Is the basic (pure virtual) class that defines the interface required from inference applications (e.g., marginal probabilities/
 * partition function calculations, etc.)
  * 
 * \line 
 * 
 * \section sec_usage Usage
 *
 * Examples on how to use the various options can be found in the exe library:
 * 
 * infer.cpp :  For reading a model, and performing inference using BP or GBP.
 * 
 * mfinfer.cpp : For reading a model and performing inference using Mean Field.
 *
 * gibbsSample.cpp : For reading a model and performing inference using Gibbs Sampling.
 * 
 * makeGrid.cpp : Building a grid with various parametrization options (for basic comparison of inference methods)
 *
 * learning.cpp : Demonstrates basic parameter estimation procedures of a relational Markov Random Field.
 * 
 * \section sec_usage_examples Usage Examples
 * Each of the above executables can be run without parameters to get the usage parameters.
 * In addition, here we provide a couple of examples on how to run these programs (assuming current dir is the FastInf root)
 *
 * Run simple lbp and print final beliefs (on cliques): \n
 * <b> build/bin/infer -i src/nets/simpleNetWithLoop -b 0 </b>
 * 
 * Run lbp with weighted queue scheduling and print final beliefs: \n
 * <b> build/bin/infer -i src/nets/simpleNetWithLoop -q 1 -b 0</b>
 *
 * Run lbp, compute also exact beliefs using clique tree and compare the beliefs: \n
 * <b> build/bin/infer -i src/nets/simpleNetWithLoop -exact + -b 0 </b>
 *
 * Run lbp, assign the evidence in the grid3x3.assign file and compute log likelihood of each assignment: \n
 * <b> build/bin/infer -i src/nets/grid3x3.net -e src/nets/grid3x3.assign -m 0  </b>
 *
 * Run generalized lbp, and compute marginals (for each variable): \n
 * <b> build/bin/infer -i src/nets/grid3x3.net -c src/nets/grid3x3.clusters  -b 0  </b>
 *
 * Run Mean Field inference and print beliefs: \n
 * <b> build/bin/mfinfer -i src/nets/simpleloop.net -b 0</b>
 *
 * Run standard Gibbs Sampling to sample 500 samples from grid9x9: \n
 * <b> build/bin/gibbsSample src/nets/grid9x9.net 10000 1000 500 gibbsData_9x9grid.assign </b>
 *
 * Run Gibbs Sampling with burning time decided by convergence test (but no larger than 10000) to sample 500 samples from grid3x3:\n
 * <b> build/bin/gibbsSample src/nets/grid3x3.net converge:10000 1000 500 gibbsData_3x3grid.assign </b>
 *
 * Run Gibbs Sampling on 6 chains with varying temperatures (with annealing) to sample 500 samples from grid9x9:\n
 * <b> build/bin/gibbsSample src/nets/grid9x9.net 10000 1000 500 gibbsData_9x9grid.assign -anneal=6 </b>
 *
 * Learn the parameters of the alarm network from the 100 data samples in alarm.100.fastInf.data:\n
 * <b> build/bin/learning -i src/nets/alarm/alarm.fastInf.net -e src/nets/alarm/alarm.100.fastInf.data -o alarmResultNet.net  </b>
 *
 * Learn the parameters of the alarm network as above with l1 regularisation (param 0.5):\n
 * <b> build/bin/learning -i src/nets/alarm/alarm.fastInf.net -e src/nets/alarm/alarm.100.fastInf.data -r1 0.5 -o alarmResultNet.net </b>
 *
 * Learn the parameters of the alarm network as above with l2 regularisation (param 0.7):\n
 * <b> build/bin/learning -i src/nets/alarm/alarm.fastInf.net -e src/nets/alarm/alarm.100.fastInf.data -r2 0.7 -o alarmResultNet.net </b>
 *
 *
 * \line 
 * 
 * \section sec_examples FastInf Format
 * There are two alternatives for creating models with fastInf. The first ones is to use an input file
 * with a model in it (to see a documented example, look for fastInf/src/nets/simpleNet). The second option
 * is to create a model inside your program. For examples on how that is done check the modelTest.cpp file
 * in fastInf/src/tests directory.
 *
 * \section sec_compile Configuring and compiling
 * 1) Go to fastInf/src and type: 
 *  > make externalLibs
 *  This command will download the GSL and GLPK libraries from their corresponding ftp sites\n
 *  and install them on your computer. If you want to download the libraries yourself or already \n
 *  have them on your computer, follow the instructions in fastInf/srd/README.\n
 *
 *2) Go to the fastInf/src and type:\n
 *  > make  \n
 *
 *3) After compilation you will find all the executables under the fastInf/build directory.\n
 *  To make sure the compilation succeeded, you can run fastInf/build/bin/infer \n
 *  with no parameters (it will result in the usage message). \n
 *  In case of linkage problems with GSL see fastInf/srd/README.    \n
 *\n
 * FastInf was compiled and run successfully on linux 32 and 64 bit
 * distributions as well as on os-x.
 * \line 
 * 
 * \section sec_tests Testing the library
 * This library comes with a set of tests that check the integrity of the basic mechanism
 * in it. If you change some code and want to make sure you have not introduced any errors,
 * you can go inside the directory fastInf/src/tests and run the test using the command: \n
 * awk -f 'runTests.awk' test.list 1  \n
 * This command will run all the tests and compare the outputs with the existing ones.
 * In case you want to run over the reference outputs run the command: \n
 * awk -f 'runTests.awk' test.list 2  \n
 * To change the list of tests or add new test, check the tests.list file in the same directory.

 * \section sec_acknowl Acknowledgments
 *  We thank Menachem Fromer, Haidong Wang, John Duchi and Varun Ganapathi
 *  for evaluating the library and contributing implementations of
 *  various functions. This library was initiated in Nir Friedman's
 *  lab in the Hebrew University and developed in cooperation with
 *  Daphne Color's lab in Stanford. AJ was supported by the Eshkol
 *  fellowship from the Israeli Ministry of Science. IM and GE were supported
 *  by the DARPA transfer learning program under contract FA8750-05-2-0249}

*/
#ifndef _Loopy__Definitions
#define _Loopy__Definitions


/**
   This file holds the declaration and implementation
   of various typeDefs and Macros

   Part of the fastInf library
   @version July 2009
   @author Ariel Jaimovich
*/

// check
#ifdef _MSC_VER

// ignore debug trancuation warning
#pragma warning (disable : 4786)

#ifndef INFINITY
#include <limits>
#define INFINITY numeric_limits<double>::infinity()
#endif

#endif


#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <lbExeption.h>
#include <sstream>
#include <assert.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <map>
#include <sys/types.h> // for uint

//#define DEBUG
using namespace boost;

#define NOT_IMPLEMENTED_YET { cerr << "NOT_IMPLEMENTED_YET\n"; assert(false); }
#define NOT_REACHED { cerr << "NOT_REACHED\n"; assert(false); }
#define NEEDS_CLEANUP { cerr << "NEEDS_CLEANUP\n"; assert(false); }

#ifdef NO_ASSERT
#ifdef assert
#undef assert
#endif
#define assert(expr)
#endif

// handle differences between linux and windows
#ifdef _MSC_VER 
/***********
 * WINDOWS *
 ***********/
#include <time.h>
#include <mathplus.h>

#define isinf !_finite
#define isnan _isnan

#else
/*********
 * LINUX *
 *********/
#include <sys/time.h>
    
#endif

namespace lbLib {

  //class declarations
  class lbMessage;  
  class lbInferenceObject;
  class lbRandomVar;
  class lbFeatureTableMeasure;
  class lbWeightedTableMeasure;
  class lbModel;
  class lbVarsList;
  class lbCardsList;
  class lbSpanningTree;
  class lbMultinomialMeasure;
  class lbIndicator;
  class lbModelListener;
  class lbSuffStat;
  class lbLearningObject;
  class lbAssignment;
  class lbValue;

  class lbGraphListener;

  typedef lbValue lbParameter;
  typedef lbValue lbFeatureValue;

  /**
     This is a genearl definitions file
     
     Part of the loopy belief library
     \version July 2009
     \author Ariel Jaimovich
  */
  typedef lbLearningObject* lbLearningObject_ptr;
  typedef lbSuffStat* lbSuffStat_ptr;
  typedef lbModelListener* lbModelListener_ptr;
  typedef lbSpanningTree* lbSpanningTree_ptr;
  typedef shared_ptr<lbMultinomialMeasure> lbMultinomialMeasure_Sptr;
  typedef lbMessage* lbMessage_ptr;
  typedef lbInferenceObject* lbInferenceObject_ptr;
  typedef lbRandomVar* lbRandomVar_ptr;
  typedef shared_ptr < lbFeatureTableMeasure > lbFeatureTableMeasure_Sptr;
  typedef shared_ptr < lbWeightedTableMeasure > lbWeightedTableMeasure_Sptr;
  typedef lbModel* lbModel_ptr;
  typedef lbVarsList* lbVarsList_ptr;
  typedef lbCardsList* lbCardsList_ptr;
  typedef bool* bool_ptr;
  typedef shared_array<char> char_ptr;
  typedef shared_ptr <ifstream > ifstream_ptr; 
  typedef lbIndicator* lbIndicator_ptr;
  // uint is defined in sys/types.h
  //#ifndef uint
  // typedef unsigned int uint;
  //#endif
  // WARNING: DO NOT CHANGE THIS - GSL AND OTHER STUFF MIGHT NOT WORK
  typedef long double probType;


  /*
  // To check illegal actions over vectors (illegal indices, etc) use
  // this wrapper of vector instead of defining safeVec as stl vector
  // Courtesy of Yoseph Barash
  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@ safe vector : $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
  template<class T > class safeVec : public vector<T> {
  public:
  //typename vector<T>::const_iterator const_iterator;
  //typename vector<T>::iterator iterator;
  
  safeVec(): vector<T>(){};
  safeVec(int size) :vector<T>(size){};
  safeVec(int size, T t) :vector<T>(size,t){};
  safeVec(vector<T> const & vt) : vector<T>(vt) {};
  T& operator[] (uint i) {
  assert( i >= 0 && i < size() );
  return vector<T>::operator[](i);
  };
    T const& operator[] (uint i) const
    {
    assert( i >= 0 && i < size() );
    //const T & val =  vector<T>::operator[](i);
    //return val;
    return vector<T>::operator[](i);
    };

    void print(ostream& out) const{
    out<<" ";
    for (uint i = 0; i<size();i++){
    out<<(*this)[i]<<" "; 
    }
    out<<endl;  
    }; 
    };  
    
  */
#define safeVec vector
  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


  
  enum var_type {CONTINOUS, DISCRETE};
  
  enum graph_type {DIR,UNDIR};

  enum lbMessageQueueType {MQT_UNWEIGHTED, MQT_WEIGHTED, MQT_MANUAL};

  enum lbMessageWeightType { MWT_L1 , MWT_L2, MWT_LINF };

  enum lbMessageCompareType {C_AVG,C_MAX,C_KL,C_AVG_LOG,C_MAX_LOG};

  enum lbSearchMethod{SM_CONJUGATE,SM_GRADIENT_ASCENT,SM_IPF};

  enum lbMessageInitType {MIT_RANDOM,MIT_UNIFORM};

  enum lbMeasureType {MT_TABLE,MT_TABLE_NOLOG};

  enum lbSharedParamMode {SPM_SUM,SPM_AVERAGE};

  enum lbMeasureParamType {MP_SHARED,MP_IDLE};

  typedef lbGraphListener* lbGraphListener_ptr;

  typedef safeVec <lbGraphListener_ptr> graphListenersVec;
  
  typedef safeVec <lbModelListener_ptr> listenersVec;
  
  typedef listenersVec::iterator listenersVecIter;
  
  typedef probType scoreType;

  typedef safeVec<bool> boolVec;

  typedef safeVec<scoreType> scoreVec;
  
  typedef safeVec<lbIndicator_ptr> indicatorVec;

  typedef safeVec<probType> probVector;
  
  typedef safeVec<probVector> measureProbVec;

  typedef measureProbVec* measureProbVec_ptr;
  
  typedef measureProbVec::iterator probVecIter;

  typedef measureProbVec::const_iterator constProbVecIter;

  typedef safeVec<lbParameter> valueVector;

  typedef safeVec<valueVector> measureValueVec;

  typedef measureValueVec* measureValueVec_ptr;
  
  typedef measureValueVec::iterator valueVecIter;

  typedef measureValueVec::const_iterator constValueVecIter;

  typedef safeVec< int > intVec;

  typedef safeVec< intVec > intVecVec;

#define LB_MAX_DOUBLE LDBL_MAX

#define LB_MIN_DOUBLE LDBL_MIN
  
#define ABS(x) ((x) >= 0 ? (x) : -(x))

#define FindInVector(vec,element,index) \
{ \
  index = -1;\
  for (uint i=0;i<vec.size();i++)\
    if (vec[i]==element) \
      index = i;\
}
		     
#define printVector(vec,out) \
{ for (uint i=0;i<vec.size();i++)\
  out<<vec[i]<<" ";\
out<<endl; }

#define copyVectorWithDuplicate(origVec,copyVec,vecType)\
{ \
uint newSize = origVec.size(); \
copyVec.clear(); \
for (uint ind1=0;ind1<newSize;ind1++)  \
  if (origVec[ind1]) \
    copyVec.push_back(origVec[ind1]->duplicate()); \
}

#define initIntegerStruct(strName) \
struct strName{\
  inline strName() ;\
  inline strName(int value);\
  inline strName& operator=(int value) ;\
  inline operator int() const ;\
  inline strName& operator++() ;\
  inline strName& operator++(int i) ;\
  inline strName operator+ (strName const& other) ;\
  inline strName& operator += (strName const& other) ;\
  int _val;\
}; 

#define implementIntegerStruct(strName) \
inline strName::strName() {\
  _val=-1;\
}\
inline strName::strName(int value){\
  _val=value;\
}\
inline strName& strName::operator=(int value) {\
  _val=value;\
  return *this;\
}\
inline strName::operator int() const {\
  return _val;\
}\
inline strName& strName::operator++() {\
  _val++;\
  return *this;\
}\
inline strName& strName::operator++(int i) {\
  _val++;\
  return *this;\
}\
inline strName strName::operator+ (strName const& other){\
  return strName(_val+other._val);\
}\
inline strName& strName::operator += (strName const& other) {\
  _val+=other._val;\
  return *this;\
} ;

#define implementIntegerStructVectorOps(strNameVec)\
inline strNameVec vecUnique(strNameVec const& v) {\
  strNameVec copy;\
  uint i;\
  for (i = 0; i < v.size(); i++) {\
    bool found = false;\
    for (uint j = 0; j < copy.size(); j++) {\
      if (v[i] == copy[j]) {\
	  found = true;\
      }\
    }\
    if (!found) {\
      copy.push_back(v[i]);\
    }\
  }\
  return copy;\
}\
inline strNameVec vecMinus(strNameVec const& v1, strNameVec const& v2) {\
  strNameVec vec;\
  for (uint i = 0; i < v1.size(); i++) {\
    bool found = false;\
    for (uint j = 0; j < v2.size(); j++) {\
      if (v1[i] == v2[j]) {\
	found = true;\
      }\
    }\
    if (!found) {\
      vec.push_back(v1[i]);\
    }\
  }\
  return vecUnique(vec);\
}\
inline strNameVec vecIntersection(strNameVec const& v1, strNameVec const& v2) {\
  strNameVec vec, vec1, vec2;\
  vec1 = v1; \
  vec2 = v2; \
  sort(vec1.begin(), vec1.end()); \
  sort(vec2.begin(), vec2.end()); \
  while (!vec1.empty() && !vec2.empty()) { \
    if (vec1.back() > vec2.back()) { \
      vec1.pop_back(); \
    } \
    else if (vec2.back() > vec1.back()) { \
      vec2.pop_back(); \
    } \
    else { \
      assert (vec1.back() == vec2.back()); \
      if (vec.empty() || vec1.back() != vec.back()) { \
	vec.push_back(vec1.back()); \
      } \
      vec1.pop_back(); \
      vec2.pop_back(); \
    } \
  } \
 return vec;\
}\
inline strNameVec vecUnion(strNameVec const& v1, strNameVec const& v2) {\
  strNameVec vec;\
  uint i;\
  for (i = 0; i < v1.size(); i++) {\
    vec.push_back(v1[i]);\
  }\
  for (i = 0; i < v2.size(); i++) {\
      vec.push_back(v2[i]);\
  }\
  return vecUnique(vec);\
}\
inline bool vecSubset(strNameVec const& v1, strNameVec const& v2) {\
  for (uint i = 0; i < v1.size(); i++) {\
    bool found = false;\
    for (uint j = 0; j < v2.size(); j++) {\
      if (v1[i] == v2[j]) {\
	found = true;\
      }\
    }\
    if (!found) {\
      return false;\
    }\
  }\
  return true;\
}\
inline bool vecEqual(strNameVec const& v1, strNameVec const& v2) {\
  return (vecSubset(v1, v2) && vecSubset(v2, v1));\
}

  initIntegerStruct(probIndex);

  initIntegerStruct(vecIndex);

  initIntegerStruct(cliqIndex);

  initIntegerStruct(measIndex);

  initIntegerStruct(varValuesNum);

  initIntegerStruct(varValue);

  initIntegerStruct(rVarIndex);

  initIntegerStruct(assignedMesIndex);

  //initIntegerStruct(varIndex);

  //implementIntegerStruct(varIndex);

  implementIntegerStruct(probIndex);
  
  implementIntegerStruct(vecIndex);
  
  implementIntegerStruct(cliqIndex);
  
  implementIntegerStruct(measIndex);
  
  implementIntegerStruct(varValuesNum);
  
  implementIntegerStruct(varValue);
  
  implementIntegerStruct(rVarIndex);
  
  implementIntegerStruct(assignedMesIndex);

  typedef safeVec< probIndex > strideVec;

  typedef safeVec< strideVec > strideVecVec;

  typedef safeVec< varValuesNum > cardVec;
  
  typedef safeVec<rVarIndex> varsVec;

  typedef safeVec<vecIndex> internalIndicesVec ;

  typedef internalIndicesVec::iterator internalIndicesVecIterator;

  typedef internalIndicesVec::const_iterator constInternalIndicesVecIterator;
  
  typedef safeVec<cliqIndex> cliquesVec;

  typedef safeVec< varsVec > separatorsVec;

  typedef safeVec< varsVec > varsVecVector;

  typedef safeVec< cliquesVec > cliquesVecVector;
  
  typedef safeVec<separatorsVec> neighborSeparatorVec;

  typedef safeVec<cliquesVec> adjListVec;
  
  typedef varsVec::iterator varsVecIter;

  typedef varsVec::const_iterator constVarsVecIter;
  
  typedef cliquesVec::iterator cliquesVecIter;

  typedef cliquesVec::const_iterator constCliquesVecIter;

  typedef safeVec< pair<vecIndex,rVarIndex> > varsVecPair;
  
  typedef safeVec< lbInferenceObject_ptr > infObjectsVec;

  typedef safeVec< string > stringVec;

  typedef safeVec< lbMessage_ptr > lbMessagePtrVec;

  typedef safeVec< lbMessagePtrVec > messagePtrVecVector;

  typedef uint vecSize;

  typedef safeVec< vecSize > vecSizeVec;

  typedef safeVec< measIndex > measIndicesVec;

  implementIntegerStructVectorOps(varsVec);

  // Up to 32 different levels of debug printing
  enum verbosity_type {V_ALL=0, 
		       V_PROPAGATION,
		       V_MESSAGES, 
		       V_FRUSTRATION, 
		       V_SUPPORT, 
		       V_MODEL=5, 
		       V_LEARNING,
		       V_LEARNING_DUMP,
		       V_ASSIGNMENTS, 
		       V_EVIDENCE,
		       V_TEMPORARY=10,
		       V_IGNORE_TOTAL_WEIGHT_0,
		       V_MAX};

  class lbDefinitions {

  public:
	
    static const string END_STR ;

    static const char DELIM;

    static const int MAX_BUF_SIZE;

    static const int VEC_SIZE;

    const static probType ZEPSILON;

    const static probType EPSILON;

    const static probType lEPSILON;

    static probType PARAM_EPSILON;

    const static probType TINY;

    const static probType SMALL;
    
    //    static int _numOfVars;

    static int _verbosity;

    static const char *verbose_descriptions[V_MAX];
  };
 

  inline bool checkVerbosity() {
    if (V_MAX > 8*sizeof(lbDefinitions::_verbosity)) {
      cerr << "Number of verbose options exceeds max size." << endl;
      return false;
    }
    
    return true;
  }

  inline bool isVerbose(verbosity_type verbosity) {
    assert (checkVerbosity());

	if ( (lbDefinitions::_verbosity & (1 << verbosity)) > 0 )
		return true;
	else
		return false;
  } 

  inline void printVerbosities() {
    assert (checkVerbosity());
    uint i;
    for (i = 0; i < V_MAX; i++) {
      cerr << i << ": " << lbDefinitions::verbose_descriptions[i] << endl;
    }

    cerr << "Enabled: ";
    for (i = 0; i < V_MAX; i++) {
      if (isVerbose((verbosity_type) i)) {
	cerr << i << " ";
      }
    }
    
    cerr << endl;
  }

  inline void clearVerbose() {
    assert (checkVerbosity());
    lbDefinitions::_verbosity = 0;
  }

  inline void addVerbose(verbosity_type verbosity) {
    assert (checkVerbosity());
    
    if (verbosity == V_ALL) {
      for (int i = 1; i < V_MAX; i++) {
	addVerbose((verbosity_type) i);
      }
    }
    else {
      assert (verbosity < V_MAX);
      lbDefinitions::_verbosity |= (1 << verbosity);
    }
  }

  template <class T>
  string vec2str (const T & container , const string & separator = " ")
  {
    stringstream res ;
    bool first = true ;
    for (typename T::const_iterator iter = container.begin() ;
	 iter != container.end() ;
	 ++iter)
    {
      if (first) {
	first = false ;
      } else {
	res << separator ;
      }

      res << *iter ;
    }

    return res.str() ;
  } // vec2str

  inline stringVec splitString (const string & str , const string & delim) {
    stringVec res ;
    
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delim, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delim, lastPos);
    
    while (string::npos != pos || string::npos != lastPos) {
      // Found a token, add it to the vector.
      res.push_back(str.substr(lastPos, pos - lastPos));
      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of(delim, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delim, lastPos);
    }

    return res ;
  }

};

#endif

