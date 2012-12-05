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

#ifndef _LB_PRIORITY_QUEUE__
#define _LB_PRIORITY_QUEUE__

#include <lbHeap.h>
using namespace lbLib;

namespace lbLib {

  /*lbPriorityQueue Class is an implementation of the lbPriorityQueue interface.
    It is implemented using the lbHeap data structure, so that it inherits from lbHeap.
  */

  template <class Item, class Value, class ValueComparator = less<Value> >
  class lbPriorityQueue : public lbHeap<Item,Value,ValueComparator> {

  public:
    typedef lbHeap<Item,Value,ValueComparator> HeapParent;
  
    //Constructs an empty lbPriorityQueue with the given ValueComparator
    lbPriorityQueue() : lbHeap<Item,Value,ValueComparator>() {}

    //Constructs a lbPriorityQueue with the given vector of (item,value)s
    lbPriorityQueue(const vector<typename lbPriorityQueue::ItemValue>& itemVals)
      : lbHeap<Item,Value,ValueComparator>(itemVals) {}
  
    Item maximum() {
      if (HeapParent::_size == 0)
	throw new string("Empty lbPriorityQueue from maximum()");
      return (*HeapParent::_heap)[HeapParent::_ROOT].first;
    }

    Item deleteMax() {
      if (HeapParent::_size == 0)
	throw new string("Empty lbPriorityQueue from deleteMax()");

      Item max = (*HeapParent::_heap)[HeapParent::_ROOT].first;
      exchange(HeapParent::_ROOT,--HeapParent::_size);
      heapify(HeapParent::_ROOT);    
      HeapParent::_indexMap->erase(max);   
      return max;
    }
  };

}


#endif
