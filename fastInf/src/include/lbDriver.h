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


#ifndef _Loopy__Driver
#define _Loopy__Driver


#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <lbDefinitions.h>
#include <lbRandomVar.h>
#include <lbMeasure.h>
#include <lbGraphStruct.h>
#include <lbModel.h>
#include <lbMeasureDispatcher.h>

using namespace std;

namespace lbLib {

  class lbDriver;
  typedef lbDriver* lbDriver_ptr;

  class lbDriver {
  
  public:
    
    lbDriver(lbMeasureDispatcher const& disp);
    
    ~lbDriver();
    
    bool readUniverse(string inputFileName, bool readLogValues = false);

    static void writeUniverse(lbModel const& model, string outputFileName,
			      bool writeLogValues = false);
    
    static void writeUniverse(lbModel const& model, ostream& os,
			      bool writeLogValues = false);
    
    //method that exits the universe
    void exitDriver(string msg);
    
    lbModel & getModel();

    lbGraphStruct & getGraph();

    lbVarsList & getVars();

    lbCardsList & getCards();

  private:
    
    lbModel_ptr _model;
    
    lbGraphStruct_ptr _graph;
    
    lbVarsList_ptr _vars;
    
    lbCardsList_ptr _cards;

    lbMeasureDispatcher const& _measDisp;
    
    bool readVariables(ifstream_ptr in);

    bool readExact(ifstream_ptr in, lbModel *model);


    //Constants for the driver :
    
    static const string END_STR ;

    static const char DELIM ;
    
    static const int MAX_BUF_SIZE ;
    
    const static int ERROR_MODE;
    
    const static string VARS_STR;

    const static string EXACT_STR;

    const static string MEASURE_STR;
    
    const static string CLIQUES_STR;
    
    const static string CLIQUE_TO_MEASURE_STR;
    
    const static string IDLE_PARAMS ;
    
    const static string SHARED_PARAMS ;

    const static string DIRECTED_MEASURES_LIST ;

    
  };

  //typedef shared_ptr<lbDriver> lbDriver_ptr;
  
}
#endif
