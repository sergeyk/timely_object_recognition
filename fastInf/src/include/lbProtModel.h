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

#ifndef _Loopy__prot__model
#define _Loopy__prot__model

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

  enum lbInteractionType {IT_INTERACTION,IT_TRANSIENT,IT_COMPLEX,IT_TWOHYB,IT_CORRSIG,IT_NUM};

  enum lbProtAttr {PA_LOCAL1,PA_LOCAL1_EXPER,PA_LOCAL2,PA_LOCAL2_EXPER,PA_LOCAL3,
		   PA_LOCAL3_EXPER,PA_LOCAL4,PA_LOCAL4_EXPER,
		   PA_ROLE1,PA_ROLE1_EXPER,PA_ROLE2,PA_ROLE2_EXPER,
		   PA_ROLE3,PA_ROLE3_EXPER,PA_ROLE4,PA_ROLE4_EXPER,PA_NUM};

  //  enum lbCellularRoles {CR_RNAPROCESS,CR_NUM_OF_ROLES};

  //enum lbLocalization {LOC_NUCLEUS,LOC_NUM_OF_LOCALS};

  class lbProtModel {

  public:

    lbProtModel(safeVec< string > & protList);
    ~lbProtModel();
    void learnParams(lbAssignment_ptr evidence);
    void buildFastInf();
    lbAssignment_ptr createEvidence();
    
  private:

    measIndex addMeasureToModel(cardVec const& card);
    void createMeasuresFromFile(ifstream_ptr in);
    void readMeasuresFromScratch();
    void addProtPairVars(lbInteractionType type);
    void addProtAttrVars(lbProtAttr type);
    void addVars();
    void createGraph();
    void createModel();
    void putPairIntoEvidence(lbAssignment_ptr evidence,lbInteractionType type);
    void putProtAttrIntoEvidence(lbAssignment_ptr evidence,lbProtAttr type);
    void unsetPairIntoEvidence(lbAssignment_ptr evidence,int p1,int p2,lbInteractionType type);
    void unsetProtAttrIntoEvidence(lbAssignment_ptr evidence,int p1,lbProtAttr type);

    
    lbVarsList_ptr _varsList;
    lbCardsList_ptr _cardsList;
    lbGraphStruct_ptr _graph;
    lbModel_ptr _model;
    lbInferenceObject_ptr _fastInf;
    lbLearningObject_ptr _learn;
    safeVec< string >& _protList;
    //    string* _attrNames;
    //string* _attrFileNames;
    //string* _pairNames;
    //string* _pairFileNames;
    int _numOfProts;
    int _numOfPairs;


    inline int getIndexForPair(int prot1, int prot2, lbInteractionType interType);
    inline int getIndexForAttr(int prot, lbProtAttr type);

    

  };
  
    inline int lbProtModel::getIndexForPair(int prot1, int prot2, lbInteractionType interType){
      int index  = 0;
      //add previous db's
      index += ((interType)*_numOfPairs);

      //make sure prot2 is smaller
      if (prot2>prot1) {
	int tmp=prot1;
	prot1=prot2;
	prot2=tmp;
      }

      //for each line add its length (line0 : full length ..)
      for (int i=0;i<prot1;i++)
	index+=(_numOfProts-i-1);

      //go to the correct place in line
      index+=prot2;
      return index;
    };

    inline int lbProtModel::getIndexForAttr(int prot,lbProtAttr type){
      int index = _numOfPairs * IT_NUM;
      index += type * _numOfProts;
      index += prot;
      //      cerr<<"index is "<<index<<endl;
      return index;
    };

}
       
       
#endif
