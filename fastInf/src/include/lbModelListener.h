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

#ifndef _Loopy__Belief_Model_Listener
#define _Loopy__Belief_Model_Listener

#include <lbModel.h>

namespace lbLib {

  class lbModelListener;
  
  typedef lbModelListener* lbModelListener_ptr;
  typedef safeVec <lbModelListener_ptr> listenersVec;
  typedef listenersVec::iterator listenersVecIter;
  
  class lbModelListener {

  public:
    inline lbModelListener(lbModel& model);
    virtual inline ~lbModelListener();
    virtual bool factorsUpdated(measIndicesVec const& vec)=0;
    lbModel const& getModel() const { return _model; }
    
  protected:
    lbModel& _model;
    lbGraphStruct const& _graph;
    int _index;
    bool _modifyAndDeleteModel;
  };

  inline lbModelListener::lbModelListener(lbModel& model): 
    _model(model),_graph(model.getGraph()),_modifyAndDeleteModel(false) {
    _index = _model.addListener(*this); 
  };
 
  inline lbModelListener::~lbModelListener(){ 
    _model.removeListener(_index);

    if (_modifyAndDeleteModel)
      delete &_model;
  };
};
#endif
