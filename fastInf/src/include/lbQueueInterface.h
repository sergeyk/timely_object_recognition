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

#ifndef _Queue_Interface_
#define _Queue_Interface_

#include <lbDefinitions.h>

namespace lbLib {
  typedef pair<cliqIndex, cliqIndex> messageIndex;

  struct lessMessageIndex {
    bool operator()(messageIndex const& m1, messageIndex const& m2) const {
      if (m1.first < m2.first) {
	return true;
      }
      else if (m1.first == m2.first) {
	return (m1.second < m2.second);
      }

      return false;
    }
  };

  typedef safeVec<messageIndex> messageIndexVec;
  typedef map<messageIndex, bool, lessMessageIndex> messageMask; 

  class lbQueueInterface {
  public:
    lbQueueInterface() {}
    virtual ~lbQueueInterface() {}

    virtual void push(messageIndex mi, double weight = 0) = 0;

    virtual messageIndex pop() = 0;
    virtual messageIndexVec pop(int size) = 0;
    virtual messageIndexVec top(int size) const = 0;
    virtual void clear() = 0;

    virtual void remove(messageIndex mi) = 0;

    virtual bool empty() const = 0;
    virtual int size() const = 0;

    virtual void print(ostream & out) const = 0;
  };
};

#endif
