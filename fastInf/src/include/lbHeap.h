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

#ifndef _LB_HEAP__
#define _LB_HEAP__

#include <utility>
#include <map>
#include <vector>
#include <cmath>
#include <iostream>
using namespace std;

/*lbHeap is a data structure of type Binary tree, which satisfies the Heap
  property: for any given Node, it's children's values are never greater
  than it's own [using ValueComparator's operator()].
  It easily allows for in-place sorting, using the heapSort() method.
  Note: Should only ever insert or have 1 of each Item
  Also supports the increaseValue function [assuming ValueComparator puts the
  "larger" Values at the top of the Heap].
*/

namespace lbLib {

  template <class Item, class Value, class ValueComparator = less<Value> >
  class lbHeap {

  public:
    //the int value returned if no such node exists.
    static const int NO_NODE = -1;

    typedef pair<Item,Value> ItemValue;
  
    lbHeap() : _heap(new vector<ItemValue>()), _size(0), _indexMap(new map<Item,int>()) {}
  
    lbHeap(const vector<ItemValue>& itemVals)
      : _heap(new vector<ItemValue>(itemVals)), _size(_heap->size()), _indexMap(new map<Item,int>()) {
      buildHeap();    
    }

    ~lbHeap() {
      delete _heap;
      delete _indexMap;
    }

    //Returns the index of the leftChild of the heap node at index i.
    int leftChild(int i) {
      if (i < _ROOT || i >= _size)
	return NO_NODE;
      int child = 2*i+1;	
      return (child < _size) ? child : NO_NODE;
    }
  
    //Returns the index of the rightChild of the heap node at index i.
    int rightChild(int i) {
      if (i < _ROOT || i >= _size)
	return NO_NODE;
      int child = 2*i+2;	
      return (child < _size) ? child : NO_NODE;
    }
  
    //Returns the index of the parent of the heap node at index i.
    int parent (int i) {
      if (i < _ROOT || i >= _size)
	return NO_NODE;
      return (int)floor((float)(i-1)/2);
    }

    /*Ensures that the sub-tree rooted at i satisfies the heap property,
      assuming that left and right sub-trees are heaps.
      In the worst case, takes O(log height of node i) time.
      @param i the "root" of the subtree on which to check/reorder the
      Heap properties
    */
    void heapify (int i) {
      int left = leftChild(i);
      int right = rightChild(i);
      int largest = i;
      if (left != NO_NODE && _valueCompare((*_heap)[largest].second,(*_heap)[left].second)) {
	largest = left;
      }
      if (right != NO_NODE && _valueCompare((*_heap)[largest].second,(*_heap)[right].second)) {
	largest = right;
      }
      if (largest != i) {
	exchange(i,largest);
	heapify(largest);
      }
    }

    /*Guaranatees that this lbHeap in fact satisfies the Heap property for all
      nodes (by calling heapify() for all nodes, starting from the first
      parent of non-null nodes)
      Takes O(n) time, where n is # of nodes in Heap
    */
    void buildHeap() {
      for (int i = 0; i < _size; ++i) {
	(*_indexMap)[(*_heap)[i].first] = i;
      }
    
      for (int i = parent(_size-1); i >= _ROOT; --i) {
	heapify(i);
      }
    }

    /*Sorts the elements of this Heap in place and returns a vector of the
      elements sorted by the given ValueComparator
      Takes O(n log n) time, where n is the # of elements in the Heap
      @return a sorted vector containing all objects from this Heap.
    */
    vector<ItemValue>* heapSort() {
      //1st resize _heap to correct _size:
      _heap->resize(_size);
      //Don't want to ruin current heap, so copy it for later
      vector<ItemValue>* originalVec = new vector<ItemValue>(*_heap);
      map<Item,int>* originalMap = new map<Item,int>(*_indexMap);    
      int originalSize = _size;
    
      for (int i = _size-1; i > _ROOT; --i) {
	exchange(_ROOT,i);
	--_size;
	heapify(_ROOT);
      }
      vector<ItemValue>* returnTemp = _heap;
      _heap = originalVec;
      _size = originalSize;
      delete _indexMap;
      _indexMap = originalMap;
      return returnTemp;
    }

