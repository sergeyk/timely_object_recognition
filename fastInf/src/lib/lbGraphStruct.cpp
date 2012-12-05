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

#include <queue>
#include <lbGraphStruct.h>
#include <lbGraphListener.h>

using namespace lbLib ;
/**
   This Class holds a Graph structure.

   General Idea :
   This class holds the graph structure.
   the structure is implied by the clique list, and 

   the connections between cliques.
   cliques do not really exist as Objects, but have indices.
   each index has an entry in a vector with its random vars list,
   and an entry in a vector with its neighboring cliques list.
   
   Part of the loopy belief library
   @version November 2002
   @author Ariel Jaimovich
*/

// default contsructor 
lbGraphStruct::lbGraphStruct(lbVarsList const & varsList, bool deepDest):
  _deepDest(deepDest),
  _varsList(varsList)
{
  //  lbDefinitions::_numOfVars=varsList.getNumOfVars();
  _cliqueVec = varsVecVector();
  _cliqueNeighborSeparatorVec = neighborSeparatorVec();
  _varToCliqueVec = cliquesVecVector(varsList.getNumOfVars());
  _cliqueNeighborsVec = adjListVec();
  _numOfCliques=0;
   _localNeighbors = cliquesVecVector();
}

// copy Ctor
lbGraphStruct::lbGraphStruct(lbGraphStruct const& oldGraph):
  _cliqueVec(oldGraph._cliqueVec),
  _varToCliqueVec(oldGraph._varToCliqueVec),
  _cliqueNeighborsVec(oldGraph._cliqueNeighborsVec),
  _cliqueNeighborSeparatorVec(oldGraph._cliqueNeighborSeparatorVec) ,
  _varsList(oldGraph._varsList),
  _numOfCliques(oldGraph._numOfCliques)
{
  cerr<<"copy ctor"<<endl;
}

//dtor
lbGraphStruct::~lbGraphStruct() {
  if (_deepDest) {
    delete &_varsList ;
  }
}

int lbGraphStruct::getNumOfVars() const {
  return _varsList.getNumOfVars();
};

varsVec const& lbGraphStruct::getVarsVecForClique(cliqIndex clique) const {  
  return  _cliqueVec[clique];
}


void lbGraphStruct::addListener(lbGraphListener & listener) const {
  _usingObjects.push_back(&listener);
}

void lbGraphStruct::removeListener(lbGraphListener_ptr listener) const {
  for (graphListenersVec::iterator iter = _usingObjects.begin();
       iter != _usingObjects.end(); ++iter) {
    if (*iter == listener) {
      _usingObjects.erase(iter);
      return;
    }
  }
}


void lbGraphStruct::updateListeners() const {
   for (uint i = 0; i < _usingObjects.size(); ++i) {
     _usingObjects[i]->graphUpdated();
  }
}

//add a clique (return its index)
cliqIndex lbGraphStruct::addClique (const varsVec& varsList,string cliqName){
  cliqIndex ind = (cliqIndex)_numOfCliques;
  _numOfCliques++;
  _cliqueVec.resize(_numOfCliques);
  _cliqueNeighborSeparatorVec.resize(_numOfCliques);
  _cliqueNeighborsVec.resize(_numOfCliques);

  _cliqueVec[ind] = varsList;
  for (rVarIndex var=0;var<(rVarIndex)varsList.size();var++) {
    if (((rVarIndex)_varToCliqueVec.size())<=varsList[var]) {
      rVarIndex tmp=varsList[var];
      tmp++;
      _varToCliqueVec.resize(tmp);
    }
    //    cerr<<"Adding c "<<ind<<" as having var "<<varsList[var]<<endl;
    //printVector(_varToCliqueVec[varsList[var]],cerr);
    _varToCliqueVec[varsList[var]].push_back(ind);
    //cerr<<"After :";
    //printVector(_varToCliqueVec[varsList[var]],cerr);
  }
  return ind;
} 
  
