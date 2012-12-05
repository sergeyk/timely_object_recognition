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

#include <lbModel.h>
#include <lbModelListener.h>

using namespace lbLib ;


/**
   This Class holds the model

   General idea : to combine all the information of the model, this
   object holds the graph structure, and a connection of a measure
   for each clique.
   
   Part of the loopy belief library
   @version November 2002
   @author Ariel Jaimovich
*/

//ctor :
lbModel::lbModel()
  : _deepDest(false),
    _cardsList(NULL),
    _graph(NULL),
    _measDispatcher(NULL)
{
  Init();
}

lbModel::lbModel(lbGraphStruct const& graph,
		 lbCardsList  const& cards,
		 lbMeasureDispatcher const& disp,
                 bool deepDest) :
  _deepDest(deepDest),
  _cardsList(&cards),
  _graph(&graph),
  _measDispatcher(&disp)
{
  Init();
}

void lbModel::Init(bool resetUsing) 
{
  if ( resetUsing ) 
    _usingObjects = listenersVec();
  _mesVec = measurePtrVec(0);
  lbAssignedMeasure_ptr aptr = NULL;
  _assignedMeasures = assignedMesVec(lbDefinitions::VEC_SIZE,aptr);
  _measureUsage = cliquesVecVector(lbDefinitions::VEC_SIZE);
  _mesNames = stringVec(0);
  if ( _graph != NULL ) 
    _cliqueUsage = measIndicesVec(_graph->getNumOfCliques(),-1);
  _sharedParams = paramPtrMap();
  _idleParams = paramPtrMap();
}

void lbModel::CopyFromModel(lbModel const& oldModel) 
{
  Clean();
  Init();

  _cardsList = oldModel._cardsList;
  _graph = oldModel._graph;
  _measDispatcher = oldModel._measDispatcher;

  // GAL: should not be copied - no longer same model with same listeners
  //  copyVector(oldModel._usingObjects,_usingObjects,listenersVec);

  // measures and cliques
  _usingObjects = listenersVec();
  copyVectorWithDuplicate(oldModel._mesVec,_mesVec,measurePtrVec);
  _mesNames = oldModel._mesNames;
  copyVectorWithDuplicate(oldModel._assignedMeasures,_assignedMeasures,assignedMesVec);
  _measureUsage = oldModel._measureUsage;
  _cliqueUsage = oldModel._cliqueUsage;

  // shared
  _sharedParams = oldModel._sharedParams;
  _idleParams = oldModel._idleParams;

  paramPtrMap::iterator pit;  
  for ( pit = _sharedParams.begin() ; pit != _sharedParams.end() ; pit++ )
    pit->second = new lbParam(*pit->second);
  for ( pit = _idleParams.begin() ; pit != _idleParams.end() ; pit++ )
    pit->second = new lbParam(*pit->second);

  // counting stuff
  _countingNums = oldModel._countingNums;
  _powerNums = oldModel._powerNums;
  copyVectorWithDuplicate(oldModel._exact,_exact,assignedMesVec);
}

void lbModel::Clean() {
  uint i;

  for (i=0;i<_assignedMeasures.size();i++)
    if ( _assignedMeasures[i] != NULL ) 
      delete _assignedMeasures[i];
  paramPtrMapIter iter;
  for (iter=_sharedParams.begin();iter!=_sharedParams.end();iter++) {
    lbParam* p= (*iter).second;
    delete p;
  }
  for (iter=_idleParams.begin();iter!=_idleParams.end();iter++) {
    lbParam* p= (*iter).second;
    delete p;
  }

  for (i=0;i<_exact.size();i++) {
    delete _exact[i];
  }
  _exact.clear();
}

//copy ctor
lbModel::lbModel(lbModel const& oldModel) 
{
  CopyFromModel(oldModel);
}

lbModel& lbModel::operator=(lbModel const& oldModel) 
{
  CopyFromModel(oldModel);
  return *this;
}

//dtor
lbModel::~lbModel () {  
  Clean();
  if (_deepDest) {
    delete _cardsList ;
    delete _graph ;
    delete _measDispatcher ;
  }
}

