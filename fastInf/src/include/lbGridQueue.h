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

#ifndef _Grid_Queue_
#define _Grid_Queue_

#include <lbManualQueue.h>
#include <lbGraphStruct.h>

namespace lbLib {

  /*
   * This is a queue is specific to grids with a special format.
   */

  typedef enum { GQT_BARS, GQT_CRISSCROSS, GQT_SNAKES } gridQueueType;

  class lbGridQueueOrder {
  public:
    lbGridQueueOrder(lbGraphStruct const & graph);
    virtual ~lbGridQueueOrder() {}

    messageIndexVec getOrdering(gridQueueType gqt);

  private:

    messageIndexVec createBars();
    messageIndexVec createCrisscross();
    messageIndexVec createSnakes();

    messageIndexVec reverse(messageIndexVec const & old);
    void add(messageIndexVec & original, messageIndexVec const & miv);
    int getNumRows() { return _rootSize; }
    int getNumCols() { return _rootSize; }
    void computeSize(lbGraphStruct const& graph);
    messageIndexVec getRowL2R(int i);
    messageIndexVec getColU2D(int i);

    int _rootSize;
    int _size;
  };

  lbGridQueueOrder::lbGridQueueOrder(lbGraphStruct const& graph) {
    computeSize(graph);
  }

  messageIndexVec lbGridQueueOrder::getOrdering(gridQueueType gqt) {
    switch (gqt) {
    case GQT_BARS:
      return createBars();
      break;
    case GQT_CRISSCROSS:
      return createCrisscross();
      break;
    case GQT_SNAKES:
      return createSnakes();
      break;
    default:
      NOT_REACHED;
    }

    return messageIndexVec();
  }

  // Just a set of horizontal and vertical bars
  messageIndexVec lbGridQueueOrder::createBars() {
    messageIndexVec ordering;

	int i;
    for (i = 0; i < getNumRows(); i++) {
      messageIndexVec row = getRowL2R(i);
      add(ordering, row);
      add(ordering, reverse(row));
    }

    for (i = 0; i < getNumCols(); i++) {
      messageIndexVec col = getColU2D(i);
      add(ordering, col);
      add(ordering, reverse(col));
    }

    return ordering;
  }

  // A set of vertical bars with a horizontal connector and a set of
  // horizontal bars with a vertical connector.
  messageIndexVec lbGridQueueOrder::createCrisscross() {
    messageIndexVec miv, ordering;

	int i;
    for (i = 0; i < getNumRows()-1; i++) {
      messageIndexVec row = getRowL2R(i);
      add(miv, row);
    }

    add(miv, getColU2D(getNumCols() - 1));
    add(ordering, miv);
    add(ordering, reverse(miv));

    miv.clear();

    for (i = 0; i < getNumCols()-1; i++) {
      messageIndexVec col = getColU2D(i);
      add(miv, col);
    }

    add(miv, getRowL2R(getNumRows() - 1));
    add(ordering, miv);
    add(ordering, reverse(miv));

    return ordering;
  }

  // One snake swerving up and down, one snake swerving left and
  // right.
  messageIndexVec lbGridQueueOrder::createSnakes() {
    messageIndexVec miv, ordering;

	int i;
    for (i = 0; i < getNumRows()-1; i++) {
      messageIndexVec row = getRowL2R(i);
      messageIndexVec col;

      if (i % 2 == 0) {
	col = getColU2D(getNumCols() - 1);
      }
      else {
	row = reverse(row);
	col = getColU2D(0);
      }

      add(miv, row);
      miv.push_back(col[2*i]);
      miv.push_back(col[2*i+1]);
    }

    add(miv, getRowL2R(getNumRows() - 1));
    add(ordering, miv);
    add(ordering, reverse(miv));

    miv.clear();

    for (i = 0; i < getNumCols()-1; i++) {
      messageIndexVec col = getColU2D(i);
      messageIndexVec row;

      if (i % 2 == 0) {
	row = getRowL2R(getNumRows() - 1);
      }
      else {
	col = reverse(col);
	row = getRowL2R(0);
      }

      add(miv, col);
      miv.push_back(row[2*i]);
      miv.push_back(row[2*i+1]);
    }

    add(miv, getColU2D(getNumCols() - 1));
    add(ordering, miv);
    add(ordering, reverse(miv));

    return ordering;
  }

  void lbGridQueueOrder::add(messageIndexVec & original, messageIndexVec const & addition) {
    for (uint i = 0; i < addition.size(); i++) {
      original.push_back(addition[i]);
    }
  }

  messageIndexVec lbGridQueueOrder::reverse(messageIndexVec const & old) {
    messageIndexVec miv;

    for (uint i = 0; i < old.size(); i++) {
      messageIndex mi = old[old.size() - 1 - i]; 
      messageIndex newmi(mi.second, mi.first);
      miv.push_back(newmi);
    }

    return miv;
  }

  void lbGridQueueOrder::computeSize(lbGraphStruct const & graph) {
    for (_rootSize = 0; true; _rootSize++) {
      _size = _rootSize*_rootSize;

      int numCliques = 3*_size - 2*_rootSize;

      if (numCliques == graph.getNumOfCliques()) {
	break;
      }

      if (_rootSize > 1000) {
	cerr << "Having trouble computing grid size..." << endl;
      }
    }
  }

  messageIndexVec lbGridQueueOrder::getRowL2R(int r) {
    messageIndexVec miv;

    for (int i = 0; i < _rootSize - 1; i++) {
      int first = r*_rootSize + i;
      int second = r*_rootSize + i + 1;
      int middle = first + _size - r;
      miv.push_back(messageIndex(first, middle));
      miv.push_back(messageIndex(middle, second));
    }
    return miv;
  }

  messageIndexVec lbGridQueueOrder::getColU2D(int c) {
    messageIndexVec miv;
    int colstart = 2*_size - _rootSize;

    for (int i = 0; i < _rootSize - 1; i++) {
      int first = c + i*_rootSize;
      int second = c + (i+1)*_rootSize;
      int middle = colstart + c*(_rootSize-1) + i;

      miv.push_back(messageIndex(first, middle));
      miv.push_back(messageIndex(middle, second));
    } 

    return miv;
  }

};

#endif