//add Random variable to clique
bool lbGraphStruct::addRandomVarToClique (cliqIndex clique, rVarIndex var) {
  _cliqueVec[clique].push_back(var);
  if (((rVarIndex)_varToCliqueVec.size())<=var)
    _varToCliqueVec.resize(var++);
  _varToCliqueVec[var].push_back(clique);
  //cout<<"pushing 2 "<<clique<<" as clique of var : "<<var<<endl;
  return true;
}
    
//remove Random Variable from clique
bool lbGraphStruct::removeRandomVarFromClique (cliqIndex clique, rVarIndex var) {
  bool found = false;
  cliquesVecIter cliqIter;
  for (cliqIter = _varToCliqueVec[var].begin();
       cliqIter<_varToCliqueVec[var].end();
       cliqIter++)
    {
      if(*cliqIter == clique){
	*cliqIter = -1;
	_varToCliqueVec[var].erase(cliqIter);
	found = true;
      }
    }
  
  
  if (!found)
    return false;
  else {
    for (varsVecIter iter = _cliqueVec[clique].begin(); 
	 iter != _cliqueVec[clique].end();
	 iter++){
      
      if (*iter == var) {
	_cliqueVec[clique].erase(iter);
	return true;
      }
    } 
  }  
  return false;
}



//set clique Neighborbours
bool lbGraphStruct::setCliqueNeighbors (cliqIndex clique, cliquesVec& cliqueNeighbors) {

  //_cliqueNeighborsVec[clique] = cliqueNeighbors;
  for (uint index=0;index<cliqueNeighbors.size();index++){
    addCliqueNeighbor(clique,cliqueNeighbors[index]);
  }
  return true;
}

bool lbGraphStruct::addCliqueNeighbor(cliqIndex clique, cliqIndex Neighbor) {
  
  varsVec vec = varsVec(); 
  //  cerr<<"WE HAVE :"<<_cliqueVec.size()<<" cliques in the graph, clique is "<<clique<<" nei is "<<Neighbor<<endl;
  for (rVarIndex ind1=0;ind1<(rVarIndex)_cliqueVec[clique].size();ind1++){
    for (rVarIndex ind2=0;ind2<(rVarIndex)_cliqueVec[Neighbor].size();ind2++){
      if (_cliqueVec[clique][ind1]==_cliqueVec[Neighbor][ind2])
	vec.push_back(_cliqueVec[clique][ind1]);
    }
  }
  if (vec.size()==0)
    cerr<<"WARNING : sep between cliq "<<clique<<" and clique "<<Neighbor<<" is empty !"<<endl;
  return addCliqueNeighbor(clique,Neighbor,vec);
}

//add Neighbor
bool lbGraphStruct::addCliqueNeighbor(cliqIndex clique, cliqIndex neighbor, varsVec separator) {
  //checking input is legal 
  if(clique>=((cliqIndex)_cliqueNeighborsVec.size())){
    cliqIndex tmp=clique;
    _cliqueNeighborsVec.resize(tmp++);
   _cliqueNeighborSeparatorVec.resize(tmp);
  }

  if (clique>_numOfCliques || neighbor>_numOfCliques){
    cerr <<"In adding to clique "<<clique<<" neighbor "<<neighbor<<
      ", clique index is not legal since we have "<<_numOfCliques<<"cliques"<<endl;
    return false;
  }
  for (uint ind=0;ind<separator.size();ind++){
    if (!(isVarInClique(clique,separator[ind])) ||
	!(isVarInClique(neighbor,separator[ind]))){
      cerr <<"In adding clique neighbor, separator var index "<<separator[ind]<<" is not legal"<<endl;
      return false;
    }
      
  }
  bool found = false;
  //add first dir
  for(uint NeighborInd=0;NeighborInd<_cliqueNeighborsVec[clique].size();NeighborInd++)
    if (_cliqueNeighborsVec[clique][NeighborInd]==neighbor)
      found=true;
  if (!found) {
    _cliqueNeighborsVec[clique].push_back(neighbor);
    _cliqueNeighborSeparatorVec[clique].push_back(separator);
  }
  //add second dir

  //if vec does not exist :
  if (neighbor>=((cliqIndex)_cliqueNeighborsVec.size())){
    cliqIndex tmp=neighbor;
    tmp++;
    _cliqueNeighborsVec.resize(tmp);
    _cliqueNeighborSeparatorVec.resize(tmp);
    _cliqueNeighborsVec[neighbor].push_back(clique);
    _cliqueNeighborSeparatorVec[neighbor].push_back(separator);
    return true;
  }
  
  for(uint neighborInd=0;neighborInd<_cliqueNeighborsVec[neighbor].size();neighborInd++)
    if (_cliqueNeighborsVec[neighbor][neighborInd]==clique)
      found=true;
  if (!found){
    _cliqueNeighborsVec[neighbor].push_back(clique);
    _cliqueNeighborSeparatorVec[neighbor].push_back(separator);
  }
  return true;
}

