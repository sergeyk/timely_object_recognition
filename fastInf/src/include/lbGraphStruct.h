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

#ifndef _Loopy__Graph_Structure
#define _Loopy__Graph_Structure

#include <lbDefinitions.h>
#include <lbVarsList.h>
#include <lbCardsList.h>
#include <set>


namespace lbLib {


  class lbGraphStruct;
  typedef lbGraphStruct* lbGraphStruct_ptr;
  
  enum edgeUpdateType { ADDEDGE, DELETEEDGE };
  /*!
     This Class holds a Graph structure.
   
     General Idea :
     This class holds the graph structure.
     the structure is implied by the clique list, and all
     the connections between cliques.
     cliques do not really exist as Objects, but have indices.
     each index has an entry in a vector with its random vars list,
     and an entry in a vector with its neighbouring cliques list.
   
     Part of the fastInf library
     \version July 2009
     \author Ariel Jaimovich
  */
  
  class lbGraphStruct {

  public:

    /*
**********************************************************
Ctors and Dtors :
**********************************************************
*/
    /*!
    Default contsructor 
    \param varsList are the varibales over which this graph is defined    
    */
    lbGraphStruct(lbVarsList const & varsList, bool deepDest = false);

    /*!
    Copy constructor
    */
    explicit lbGraphStruct(lbGraphStruct const& oldGraph) ;

    /*!
      Destructor
     */
    ~lbGraphStruct();

    /*  
**********************************************************
Mutators
**********************************************************
*/
    //sets graph vars - dangerous to use after adding cliques !!!
    //inline void setGraphVars(lbVarsList & varsList);
    /*!
    Add Random variable to clique
    \param clique is the index of the clique
    \param var is the index of the variable
    \retrun true if the addition was succesful
    */
    bool addRandomVarToClique (cliqIndex clique, rVarIndex var); 

    /*!
    Remove Random Variable from clique
    \param clique is the index of the clique
    \param var is the index of the variable
    \retrun true if the removal was succesful
    */
    bool removeRandomVarFromClique (cliqIndex clique, rVarIndex var);
    
    /*!
    Add a clique to the graph (return its index)
    \param varsList is the graph we want to add
    \param cliqName is the name of the clique (default is empty name)
    \return the index of the added clique.
    */
    cliqIndex addClique(const varsVec& varsList, string cliqName="");

    /*!
    Sets the clique neighbors (neighbors are added in both directions)
    \param clique is the index of the clique to which we are adding neighbors
    \param cliqueNeighbors is the list of indices of the neighbors we want to add
    \return true if succesful
    */
    bool setCliqueNeighbors (cliqIndex clique, cliquesVec& cliqueNeighbors);

    /*!
    Add one neighbor to a clique. Here, the seprator set is aloways set to be the intersection 
    of the variable lists of both cliques
    (neighbors are added in both directions)
    \param clique is the index of the clique to which we are adding the neighbor
    \param neighbor is the clique index of the neighbor being adeed
    \return true if the operation was sucessful
    */
    bool addCliqueNeighbor(cliqIndex clique, cliqIndex neighbor);

    /*!
    Add one neighbor to a clique. Here, the seprator set is defined by the user
    (neighbors are added in both directions)
    \param clique is the index of the clique to which we are adding the neighbor
    \param neighbor is the clique index of the neighbor being adeed
    \param separator is the list of var indices to be included in the separator set
    \return true if the operation was sucessful
    */
    bool addCliqueNeighbor(cliqIndex clique, cliqIndex neighbor, varsVec separator);
    

    /*!
      adds a listener to this graph.
    */
    void addListener(lbGraphListener & listener) const;
    
    /*!
      Removes the listener at INDEX from the list of listeners.
    */
    void removeListener(lbGraphListener_ptr listener) const;
    
    /*!
      Updates automatically any stored messages for the listeners
    */
    void updateListeners() const;


    /*!
    Remove  a clique neighbor
    */
    bool removeCliqueNeighbor(cliqIndex clique, cliqIndex Neighbor);

    /*!
    Adds automatically neighbors (an edge between each two
    cliques that share at least variable). This can be used in case we don't want to assign all neighborhoods
    manually. However, this method will add potentially too many neighbors to each clique (i.e., potentially creating
    self reinforcing cycles over single variables, that will need to be broken later)
    */
    void addNeighboursToAllCliques() ;
    
    /*!
     * Read all cliques from file with the following tab-delimited format:

     * @Cliques

     * cl0	1	0	2	1 2

     * cl1	2	0 1	1	0

     * cl2	2	0 2	1	0

     * @End

     * Where the columns indicate:

     * 1) clique name 

     * 2) number of vars in clique

     * 3) the list of vars in clique 

     * 4) number of neighbourss 

     * 5) list of neighbouring cliques indices

     */
    bool readCliques(ifstream_ptr in);

    /*  
**********************************************************
Accessors
**********************************************************
*/

    /*!
    Get the number of variables in the graph
    */
    int getNumOfVars() const;
        
    /*!
      Get the variables that participate in a clique
      @param clique is the clique index for which we want the variables
      @return a vector with 
    */
    varsVec const& getVarsVecForClique(cliqIndex clique)  const;

    /*!
    Get clique neighbors
    */
    inline cliquesVec const& cliqueNeighbors(cliqIndex clique) const;
    
    /*!
    Get the variables over which the sepset is defined between this clique and all its neighbors 
    @param clique is the clique for which we want the neighbor's sepsets
    @return a vector of sepset's in the same order as its neighbor's list.
    */
    inline separatorsVec const& cliqueNeighborSeparators(cliqIndex clique) const;