//set a meaure for a clique
bool lbModel::setMeasureForClique(cliqIndex clique, measIndex measure) {
  lbAssignedMeasure_ptr temp(new lbAssignedMeasure(_mesVec[measure],_graph->getVarsVecForClique(clique)));
  if (_cliqueUsage.size() <= (uint)clique) {
    _cliqueUsage.resize(_graph->getNumOfCliques(),-1);
  }
  if (_assignedMeasures.size() <= (uint)clique) {
    _assignedMeasures.resize(_graph->getNumOfCliques());
  }
  if (_measureUsage.size() <= (uint)measure) {
    _measureUsage.resize(_mesVec.size());
  }
  _assignedMeasures[clique] = temp;
  _measureUsage[measure].push_back(clique);
  _cliqueUsage[clique]=measure;

  updateListeners(measIndicesVec(1, measure));

  //checking validity of assignment:
  varsVec const& cvars = _graph->getVarsVecForClique(clique);
  cardVec const & measCard =_mesVec[measure]->getCards();
  for (uint ind = 0 ; ind < cvars.size() ; ind++) {
    int tmpCard = _cardsList->getCardForVar(cvars[ind]);
    if (tmpCard!=measCard[ind]){
      cerr<<"Cardinality doesn't match! tmpCard="<<tmpCard<<" measCard="<<measCard[ind]<<endl;
      cerr<<"clique number: "<<clique<<" measure number: "<<measure<<endl;
      assert(false);
    }
  }
  return true;
}

//sets one measure for a group of cliques
bool lbModel::setMeasureForSetOfCliques(cliquesVec const& cliqueIndices, measIndex measure) {
  for (constCliquesVecIter iter = cliqueIndices.begin();
       iter < cliqueIndices.end();
       iter++) {
    assert (*iter<_graph->getNumOfCliques());
    setMeasureForClique(*iter,measure);
  }
  return true;
}

/*
lbAssignedMeasure_ptr lbModel::duplicate(lbAssignedMeasure const& original) {
  lbMeasure_ptr newMeasure=original.duplicate();
  addMeasure(newMeasure); //ignoring the incoming index
  return new lbAssignedMeasure(*newMeasure,original.getVars());
  
}
*/



//adds measure to the "measure bank", the responsability for this measure
// is now in the hands of the model.
measIndex lbModel::addMeasure(lbMeasure_Sptr measurePtr,string mesName) {
  int newSize=(int)_mesVec.size()+1;
  measIndex index=newSize-1;
  _mesVec.resize(newSize);
  _mesVec[index]=measurePtr;
  _mesNames.resize(newSize);
  _mesNames[index]=mesName;
  return index;
}

void lbModel::replaceMeasure(lbMeasure_Sptr measurePtr,measIndex index,string mesName,
			     bool notifyListeners)
{
  assert((unsigned)index < _mesVec.size());
  _mesVec[index]=measurePtr;
  _mesNames[index]=mesName;
  for ( uint c=0; c<_measureUsage[index].size() ; c++ ) 
    _assignedMeasures[_measureUsage[index][c]]->replaceMeasure(measurePtr);
  
  if (notifyListeners) {// notify listeners
    measIndicesVec measures(1,index);
    updateListeners(measures);
  }
}

//update inference object that is using this model
int lbModel::addListener(lbModelListener& listener) {
  _usingObjects.push_back(&listener);
  return _usingObjects.size()-1;
}

void lbModel::removeListener(int index) {
  int i=0;
  for (listenersVecIter iter = _usingObjects.begin() ;
       iter != _usingObjects.end() ;
       iter++) {
    if (index==i){
      _usingObjects.erase(iter);
      break;
    }
    i++;
  }
}
  

bool lbModel::readMeasures(ifstream_ptr in, bool readLogValues) {

  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
  (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
  string str(buffer.get());
  string mesName;
  //  measIndex ind=1;

  while(!(str==lbDefinitions::END_STR)) {
    if ((*in).peek()!='@') {
      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE,lbDefinitions::DELIM);
      str = string(buffer.get());
      istringstream iss(str);
      iss >> str;
      mesName = str;
      //      cerr<<"Reading measure : "<<mesName<<endl;
      //read Measure properties
      
      lbMeasure_Sptr measPtr = _measDispatcher->getNewMeasure();
      
      measPtr->readOneMeasure(in,_sharedParams,_idleParams,readLogValues);
      
      addMeasure(measPtr,mesName);
            
    }
    else {
      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
      str = string(buffer.get());
      istringstream iss(str);
      iss >> str;
    }
  }
  //  printModelToFastInfFormat(cerr);
  //  delete[] buffer;
  return true;
}
  

