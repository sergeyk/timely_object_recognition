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

#ifndef _Loopy__Message
#define _Loopy__Message

#include <lbDefinitions.h>
#include <lbAssignedMeasure.h>
#include <lbMeasureDispatcher.h>

namespace lbLib {
  /**
     This Class holds the loopy belief message
     
     Part of the loopy belief library
     @version November 2002
     @author Ariel Jaimovich
  */
  
  
  class lbMessage {
    
  public:
    
    inline lbMessage() ;

    inline lbMessage(cliqIndex fromCliq, cliqIndex toCliq,varsVec const& vars);

    inline lbMessage(cliqIndex fromCliq, cliqIndex toCliq);

    explicit lbMessage(lbMessage const& otherMessage);

    lbMessage& operator= (lbMessage const& otherMessage);
    
    ~lbMessage();

    internalIndicesVec findCommonVars(varsVec const& vec1,
				      varsVec const& vec2);
    
    inline bool isDifferent(lbAssignedMeasure& other,
			    const lbMessageCompareType type=C_KL,
			    probType eps=lbDefinitions::EPSILON) const ;
    
    inline void smoothMessage(lbAssignedMeasure_ptr assignedMeasurePtr,probType smoothParam) ;
    
    void createMessage(lbAssignedMeasure const& fromMes,
		       lbAssignedMeasure const& toMes,
		       varsVec const& separator,
		       cardVec const& cards,
		       lbMeasureDispatcher const& disp,
		       lbMessageInitType initType) ;

    inline lbAssignedMeasure& getAssignedMeasure() const;

    inline void setAssignedMeasure(lbAssignedMeasure_ptr assignedMeasurePtr);
			    
    // inline bool isUpdated() const;
    //inline void update(bool updated);
    
    inline bool isInQueue() const;

    inline void queueStatus(bool status);

    inline cliqIndex fromCliq() const;

    inline void setFromCliq(cliqIndex cliq);

    inline cliqIndex toCliq() const;

    inline void setToCliq(cliqIndex cliq);
    
    inline varsVec const& getVars() const;

    inline void setVars(varsVec const& vars);

    inline void updateConvergence(bool converged);

    inline bool hasConverged() const;

    inline bool isRelevant() const;

    inline void updateRelevance(bool relevance);
    
    void print(ostream & out) const;

  private:
      
    lbAssignedMeasure_ptr _assignedMes;

    bool _isRelevant;
    
    // bool _updated;
    
    bool _converged;

    bool _inQueue;
    
    cliqIndex _fromCliq;

    cliqIndex _toCliq;

    varsVec _vars;

    

  };

  
inline lbMessage::lbMessage() {
}

inline lbMessage::lbMessage(cliqIndex fromCliq, cliqIndex toCliq,varsVec const& vars):
  _vars(vars){
  _assignedMes=NULL;
  _fromCliq=fromCliq;
  _toCliq=toCliq;
  //  _updated=true;
  _inQueue=false;
  _converged=false;
  _isRelevant=true;
}

inline lbMessage::lbMessage(cliqIndex fromCliq, cliqIndex toCliq){
  _assignedMes=NULL;
  _fromCliq=fromCliq;
  _toCliq=toCliq;
  //  _updated=true;
  _inQueue=false;
  _converged=false;
  _isRelevant=true;
}

inline lbMessage::~lbMessage(){
  if (_assignedMes)
    delete _assignedMes;  
}

inline lbAssignedMeasure& lbMessage::getAssignedMeasure() const{
  return *_assignedMes;
}

inline void lbMessage::setAssignedMeasure(lbAssignedMeasure_ptr assignedMeasurePtr){
  
  if (_assignedMes)
    delete _assignedMes;
  
  _assignedMes=assignedMeasurePtr;
}

inline bool lbMessage::isDifferent(lbAssignedMeasure& other,const lbMessageCompareType type,probType eps) const{
  return _assignedMes->isDifferent(other,type,eps);
}
  /*
bool lbMessage::isUpdated() const{
  return _updated;
}

void lbMessage::update(bool updated){
  _updated=updated;
}
  */
inline bool lbMessage::isInQueue() const{
  return _inQueue;
}

inline void lbMessage::queueStatus(bool status){
  _inQueue=status;
}

inline cliqIndex lbMessage::fromCliq() const{
  return _fromCliq;
}

inline void lbMessage::setFromCliq(cliqIndex cliq){
  _fromCliq=cliq;
}

inline cliqIndex lbMessage::toCliq() const{
  return _toCliq;
}

inline void lbMessage::setToCliq(cliqIndex cliq){
  _toCliq=cliq;
}
    
inline varsVec const& lbMessage::getVars() const{
  return _vars;
}

inline void lbMessage::setVars(varsVec const& vars){
  _vars = varsVec(vars);
}

inline bool lbMessage::hasConverged() const{
  return _converged;
}

inline void lbMessage::updateConvergence(bool newStatus){
  _converged=newStatus;
}

inline bool lbMessage::isRelevant() const {
  return _isRelevant;
}

inline void lbMessage::updateRelevance(bool relevance) {
  _isRelevant=relevance;
}


//replace current message by the new measure (smoothed by current one)
inline void lbMessage::smoothMessage(lbAssignedMeasure_ptr assignedMeasurePtr,probType smoothParam) {
  if (smoothParam>=1 || smoothParam<0)
    cerr<<"smooth param is illegal"<<endl;
  else {
    //take new measure and smooth it by old one, then assign it to this message
    _assignedMes->updateMeasureValues(assignedMeasurePtr->getMeasure(),smoothParam);
  }
}

};

#endif