    /*!
    Get all the separators sets in this graph
    \sa cliqueNeighborSeparators
    \return a vector in which for each clique we have a vector of sep-sets in the order
    of its nighbors
    */
    inline neighborSeparatorVec const& getGraphSeparators() const;

    /*!
    Get number of cliques in  this graph
    */
    int getNumOfCliques()  const;

    /*!
    Get all cliques that ths variable is in
    \param var is the index of the variable for which we want the list of cliques
    \return a vector with the list of cliques.
    */
    inline cliquesVec const & getAllCliquesForVar(rVarIndex var) const;

    /*!
    Get the variable list for this graph
    */
    inline lbVarsList const& getVars() const;

    /*!
      Get the name of a random variable
     */
    string getNameOfVar(rVarIndex var) const;

    /*!
      Get size of largest clique (price of inference on this graph, if its triangulated)
     */
    inline int getSizeOfLargestClique();
    
    /*!
     * Returns whether running intersection is satisfied for the given variable.
     * That is, whether there is a path containing that variable between any
     * two places in which it appears in the graph.
     * @return True if RIP is satisfied for var, false otherwise.
     */
    bool runningIntersectionSatisfied(rVarIndex var) const;
    
    /*!
     * Checks whether the graph is connected or has disconnected components 
     * (semantically separate from running intersection).
     * @return True if the graph is connected, false otherwise.
     */
    bool isConnected() const;

    /*  
**********************************************************
Operators
**********************************************************
*/
    
    //operator ==
    bool operator==(lbGraphStruct const& otherGraph) ;

    
    /*  
**********************************************************
Print methods
**********************************************************
*/
    /*!
    Print graph
    \param out is the ostream to which we want to print
    */
    void printGraph(ostream& out) const;

    /*!
    Print graph in fastInf format
    \param out is the ostream to which we want to print
    */
    void printGraphToFastInfFormat(ostream& out) const;

    /*!
    Print graph in dot format (for graphical view, see http://www.graphviz.org/ )
    \param out is the ostream to which we want to print
    */
    void printToDotFile(ostream& out) const ;
    
    /*!
    Checks wether a var is in a clique
    */
    bool isVarInClique(cliqIndex clique,rVarIndex varInd) const;
    

  private:

    //----------------------------methods----------------------------
    
    // reads from file one clique
    void readOneClique(ifstream_ptr in ,string cliqName);

    //------------------------data members---------------------------

    bool _deepDest ;
    
    //the cliques list 
    varsVecVector _cliqueVec;
    
    //the assignments of random vars to cliques (the entry is the vars index and
    cliquesVecVector _varToCliqueVec;
    
    //the adjacency list of neighboring cliques
    adjListVec _cliqueNeighborsVec;

    //separators vectors for each clique
    neighborSeparatorVec _cliqueNeighborSeparatorVec;

    //vars list
    lbVarsList const & _varsList;

    //temporary
    cliquesVecVector _localNeighbors;

    //
    int _numOfCliques;
    
    // The listeners to this graph. Declared mutable for updating a
    // const graph.
    mutable graphListenersVec _usingObjects;

  };

  //-------------------INLINES---------------------------

  
  inline separatorsVec const& lbGraphStruct::cliqueNeighborSeparators(cliqIndex clique) const{
    return _cliqueNeighborSeparatorVec[clique];
  }
    
  inline cliquesVec const& lbGraphStruct::cliqueNeighbors(cliqIndex clique) const{
    return _cliqueNeighborsVec[clique];
  }

  /*  inline void lbGraphStruct::setGraphVars(lbVarsList & varsList){
    _varsList = varsList;
    //    lbDefinitions::_numOfVars=_varsList.getNumOfVars();
    }*/

  inline int lbGraphStruct::getNumOfCliques() const{
    return (_numOfCliques);
  }

  inline cliquesVec const& lbGraphStruct::getAllCliquesForVar(rVarIndex var) const{
    assert (((uint)var)<_varToCliqueVec.size());
    return _varToCliqueVec[var];
  }

  inline neighborSeparatorVec const& lbGraphStruct::getGraphSeparators() const{
    return _cliqueNeighborSeparatorVec;
  }

  /*
    inline int lbGraphStruct::getNumOfNeighborsForVar(rVarIndex var) const{
    set<rVarIndex> NeighborsSet = set<rVarIndex>();
    cliquesVec cliques = _varToCliqueVec[var]; 
    for (uint index=0;index<cliques.size();index++){
    varsVec thisCliq = _cliqueVec[cliques[index]];
    for (uint Neighbor =0;Neighbor<thisCliq.size();Neighbor++)
    if (var!=thisCliq[Neighbor]){
    //cout<<"adding "<<thisCliq[Neighbor]<<" as Neighbor of "<<var<<endl;
    NeighborsSet.insert(thisCliq[Neighbor]);
    }
    }
    return NeighborsSet.size();
    }
  */
  
  inline lbVarsList const& lbGraphStruct::getVars() const {
    return _varsList;
  };

  inline int lbGraphStruct::getSizeOfLargestClique() {
    uint maxSize=0;
    for (int i=0;i<_numOfCliques;i++) {
      if (_cliqueVec[i].size()>maxSize)
	maxSize=_cliqueVec[i].size();
    }
    return (int)maxSize;
  };
  
}
       
       
#endif

//get number of variable neighbours
//    inline int getNumOfNeighborsForVar(rVarIndex var) const;

    //
    //bool readVariables(ifstream_ptr in); 
