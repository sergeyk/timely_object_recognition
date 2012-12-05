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

#ifndef _Loopy__utils
#define _Loopy__utils

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "lbDefinitions.h"

namespace lbLib {
  
      /**
   This is a genearl utils file
   
   Part of the loopy belief library
   @version November 2002
   @author Ariel Jaimovich
*/

      class lbUtils {

      public:
	
	lbUtils();
	~lbUtils();

	static ifstream_ptr getSmartFileStream(const char* fileName);
	static vector<string> tokenize(const string& str,const string& delimiters);
	
      private:
	
      };
}


#endif

      