    Value getValue(Item itm) {
      typename map<Item,int>::const_iterator iter = _indexMap->find(itm);
      if (iter == _indexMap->end()) {
	throw new string("Invalid Item requested in lbHeap::getValue()!");
      }
      int i = iter->second;
      return (*_heap)[i].second;
    }
  
    /*Sets the value of Item it to Value val,
      assuming that key may cause that element to be considered larger by the
      ValueComparator, and fixes the Heap accordingly.  NOTE: the key sent should
      not cause the ValueComparator to consider the element smaller, and if so this
      method will decrease the Key, but will not properly fix the Heap
      accordingly -- i.e. doing so ruins this Heap.
    */
    bool increaseValue(Item itm, Value val) {
      typename map<Item,int>::const_iterator iter = _indexMap->find(itm);
      if (iter == _indexMap->end()) {
	return false;
      }
      int i = iter->second;
      (*_heap)[i].second = val;
      fixUpHeap(i);
      return true;
    }
  
    /*Inserts (item,val) into this Heap, maintaining the Heap property.
      The worst-case running time is O(log n), if the element is inserted or
      not (because it is a duplicate).    
    */
    void insert(Item item, Value val) {
      typename map<Item,int>::const_iterator iter = _indexMap->find(item);
      if (iter != _indexMap->end()) {
	return;
      }
      if(_size == (int)_heap->size()) {//To extend vector size:
	_heap->resize(2 * _size + 1);//double the size, so that have amortized cost of O(1)
      }
      int i = _size++;
      (*_indexMap)[item] = i;
      (*_heap)[i] = ItemValue(item,val);
      fixUpHeap(i);
    }

    bool empty(){
      return (_size == 0);
    }

    int size() {
      return _size;
    }
    
    //Prints the Heap in the order of it's indices (0 to n-1).
    friend ostream& operator<<(ostream& stream, lbHeap const& h) {
      for (int i = _ROOT;i < h._size; ++i) {
	ItemValue itVal = (*h._heap)[i];
	stream << "(" << itVal.first << "," << itVal.second << ") ";    
      }
      stream << endl;
      return stream;
    }
    
  protected:
    vector<ItemValue>* _heap;   
    int _size;

    //map of items to indices:
    map<Item,int>* _indexMap;
  
    ValueComparator _valueCompare;  

    //The "index" of the top of this Heap.
    static const int _ROOT = 0;
  
    /*Exchanges two objects in this Heap, one in place of the other.
      @param i index of first object to switch.
      @param j index of second object to switch.
    */
    void exchange(int i,int j) {
      (*_indexMap)[(*_heap)[i].first] = j;
      (*_indexMap)[(*_heap)[j].first] = i;
    
      ItemValue temp = (*_heap)[i];
      (*_heap)[i] = (*_heap)[j];
      (*_heap)[j] = temp;
    }

    /*Ensures that the Heap satisfies the Heap property, assuming that the
      only problem may be that the element at index i may be "larger" (by the
      ValueComparator) than its parent (so pushes it up toward the root, as necessary)
      @param i the index of the element which may have exclusively ruined the
      heap property by being larger (by the ValueComparator) than its parents.
    */
    void fixUpHeap (int i) {
      while (i > _ROOT && _valueCompare((*_heap)[parent(i)].second,(*_heap)[i].second)){
	exchange(i,parent(i));
	//exchanges the Objects at indices i,parent(i)
	i = parent(i);
	/*advance i, so that go up the tree making sure that now the Object
	  at parent(i) is less than its parent*/
      }
    }

    void printIndexMap(ostream& stream) {
      stream << endl << "heap:" << endl << *this << endl;
      stream << "IndexMap:" << endl;
      for (typename map<Item,int>::const_iterator it = _indexMap->begin();
	   it != _indexMap->end(); ++it) {
	stream << it->first << ": " << it->second << endl;
      }
    }
  
  };

}


#endif