bool lbModel::assignMeasureForCliques(ifstream_ptr in) {

  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
  (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
  string str(buffer.get());
  measIndex mes ;
  cliqIndex cliq ;
  while(!(str==lbDefinitions::END_STR)) {
    (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
    str = string(buffer.get());
    istringstream iss(str);
    iss >> str;
    if ((str[0]!='@')) {
      cliq = atol(str.c_str());
      int tmp;
      iss >> tmp;
      mes=tmp;
      //cout<<"before set: "<<mes<<" to cliq "<<cliq<<endl; 
      setMeasureForClique(cliq,mes);
    }
  }
  //  cout<<"end model$%$%$%$%$%$%$%%"<<endl;
  //  delete[] buffer;
  return true;
}

void lbModel::printModel(ostream& out) const{
  out << "printing model" <<endl;
  if (_mesVec.size()!=0) {
    out << "measures are : "<<endl;
    for (int mes =  0 ; mes < (measIndex)_mesVec.size() ; mes++) {
      out << "mes name: "<<_mesNames[mes]<< " index: "<<mes<<endl;
      out << "mesure : ";
      //      _mesVec[mes]->normalize();
      _mesVec[mes]->print(out);
      out<<endl;
      out << "cliques using this measure: ";
      for (constCliquesVecIter iter = _measureUsage[mes].begin();
	   iter < _measureUsage[mes].end();
	   iter++) {
	out << *iter <<" , ";
      }
      out <<endl;
    }
    if (_assignedMeasures.size()!=0){
      out <<" printing cliques and their measures :" <<endl;
      for (int cliq = 0 ; cliq <= _graph->getNumOfCliques()-1 ; cliq++) {
	out << "cliq "<<cliq<<" mes ";
	_assignedMeasures[cliq]->print(out);
	out<<endl;
      }
    }
    else {
      out<<"NO ASSIGNED MEASURES"<<endl;
    }
  }
  else {
    out<<"NO MEASURES IN LIST"<<endl;
  }
}



void lbModel::printModelToFastInfFormat(ostream& out,bool printNormalized,int prec,
					bool printLogValues) const{
  //
  printVarsAndCardToFile(out);
  getGraph().printGraphToFastInfFormat(out);

  //  out<<_mesVec.size()<<" measures"<<endl;
  out<<"@Measures"<<endl;
  if (_mesVec.size()!=0) {
    for (int mes =  0 ; mes < (measIndex)_mesVec.size() ; mes++) {
     out << _mesNames[mes]<<"\t";
     lbMeasure_Sptr printMes = _mesVec[mes]->duplicate();
     if ( printNormalized ) {
       if (printMes->isDirected())
	 printMes->normalizeDirected();
       else 
	 printMes->normalize();
     }
    
     printMes->printToFastInfFormat(out,printNormalized,prec,printLogValues);
    }
  }
  out<<"@End\n\n"<<endl;
  out << "@CliqueToMeasure\n";
  uint i;
  for (i=0;i < _cliqueUsage.size() ;i++) {
    out <<i<<"\t"<<_cliqueUsage[i]<<endl;
  }
  out <<"@End"<<"\n\n"<<endl;
  out <<"@DirectedMeasures\n";
  for (i=0;i < _mesVec.size() ;i++) {
    if (_mesVec[i]->isDirected())
      out <<i<<endl;
  }
  out <<"@End"<<"\n\n"<<endl;
}

void lbModel::updateLogParamsFromVector(probType const* vec,bool undirected,set<measIndex> const& measSet) {
    int index =0 ;
    int numOfMeasures=_mesVec.size();
    measIndicesVec measures = measIndicesVec();
    bool changed=false;
    for (measIndex meas=0;meas<numOfMeasures;meas++){
        if ( measSet.size()==0 || measSet.find(meas)!=measSet.end() )
            if ((!undirected) || (!(_mesVec[meas]->isDirected())) ) {
                changed=false;
                int tmp =_mesVec[meas]->setLogParams(vec,index,changed);
                if (changed) {
                    for ( uint c=0; c<_measureUsage[meas].size() ; c++ ) 
                        _assignedMeasures[_measureUsage[meas][c]]->replaceMeasure(_mesVec[meas]);
                    measures.push_back(meas);
                }
                index+=tmp;
            }
    }
  
    if (measures.size()>0){
        updateListeners(measures);
    }
}

void lbModel::addSharedParamsToMeasure(measIndex meas,
				       assignmentPtrVec const & assignVec,
				       varsVec const& vars){
  _mesVec[meas]->addSharedParams(assignVec,vars);
  for ( uint c=0; c<_measureUsage[meas].size() ; c++ ) 
    _assignedMeasures[_measureUsage[meas][c]]->replaceMeasure(_mesVec[meas]);
}


void lbModel::makeMeasureDirected(measIndex meas) {
  //cerr << "Making measure " << meas << " directed\n";
  _mesVec[meas]->makeDirected();
  _mesVec[meas]->normalizeDirected();
  for ( uint c=0; c<_measureUsage[meas].size() ; c++ ) 
    _assignedMeasures[_measureUsage[meas][c]]->replaceMeasure(_mesVec[meas]);
}

bool lbModel::readSharedAndIdleParams(ifstream_ptr in,lbMeasureParamType type) {
  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
  (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
  string str(buffer.get());
  string name,description;
  probType val;
  while(!(str==lbDefinitions::END_STR)) {
    if ((*in).peek()!='@') {
      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE,lbDefinitions::DELIM);
      name = string(buffer.get());
      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE,lbDefinitions::DELIM);
      str = string(buffer.get());
      istringstream iss(str);     
      iss >> val;  
      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
      description = string(buffer.get());
      lbParam* pp = new lbParam(name,val,description);
      if (type==MP_IDLE)
	_idleParams[name]=pp;
      else if (type==MP_SHARED)
    	_sharedParams[name]=pp;
    }
    else {
      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
      str = string(buffer.get());
    }
  }
  return true;
}

bool lbModel::readDirectedMeasuresList(ifstream_ptr in) {
  char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
  (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
  string str(buffer.get());
  int meas;
  while(!(str==lbDefinitions::END_STR)) {
    if ((*in).peek()!='@') {

      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
      str = string(buffer.get());
      istringstream iss(str);

      //cerr << str;     
      iss >> meas;  
      makeMeasureDirected(meas);
    }
    else {
      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
      str = string(buffer.get());
    }
  }
  return true;
}

void lbModel::updateListeners(measIndicesVec vec){
  if (vec.size() != 0) {
    for (uint i =0; i<_usingObjects.size();i++){
      //cerr<<"Updating listener num : "<<i<<endl;
      _usingObjects[i]->factorsUpdated(vec);
    }
  }
}


void lbModel::makeSparseHack(double strength) {
    for (uint i = 0; i < _mesVec.size(); i++) {
      if (_mesVec[i]->getCards().size() > 1) {
	_mesVec[i]->makeSparse(strength);
	_mesVec[i]->setSparsityThresh(0.0);
	assert(_mesVec[i]->totalWeight() != 0);
      }
    }
  }

long int lbModel::getParamNum(bool undirectedOnly,set<measIndex> const& measSet) const {
    long int res=0;
    for (measIndex meas=0;meas<getNumOfMeasures();meas++) {
        if ( measSet.size()==0 || measSet.find(meas)!=measSet.end() )
            if ( (!undirectedOnly) || (!(_mesVec[meas]->isDirected())) )
                res+=_mesVec[meas]->getParamNum();
    }
    return res;
}

long int lbModel::getSize(bool undirectedOnly,set<measIndex> const& measSet) const {
    long int res=0;
    for (measIndex meas=0;meas<getNumOfMeasures();meas++) 
        if ( measSet.size()==0 || measSet.find(meas)!=measSet.end() )
            if ( (!undirectedOnly) || (!(_mesVec[meas]->isDirected())) )
                res += _mesVec[meas]->getSize();
    return res;
}

probType* lbModel::getLogParamVector(int& length,bool undirectedOnly,set<measIndex> const& measSet) const{

    length = getParamNum(undirectedOnly,measSet);
  
    probType* res = new probType[length];
    for ( int l=0 ; l<length ; l++ )
        res[l] = 0;

    int index =0;
    for (measIndex meas=0;meas<getNumOfMeasures();meas++) {
        if ( measSet.size()==0 || measSet.find(meas)!=measSet.end() )
            if ( (!undirectedOnly) || (!(_mesVec[meas]->isDirected())) ) {
                int tmp = _mesVec[meas]->extractLogParamsAddToVector(res,index);
                index+=tmp;
            }
    }
    return res;
}

probType* lbModel::getLogValueVector(int& length,bool undirectedOnly,set<measIndex> const& measSet) const{

    length = getSize(undirectedOnly,measSet);
  
    probType* res = new probType[length];
    for ( int l=0 ; l<length ; l++ )
        res[l] = 0;

    int index =0;
    for (measIndex meas=0;meas<getNumOfMeasures();meas++) {
        if ( measSet.size()==0 || measSet.find(meas)!=measSet.end() )
            if ( (!undirectedOnly) || (!(_mesVec[meas]->isDirected())) ) {
                int tmp = _mesVec[meas]->extractValuesAddToVector(res,index,true);
                index+=tmp;
            }
    }
    return res;
}

void lbModel::makeVarValidCountingNums() {
  
  assert(_countingNums.size() == (uint) getGraph().getNumOfCliques());

  int numOfVars = getGraph().getNumOfVars() ;

  vector<double> varsTotal (numOfVars, 0.0) ;
  intVec varsInd (numOfVars, -1) ;

  for (cliqIndex cliq=0; cliq<getGraph().getNumOfCliques(); cliq++) {
    double curCountNum = getCountingNum (cliq) ;
    varsVec cliqVars = getGraph().getVarsVecForClique (cliq);
    
    if (cliqVars.size() == 1) { // variables
      //cerr << "[DEBUG] Univar cliq " << cliq << endl ;
      varsInd[cliqVars[0]] = (int)cliq ;
      // do we want to do something with univars?
    }
    else {  // larger factor
      for (uint i=0 ; i<cliqVars.size() ; i++) {
        varsTotal[cliqVars[i]] += curCountNum ;
      }
    }
  }

  for (uint i=0 ; i<varsTotal.size() ; ++i) {
    setCountingNum (varsInd[i], 1-varsTotal[i]) ;
  }
}
