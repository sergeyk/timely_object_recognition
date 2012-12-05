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

#ifndef _Loopy__Belief_Graph_Listener
#define _Loopy__Belief_Graph_Listener

#include <lbGraphStruct.h>

namespace lbLib {

  /**
   * Class: lbGraphListener
   * This class implements an interface to which anyone listening for the
   * changes in a graph must listen.
   *
   * 
   * This is not implemented in lbModel - if you want to use it, you
   * need to implement grpahUpdated method in all the listenrs (e.g., lbModel)
   */

  class lbGraphListener {
  public:
    inline lbGraphListener (lbGraphStruct const * graph);
    inline lbGraphListener () : _graph(NULL){};
    
    virtual inline ~lbGraphListener();
    
    //This graph has been updated
    virtual bool graphUpdated() =0;
  protected:
    lbGraphStruct const * _graph;
  };
  
  inline lbGraphListener::lbGraphListener(lbGraphStruct const * graph) {
    _graph = graph;
    _graph->addListener(*this);
  }
  
  inline lbGraphListener::~lbGraphListener() {
    if (_graph!=NULL) {
      _graph->removeListener(this);
    }
  }
  
};

#endif