//remove neighbor
bool lbGraphStruct::removeCliqueNeighbor(cliqIndex clique, cliqIndex neighbor) {
  for (cliquesVecIter iter = _cliqueNeighborsVec[clique].begin();
       iter<_cliqueNeighborsVec[clique].end();
       iter++){
    if (*iter == neighbor) {
      _cliqueNeighborsVec[clique].erase(iter);
      return true;
    }
  }

  //neighbor was not found :
  return false;
}

     
    
// checks wether a var is in a clique
bool lbGraphStruct::isVarInClique(cliqIndex clique,rVarIndex var) const{
  for (constCliquesVecIter cliqIter = _varToCliqueVec[var].begin();
       cliqIter<_varToCliqueVec[var].end();
       cliqIter++) 
    if(*cliqIter == clique)
      return true;
  return false;
 
}
  
//operator ==
bool lbGraphStruct::operator==(lbGraphStruct const& otherGraph) {
  if (_cliqueNeighborsVec==otherGraph._cliqueNeighborsVec)
    if (_varToCliqueVec==otherGraph._varToCliqueVec)
      if (_cliqueVec==otherGraph._cliqueVec)
	return true;
  //graphs are different
  return false;
}





//void lbGraphStruct::setGraphCards(cardsList const& cards){
//  _cardsList = _cards;
//}

bool lbGraphStruct::readCliques(ifstream_ptr in) {
  
  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
  (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
  string str(buffer.get());
  //cliqIndex ind =1;
  string cliqName;
  
  while(!(str==lbDefinitions::END_STR)) {
    if ((*in).peek()!='@') {
      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE,lbDefinitions::DELIM);
      str = string(buffer.get());
      istringstream iss(str);
      iss >> str;
      
      //first string is measure name
      cliqName = str;
            

      //read clique properties
      readOneClique(in,cliqName);
    }
    else {
      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
      str = string(buffer.get());
      istringstream iss(str);
      iss >> str; 
    }
  }
  
  //TEMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //adding cliqeu neighbours
  for (cliqIndex cliq=0;(uint)cliq<_localNeighbors.size();cliq++){
    if (_localNeighbors[cliq].size()>0)
      setCliqueNeighbors(cliq,_localNeighbors[cliq]);
    else {
      cliqIndex tmp=cliq;      
      _cliqueNeighborsVec.resize(tmp++);
      _cliqueNeighborSeparatorVec.resize(tmp);
      _cliqueNeighborsVec[cliq]=cliquesVec();
    }
  }
  
  //addNeighboursToAllCliques();

  //  printGraph(cerr);
  //  delete[] buffer;
  return true;
  
}


void lbGraphStruct::addNeighboursToAllCliques() {
  for (cliqIndex cliqOne=0;cliqOne<(_numOfCliques-1);cliqOne++) {
    for(cliqIndex cliqTwo=((int)cliqOne+1);cliqTwo<_numOfCliques;cliqTwo++){
      //find all cliques that have same var in their scope and make them neighbours:
      varsVec& firstVars = _cliqueVec[cliqOne];
      varsVec& secondVars = _cliqueVec[cliqTwo];
      bool found=false;
      for (uint varOne=0;(varOne<firstVars.size() && !found);varOne++) {
		for (uint varTwo=0;(varTwo<secondVars.size() && !found);varTwo++) {
			if (firstVars[varOne]==secondVars[varTwo]){
				found=true;
				addCliqueNeighbor(cliqOne,cliqTwo);
	    //	    setCliqueNeighbors(cliqTwo,cliqOne);
	  }
	}
      }
    }
  }
}

