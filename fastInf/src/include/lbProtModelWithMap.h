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

#ifndef _Loopy__prot__model_wm
#define _Loopy__prot__model_wm

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
#include <lbLoopyInf.h>
#include <lbLearningObject.h>
#include <lbSuffStat.h>


namespace lbLib {

  enum lbInteractionType {IT_INTERACTION,IT_COMPLEX,IT_TWOHYB,IT_CORRSIG,IT_NUM};

  enum lbProtAttr {PA_LOCAL1,PA_LOCAL1_EXPER,PA_LOCAL2,PA_LOCAL2_EXPER,PA_LOCAL3,
		   PA_LOCAL3_EXPER,PA_LOCAL4,PA_LOCAL4_EXPER,
  		   PA_ROLE1,PA_ROLE1_EXPER,PA_ROLE2,PA_ROLE2_EXPER,
		   PA_ROLE3,PA_ROLE3_EXPER,PA_ROLE4,PA_ROLE4_EXPER,
		   PA_NUM};

  typedef int protIndex;
  
  struct triplet {
    triplet(protIndex prot1,protIndex prot2,protIndex prot3):
      _prot1(prot1),
      _prot2(prot2),
      _prot3(prot3) { };
    triplet() {} ;
    protIndex _prot1,_prot2,_prot3;
  };

  typedef pair< protIndex,protIndex > protPair;
  typedef map<string,protIndex> protMap;
  typedef map<protIndex,string> protNamesMap;
  typedef map<int,protPair> protPairMap;
  typedef map<protPair,int> protPairNamesMap;
  typedef protPairMap::iterator protPairMapIter ;
  typedef map<protPair,rVarIndex> protPairAttrMap;
  typedef map<protIndex,rVarIndex> protAttrMap;
  typedef map<varsVec,cliqIndex> cliquesMap;
  //typedef map<char*,measIndex> measureMap;
  typedef safeVec<triplet> tripletsVec;
  typedef vector<protAttrMap> vecOfAttrMaps;
  typedef vector<protPairAttrMap> vecOfPairAttrMaps;

  class lbProtModelWithMap {

  public:

    lbProtModelWithMap(protMap & protList,
		       protPairMap & pairMap ,
		       protNamesMap & protNames,
		       tripletsVec & tripVec,
		       bool withChain);
    ~lbProtModelWithMap();
    void learnParams(lbAssignment_ptr evidence);
    void buildFastInf(bool test,lbDriver* driver=NULL);
    lbAssignment_ptr createEvidence();
    void printPredictedInteractions(lbAssignment_ptr evidence,bool testPairs);
    
  private:
    lbMeasureDispatcher* _disp;
    //measIndex addMeasureToModel(cardVec const& card);
    void createMeasuresFromFile(ifstream_ptr in,bool test);
    void readMeasuresFromScratch();
    void addProtPairVars(lbInteractionType type);
    void addProtAttrVars(lbProtAttr type);
    void addVars();
    void createGraph();
    void createModel(bool test);
    void putPairIntoEvidence(lbAssignment_ptr evidence,lbInteractionType type);
    void putProtAttrIntoEvidence(lbAssignment_ptr evidence,lbProtAttr type);
    void unsetPairIntoEvidence(lbAssignment_ptr evidence,int p1,int p2,lbInteractionType type);
    void unsetProtAttrIntoEvidence(lbAssignment_ptr evidence,int p1,lbProtAttr type);
    void addNeighborsToInteraction(int prot1,lbProtAttr attrType,cliqIndex cliq) ;
    protPairMap* removeTestPairsFromEvid(lbAssignment_ptr evidence) ;
    
    lbVarsList_ptr _varsList;
    lbCardsList_ptr _cardsList;
    lbGraphStruct_ptr _graph;
    lbModel_ptr _model;
    lbInferenceObject_ptr _fastInf;
    lbLearningObject_ptr _learn;
    protMap& _protList;
    protNamesMap& _namesMap;
    protPairMap& _pairsList;
    tripletsVec& _tripVec;
    bool _withChain;
    //    string* _attrNames;
    //string* _attrFileNames;
    //string* _pairNames;
    //string* _pairFileNames;

    vecOfPairAttrMaps _pairAttrMaps;
    vecOfAttrMaps _attrMaps;
    cliquesMap _cliqMap;
    //measureMap _measMap;

    int _numOfProts;
    int _numOfPairs;


    inline rVarIndex getIndexForPair(int prot1, int prot2, lbInteractionType interType);
    inline rVarIndex getIndexForAttr(int prot, lbProtAttr type);
    inline cliqIndex getIndexForClique(varsVec const& vars);
    

  };
  
    inline rVarIndex lbProtModelWithMap::getIndexForPair(int prot1,
							 int prot2,
							 lbInteractionType interType){
      protPair pair(prot1,prot2);
      uint size = _pairAttrMaps[interType].size();
      rVarIndex temp = _pairAttrMaps[interType][pair];
      if (size!=_pairAttrMaps[interType].size()){
	cerr<<"Warning : pair : "<<prot1<<" , "<<prot2<<" is not part of the game"<<endl;
	return -1;
      }
      return temp;
      
    };

    inline rVarIndex lbProtModelWithMap::getIndexForAttr(int prot,lbProtAttr type){
      return _attrMaps[type][prot];
    };

    inline cliqIndex lbProtModelWithMap::getIndexForClique(varsVec const& vars) { 
      return _cliqMap[vars];
    }
}
       
       
#endif
