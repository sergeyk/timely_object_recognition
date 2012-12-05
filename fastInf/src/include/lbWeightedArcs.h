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

#ifndef _Weighted_Arcs_
#define _Weighted_Arcs_

#include <lbGraphBase.h>
#include <lbRedBlackTree.h>

namespace lbLib {
  class lbWeightedArcs {
  public:
    lbWeightedArcs() {  }

    inline void resize(int size) { _weightedArcs.resize(size, NULL); }
    inline int size() const { return _weightedArcs.size(); }

    inline void setWeight(arcIndex ai, double weight);
    inline double getWeight(arcIndex ai) const;
    inline double getMaxWeight();
    inline arcIndex getMaxArcIndex();
    
  private:
    
    TStorage<double, arcIndex> _rbt;
    vector<TStorageNode<double, arcIndex> *> _weightedArcs;
  };

  inline void lbWeightedArcs::setWeight(arcIndex ai, double weight) {
    assert (ai >= 0 && ai < (int) _weightedArcs.size());
    
    if (_weightedArcs[ai] != NULL) {
      _rbt.Delete(_weightedArcs[ai]);
    }
    
    _weightedArcs[ai] = _rbt.Insert(weight, ai);
  }
  
  inline double lbWeightedArcs::getWeight(arcIndex ai) const {
    assert (ai >= 0 && ai < (int) _weightedArcs.size());
    return _weightedArcs[ai]->GetKey();
  }

  inline double lbWeightedArcs::getMaxWeight() {
    TStorageNode<double, arcIndex> * max = _rbt.GetLast();
    return max->GetKey();    
  }

  inline arcIndex lbWeightedArcs::getMaxArcIndex() {
    TStorageNode<double, arcIndex> * max = _rbt.GetLast();
    return max->GetData();
  }

};

#endif
