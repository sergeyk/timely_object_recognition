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

#include <lbDefinitions.h>

using namespace lbLib ;

const string lbDefinitions::END_STR = string("@End");

const char lbDefinitions::DELIM = '\t';

const int lbDefinitions::MAX_BUF_SIZE = 1000000;

const int lbDefinitions::VEC_SIZE = 50;
  
//int lbDefinitions::_numOfVars = 11;

int lbDefinitions::_verbosity = 0;

const char *lbDefinitions::verbose_descriptions[] = {
  "All",
  "Propagation",
  "Messages",
  "Frustration",
  "Support",
  "Model",
  "Learning",
  "Learning File Dumps",
  "Assignments",
  "Evidence",
  "Temporary",  // Optional prints for short-term debugging
  "Ignore Total Weights of 0"
};

const probType lbDefinitions::ZEPSILON = 1e-250;

const probType lbDefinitions::EPSILON = 1e-10;

const probType lbDefinitions::lEPSILON = log(1e-10);

const probType lbDefinitions::TINY = 1e-7;

const probType lbDefinitions::SMALL = 1e-4;

probType lbDefinitions::PARAM_EPSILON = 1e-10;