void lbGraphStruct::readOneClique(ifstream_ptr in , string cliqName){
  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
  
  //read num of vars in clique
  (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE,lbDefinitions::DELIM);
  string str = string(buffer.get());
  istringstream iss(str);
  int numOfVarsInCliq;

  iss >> numOfVarsInCliq;
  
  (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE,lbDefinitions::DELIM);
  str = string(buffer.get());
  
  istringstream iss2(str);
  varsVec cliqueVars =  varsVec(numOfVarsInCliq);
  vecIndex var;
  
  //read clique vars
  for (int i=0;i<numOfVarsInCliq;i++) {
    int tmp;
    iss2>>tmp;
    var=tmp;
    cliqueVars[i]=var;
  }
  cliqIndex cliq=addClique(cliqueVars,cliqName);
  //read num of neighbors
  (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE,lbDefinitions::DELIM);
  str = string(buffer.get());
  istringstream iss3(str);
  int numOfNeighbors;
  iss3 >> numOfNeighbors;
  
  //_cliqueNeighsVec[ind] = cliquesVec(numOfNeighs);
  cliquesVec Neighbors= cliquesVec(numOfNeighbors);
  if (numOfNeighbors>0){
    //read neighs
    (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
    str = string(buffer.get());
    istringstream iss4(str);
    cliqIndex Neighbor;

    //read clique vars
    for (int i=0;i<numOfNeighbors;i++) {
      int tmp;
      iss4>>tmp;
      Neighbor = tmp;
      Neighbors[i]=Neighbor;
    }
    //setCliqueNeighs(cliq,neighs);
    if(_localNeighbors.size()<=(uint)cliq){
      _localNeighbors.resize((int)cliq+1);
      _localNeighbors[cliq]=cliquesVec(Neighbors);
    }
    else{
      _localNeighbors[cliq]=cliquesVec(Neighbors);
    }
  }  
  else {
    _localNeighbors.resize((int)cliq+1);
    _localNeighbors[cliq]=cliquesVec();
    (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
  }
  // delete[] buffer;
}


void lbGraphStruct::printGraph(ostream& out) const{
  out << "Printing graph "<<endl;
  if (_numOfCliques==0){
    out<<"GRAPH IS EMPTY"<<endl;
    return;
  }
  out << "Cliques are " <<endl;
  for (int cliq = 0 ;cliq < _numOfCliques ; cliq++) {
    out<<"Cliq num "<<cliq;
    out<<", vars are :";
    for (constVarsVecIter iter = _cliqueVec[cliq].begin();
	 iter<_cliqueVec[cliq].end();
	 iter++) {
      out<<*iter<<" ";
    }
    out<<endl;
    out<<"Neighs list: ";
    for (constCliquesVecIter citer = _cliqueNeighborsVec[cliq].begin();
	 citer<_cliqueNeighborsVec[cliq].end();
	 citer++) {
      out<<*citer<<" ";
    }
    out<<endl;
    
  }
}


void lbGraphStruct::printGraphToFastInfFormat(ostream& out) const{

  out << "@Cliques"<<endl;
  for (int cliq = 0 ;cliq < _numOfCliques ; cliq++) {
    out<<"cliq"<<cliq<<"\t";
    out<<_cliqueVec[cliq].size()<<"\t";
    for (constVarsVecIter iter = _cliqueVec[cliq].begin();
	 iter<_cliqueVec[cliq].end();
	 iter++) {
      out<<*iter<<" ";
    }
    out<<"\t";
    out<<_cliqueNeighborsVec[cliq].size()<<"\t";
    for (constCliquesVecIter citer = _cliqueNeighborsVec[cliq].begin();
	 citer<_cliqueNeighborsVec[cliq].end();
	 citer++) {
      out<<*citer<<" ";
    }
    out<<endl;

  }
  out<<"@End"<<"\n\n"<<endl;
}

void lbGraphStruct::printToDotFile(ostream& out) const {

  out << "graph G {\n";
  out <<" nodesep = 0; \n ";
  out << "size = \"40,60\" ; \n"; 
  out << "ratio=compress; \n";
  out << "center=true ; \n "; 
  out << "fontname=Helvetica;\n"; 
  out << "orientation=portrait;\n";
 
  out<<"node[ shape = ellipse , color=blue , fontcolor = red ] ;\n";

  int cliq;
  for (cliq =0 ; cliq<_numOfCliques ; cliq++) {
    string s = ("cliq"); 
    char c[15];
    sprintf(c,"%d",cliq);
    s= s+string(c);
    //    out <<s<< "[ label = \" cliq <<cliq<<": ";
    out <<s<< "[ label = \"" << cliq << "\" ,width=0.3,height=0.3]\n";
    varsVec cliqVars = _cliqueVec[cliq];
    /*
    for (int i =0; i<(int)cliqVars.size() ; i++) {
      out <<cliqVars[i];//_varsList.getNameOfVar(cliqVars[i]);
      if (i<(int)cliqVars.size()-1)
	;//	out<<", ";
    }
    ;//    out<<" \" ] ;"<<endl;
    */
  }

  for (cliq =0 ; cliq<(int)_numOfCliques ; cliq++) {
    string s1 = ("cliq"); 
    char c[15];
    sprintf(c,"%d",cliq);
    s1= s1+string(c);
    cliquesVec neighs = _cliqueNeighborsVec[cliq];
    for (int i =0; i<(int)neighs.size() ; i++) {
      int j = (int) neighs[i];
      if (cliq<j) {
	string s2 = ("cliq"); 
	char c1[15];
	sprintf(c1,"%d",j);
	s2= s2+string(c1);
	out <<s1<< "--" <<s2 ;
	out <<"[ label = \" ";
	varsVec sepVec = _cliqueNeighborSeparatorVec[cliq][i];
	for (int k=0;k<(int)sepVec.size();k++) {
	  out<<sepVec[k];
	  if (k<(int)sepVec.size()-1)
	    out<<" ";
	}
	out <<"\" ] ;"<<endl;	
      }
    }
  }
  out<<"}"<<endl;
}


string lbGraphStruct::getNameOfVar(rVarIndex var) const {
  return _varsList.getNameOfVar(var);
  
}

bool lbGraphStruct::runningIntersectionSatisfied (rVarIndex var) const {
  const cliquesVec& cvec(getAllCliquesForVar(var));
  if (cvec.size() == 0) return true;
  uint i, j;
  queue<cliqIndex> q;
  boolVec visitedVec(_numOfCliques,false);
  //  for (i = 0; i < cvec.size(); ++i) {
  // visitedVec[cvec[i]] = false;
  //}
  q.push(cvec[0]);
  visitedVec[cvec[0]] = true;
  while(!q.empty()) {
    const cliquesVec& neighbors(cliqueNeighbors(q.front()));
    q.pop();
    for (j = 0; j < neighbors.size(); ++j) {
      const varsVec& vvec = getVarsVecForClique(neighbors[j]);
      if (find(vvec.begin(), vvec.end(), var) != vvec.end()
	  && !visitedVec[neighbors[j]]) {
	visitedVec[neighbors[j]] = true;
	q.push(neighbors[j]);
      }
    }
  }
  for (i = 0; i < cvec.size(); ++i) {
    if (!visitedVec[cvec[i]]) {
      return false;
    }
  }
  return true;
}

bool lbGraphStruct::isConnected () const {
  queue<cliqIndex> q;
  uint j;
  boolVec visitedVec(_numOfCliques,false) ;
  //fill(visitedVec.begin(), visitedVec.end(), false);
  //TODO: What do we do in the case of erased cliques?
  q.push(0);
  visitedVec[0] = true;
  while(!q.empty()) {
    const cliquesVec& neighbors(cliqueNeighbors(q.front()));
    q.pop();
    for (j = 0; j < neighbors.size(); ++j) {
      if (!visitedVec[neighbors[j]]) {
	visitedVec[neighbors[j]] = true;
	q.push(neighbors[j]);
      }
    }
  }
  // If we cannot find false in the visited vec, we know it's connected.
  return (find(visitedVec.begin(), visitedVec.end(), false)
	  == visitedVec.end());
}
     
