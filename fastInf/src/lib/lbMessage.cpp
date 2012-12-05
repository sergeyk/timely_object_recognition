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

#include <lbMessage.h>
 
using namespace lbLib;

lbMessage::lbMessage(lbMessage const& otherMessage) {
  
  _assignedMes = new lbAssignedMeasure(*otherMessage._assignedMes);
  _isRelevant = otherMessage._isRelevant;
  //  _updated = otherMessage._updated;
  _converged =otherMessage._converged;
  _inQueue =otherMessage._inQueue;
  _fromCliq = otherMessage._fromCliq;
  _toCliq = otherMessage._toCliq;
  _vars = varsVec(otherMessage._vars);
}

lbMessage& lbMessage::operator= (lbMessage const& otherMessage){
  if (_assignedMes)
    delete _assignedMes;
  _assignedMes = new lbAssignedMeasure(*otherMessage._assignedMes);
  _isRelevant = otherMessage._isRelevant;    
  //  _updated = otherMessage._updated;
  _converged =otherMessage._converged;
  _inQueue =otherMessage._inQueue;
  _fromCliq = otherMessage._fromCliq;
  _toCliq = otherMessage._toCliq;
  _vars = varsVec(otherMessage._vars);
  return *this;
}

internalIndicesVec lbMessage::findCommonVars(varsVec const& vec1,
					     varsVec const& vec2){
  assert(false);
  internalIndicesVec indices = internalIndicesVec();
  /*
  _vars = varsVec();
  
  //  bool_ptr array(new bool[lbDefinitions::_numOfVars]);
  uint i;
  //for (i =0;i<(uint)lbDefinitions::_numOfVars;i++)
  //array[i]=false;
  for (i =0;i<vec1.size();i++){
    array[vec1[i]]=true;
  }
  for (i =0;i<vec2.size();i++)
    if (array[vec2[i]]) {
       indices.push_back(vecIndex(i));
      _vars.push_back(vec2[i]);
    }
  */
  //  delete[] array;
  return indices;
}

    
void lbMessage::createMessage(lbAssignedMeasure const& fromMes,
			      lbAssignedMeasure const& toMes,
			      varsVec const& separator,
			      cardVec const& cards,
			      lbMeasureDispatcher const& disp,
			      lbMessageInitType initType) {
  
  _vars=varsVec(separator);
  lbMeasure_Sptr measPtr;
  if (initType==MIT_RANDOM)
    measPtr = disp.getNewMeasure(cards,true);
  else if(initType==MIT_UNIFORM)
    measPtr = disp.getNewMeasure(cards,false);
  _assignedMes= new lbAssignedMeasure(measPtr,_vars);
  _assignedMes->normalize();
}
    
void lbMessage::print(ostream & out) const
{
  out << "Message from clique " << _fromCliq;
  out << " to clique " << _toCliq << endl;
  _assignedMes->print(out);
  out << "isRelevant: " << _isRelevant << endl;
  out << "converged: " << _converged << endl;
  out << "inQueue: " << _inQueue << endl;
}
