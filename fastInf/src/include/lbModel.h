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

#ifndef _Loopy__Model
#define _Loopy__Model

#include <lbDefinitions.h>
#include <lbGraphStruct.h>
#include <lbAssignedMeasure.h>
#include <lbMeasureDispatcher.h>
#include <lbGraphListener.h>

namespace lbLib {


  class lbModel;
  typedef lbModel* lbModel_ptr;
  
  /*!
   This Class holds the model

   General idea : to combine all the information of the model, this
   object holds the graph structure, and a connection of a measure
   for each clique.
   
   Part of the loopy belief library
   \version July 2009
   \author Ariel Jaimovich
*/
  class lbModel : public lbGraphListener {

  public:

    /*
**********************************************************
Ctors and Dtors :
**********************************************************
*/
    
    /*!
    Basic constructor
    \param graph is the grpah over which the model is defined
    \param cards is the set of variables and their cardinalities
    \param disp is the factory dispatching measures
    */
    lbModel(lbGraphStruct const& graph,lbCardsList const& cards,lbMeasureDispatcher const& disp, bool deepDest = false) ;
    
    /*!
      Copy constructor
    */
    explicit lbModel(lbModel const& oldModel);

    /*!
      Operator =
     */
    lbModel& operator=(lbModel const& oldModel);

    /*!
      Distructor
    */
    ~lbModel() ;
    
    /*  
**********************************************************
Mutators
**********************************************************
*/
    /*!
      Set all measures with a minimum fraction of zero entries to be sparse
    */
    void makeSparseHack(double strength);
    
    /*!
    Combine between a clique and a template measure
    This function creates an lbAssignedMeasure with this measure
    and the variables in this clique
    \param clique the index of the clique we want to assign
    \param measure the index of the measure we want to assign
    \return true if the operation was completed succesfully
    \sa setMeasureForSetOfCliques
    \sa setMeasureForAllCliques

    */
    bool setMeasureForClique(cliqIndex clique, measIndex measure);

    /*!
    Sets one measure for a group of cliques
    \sa setMeasureForClique
    \sa setMeasureForAllCliques
    */
    bool setMeasureForSetOfCliques(cliquesVec const& cliqueIndices, measIndex measure);

    /*!
    Sets a measure for all cliques
    \sa setMeasureForClique
    \sa setMeasureForSetOfCliques
    */
    bool setMeasureForAllCliques(measIndex measure);

    /*!
      Adds measure to the "measure bank"
    */
    measIndex addMeasure(lbMeasure_Sptr measurePtr,string mesName = string("noName"));
    
    /*!
    Replaces a measure in the measure bank
    */
    void replaceMeasure(lbMeasure_Sptr measurePtr,measIndex index,
			string mesName = string("noName"), bool notifyListeners = true);

    /*!
      Add a listener to this model. This listeners are classes
      that inherit from lbModelListener (e.g., lbSuffStat, lbInferenceOBject)
      and that need to notified in case a model has changed (e.g., during parameter estimation)
      \return the index of this listener (the index is needed to further operations- do not lose it)
      \sa removeListener
    */
    int addListener(lbModelListener & listener);    

    /*!
    Remove a listener form listener list
    \param index is the index of th listener that needs to be removed
    */
    void removeListener(int index);

    /*!
      Read template measures from a tab delimited file with the following format

      @Measures

      mes0	1	2	0.4 0.6

      mes1	2	2 2	0.5 0.5 0.9 0.1
      
      mes2	2	2 2	0.3 0.7 0.6 0.4
      
      @End
      
      Where the columns are:
      1) MeasureName 
      2) Number of vars in measure 
      3) The vars cardinality
      4) Table of potentials, ordered when the assignment advances like a binary number i.e : 000 001 010 011 100 ...


    */

    bool readMeasures(ifstream_ptr in, bool readLogValues = false);
    /*!
      Reads a set of idle and shared params (e.g., for use in learning, see lbMeasure)
      from a tab delimited file with the following format:
      @IdleParams
      ip1	100	idle param for trial
      ip2	150	idle param for trial 2
      @End
      
      @SharedParams
      sp1	100	shared param for trial
      sp2	150	shared param for trial 2
      @End

      In turn, these names can be used instead of numbers when reading measures
     */
    bool readSharedAndIdleParams(ifstream_ptr in,lbMeasureParamType type);

    /*!
      Read directed measures from file (to create a chain graph)
     */
    bool readDirectedMeasuresList(ifstream_ptr in);
    
    /*!
      Read assignment of template measures to cliques from tab delimieted file
      in the following format:

      @CliqueToMeasure
      
      0	0

      1	1

      2	2

      @End

      Where the columns are:
      1)Index of clique 
      2)Index of measure
    */
    bool assignMeasureForCliques(ifstream_ptr in);


    /*!
      Updates the values of this measure from the values of another
      measure (optionally, into a weighted average between new and old
      measures)
      \param meas is the index of the measure that needs updating
      \param updatedMeasure is the other measure from which we update the
      values
      \param smoothingParam is the weight of this measure in the average
      (0.0 take other measure values, 1.0 don't touch values)
      \param epsilon a sensitivity thresold under which values should not be
      replaced
      \return true if the measure has changed
     */
    inline bool updateMeasure(measIndex meas,lbMeasure const& updatedMeasure,probType smoothingParam,
			      probType epsilon = lbDefinitions::EPSILON);

    /*!
      Update measure parameters from prob vector (with log values)
     */
    inline int setLogParamsToMeasure(measIndex meas,probType const* vec,bool& changed);

    /*!
      Notify the listeners of a change in measures (possibly only a set of a measures can be touched)
      \param vec is the set of measures that 
    */
    void updateListeners(measIndicesVec vec);

    /*!
      Take parameters of measures from vector of log-values
    */
    void updateLogParamsFromVector(probType const* vec,bool undirected,set<measIndex> const& measSet = set<measIndex>());
    
    /*!
      Add shared parameters to measure
    */
    void addSharedParamsToMeasure(measIndex meas,assignmentPtrVec const & assignVec,varsVec const& vars);

    /*!
      Make a measure directed and normalize
    */
    void makeMeasureDirected(measIndex meas);
    
    /*!
      Updated local copies of counting and region numbers (to be used in region BP)
     */
    inline void setRegionNumbers(vector<double> countingNums,
				 vector<double> powerNums);

    /*!
      Get local counting numbers of the model  (to be used in region BP)
     */
    const vector<double> & getCountingNums() const { return _countingNums; }

    /*!
      Set local counting numbers of the model  (to be used in region BP)
     */
    inline void setCountingNums (const vector<double> & newCountingNums) ;

    /*!
      Set local counting numbers for one measure of the model  (to be used in region BP)
     */
    inline void setCountingNum (int cliq, double newCountingNum) ;
    
    /*!
      TBD
     */
    inline void updateCountingNums (const vector<double> & newCountingNum) ;

    /*!
      Set power numbers for the model (to be used in region BP)
    */
    inline void setPowerNums (const vector<double> & newPowerNums) ;

    /*!
      Get local counting number for one measure of the model  (to be used in region BP)
     */
    inline double getCountingNum(int index);

    /*!
      Get local power number for one measure of the model  (to be used in region BP)
     */
    inline double getPowerNum(int index);

    /*!
      Set local copy of Bethe counting numbers of the model  (to be used in region BP)
     */
    inline void setBetheCountingNums (const vector<double> & betheCountingNums) ;

    /*!
      Get local copy of Bethe counting numbers of onde measure  (to be used in region BP)
     */
    inline double getBetheCountingNum(int index) ;

    /*!
      Make counting numbers variable valid
     */
    void makeVarValidCountingNums() ;

    /*!
      Revert a measure to its original parameters
     */
    inline void setOrigMeas (cliqIndex ni , const lbMeasure_Sptr meas) ;

    /*!
      Get original parameters of a measure
     */
    inline const lbMeasure_Sptr getOrigMeas (cliqIndex ni) const ;


    /*!
      Implementation of lbGraphListener method - 
      this needs to be filled if the graph listening
      mechanism need to be used
     */
    virtual bool graphUpdated() { assert(false); } ;
    /*  
**********************************************************
Accessors
**********************************************************
*/

    /*!
      Get the assigned measure for a clique
    */
    inline lbAssignedMeasure const& getAssignedMeasureForClique(cliqIndex clique) const;

    /*!
      Get the assigned measure for a clique
    */
    inline lbAssignedMeasure& getAssignedMeasureForCliqueNonConst(cliqIndex clique);

    /*!
      Get cardVec for a list of vars
    */
    inline cardVec getCardForVars(varsVec const& vars) const;

    /*!
      Get a set of cliques using a measure (for example, to compute its derivative)
     */
    inline cliquesVec const&  getCliquesUsingMeasure(measIndex meas) const;

    /*!
      Get total number of measures
     */
    inline int getNumOfMeasures() const;

    /*!
      get a measure
     */
    inline lbMeasure const& getMeasure(measIndex meas) const;

    /*!
      get a measure pointer
     */
    inline lbMeasure_Sptr & getMeasureMut (measIndex meas) ;

    /*!
      Set measure name
     */
    inline string getMeasureName(measIndex meas) const;

    /*!
      Get the measure assigned for a clique
     */
    inline measIndex getMeasureIndexForClique(cliqIndex cliq) const;

    /*!
      Find out whether a measure is assigned for a clique
     */
    inline bool isMeasureAssignedForClique(cliqIndex cliq) const;

    /*!
    Number of parameters in the model
    */
    long int getSize(bool undirectedOnly = false, set<measIndex> const& measSet = set<measIndex>()) const;
    
    /*!
      Number of parameters in the model
    */
    long int getParamNum(bool undirectedOnly = false, set<measIndex> const& measSet = set<measIndex>()) const;

    /*!
      calculate length of a vector (undir meas only) that will have room for all parameters
    */
    inline int calcUndirectedVecLength() const ;
    
    /*!
      Get vector of all parameters in log-value (to be used in learning)
     */
    virtual probType* getLogParamVector(int& length,bool undirected,set<measIndex> const& measSet = set<measIndex>()) const; 
    
    /*!
      Depracated ?
     */
    virtual probType* getLogValueVector(int& length,bool undirected,set<measIndex> const& measSet = set<measIndex>()) const; 

    /*!
      Get the graph fro this model
     */
    inline lbGraphStruct const& getGraph() const; 

    /*!
      get cardinalities and variables for this model
     */
    inline lbCardsList const& getCards() const; 

    /*!
      Get one vector with all cardinalities
     */
    inline cardVec getCardVec() const;

    /*!
      Get number of instnces for a template measure
     */
    inline int getNumOfMeasuresInstances(measIndex meas) const; 
      
    /*  
**********************************************************
Print methods
**********************************************************
*/
    /*!
    Print model
    */
    void printModel(ostream& out) const ;

    /*!
      print to fastinf format
      \sa readMeasures
    */
    void printModelToFastInfFormat(ostream& out,bool printNormalized = false,int prec = 5,
				   bool printLogValues = false) const ;

    /*!
      Print only vars and cards
    */
    inline void printVarsAndCardToFile(ostream& out) const ;

    /*!
      print all the model to a file
     */
    inline void printAllNetToFile(string fileName) const;
        
    /*!
      get measure dispatcher (factory for measures)
     */
    lbMeasureDispatcher const& measDispatcher() const { return *_measDispatcher; };

    /*!
      Set exact beliefs (for comparison and evaluation on small models)
     */
    void setExactBeliefs(assignedMesVec const& exact) { _exact = exact; }

    /*!
      Get exact beliefs (for comparison and evaluation on small models)
     */
    const assignedMesVec& getExactBeliefs() const { return _exact; }

  protected:
    
    explicit lbModel();
    void Init(bool resetUsing = true);
    void Clean();
    void CopyFromModel(lbModel const& oldModel);

    void SetGraph(lbGraphStruct const* graph) { _graph = graph; };
    void SetCards(lbCardsList const* cards) { _cardsList = cards; };
    void SetMeasureDispatcher(lbMeasureDispatcher const* md) { _measDispatcher = md; };

    measurePtrVec& MeasureVec() { return _mesVec; };

  private:

    bool _deepDest ;
    
    listenersVec _usingObjects;

    lbCardsList const* _cardsList;

    lbGraphStruct const* _graph;
    
    //holds all types of measure
    measurePtrVec _mesVec;
    
    //the vector with names of the measures
    stringVec _mesNames;

    //holds a measure type for each clique
    assignedMesVec  _assignedMeasures;

    //holds a list of cliques using each measure
    cliquesVecVector _measureUsage;

    //holds a list of measures using each clique
    measIndicesVec _cliqueUsage;

    //holds the measure dispatcher
    lbMeasureDispatcher const* _measDispatcher;

    paramPtrMap _sharedParams;
    paramPtrMap _idleParams;

    vector<double> _countingNums;
    vector<double> _powerNums;
    vector<double> _BetheCountingNums ;
    measurePtrVec _origMeasures ;
    assignedMesVec _exact;
  };
    
  inline void lbModel::setRegionNumbers(vector<double> countingNums,
					vector<double> powerNums) {
    setCountingNums (countingNums) ;
    setPowerNums (powerNums) ;
  }

  inline void lbModel::setCountingNums (const vector<double> & newCountingNums) {
    assert(newCountingNums.size() == (uint) getGraph().getNumOfCliques());
    _countingNums = newCountingNums ;
  }

  inline void lbModel::setCountingNum (int cliq, double newCountingNum) {
    assert (0 <= cliq && cliq < (int)_countingNums.size()) ;
    _countingNums[cliq] = newCountingNum ;
  }

  inline void lbModel::updateCountingNums (const vector<double> & newCountingNums) {
    assert (newCountingNums.size() == (uint) getGraph().getNumOfCliques()  &&
            newCountingNums.size() == _countingNums.size()) ;

    for (uint i=0 ; i<newCountingNums.size() ; ++i) {
      // we need to update measures of factors (see
      // lbRegionBP::addSingleCliqueMeasure)
      const varsVec & cliqVars = getGraph().getVarsVecForClique((cliqIndex) i);
      if (cliqVars.size() > 1) { // factor (we don't need to raise variabels potentials)
        lbMeasure_Sptr mes = getMeasureMut (getMeasureIndexForClique ((cliqIndex) i)) ;
        mes->raiseToThePower (_countingNums[i] / newCountingNums[i]);
      }

      _countingNums[i] = newCountingNums[i] ;
    }
  }

  inline void lbModel::setPowerNums (const vector<double> & newPowerNums) {
    assert(newPowerNums.size() == (uint) getGraph().getNumOfCliques());
    _powerNums = newPowerNums ;
  }

  inline void lbModel::setBetheCountingNums (const vector<double> & betheCountingNums) {
    assert(betheCountingNums.size() == (uint) getGraph().getNumOfCliques());
    _BetheCountingNums = betheCountingNums ;
  }
  
  inline void lbModel::setOrigMeas (cliqIndex ni , const lbMeasure_Sptr meas) {
    if (ni >= (int)_origMeasures.size()) {
      _origMeasures.resize ((int)ni+1) ;
    }
    _origMeasures[ni] = meas ;
  }
  inline const lbMeasure_Sptr lbModel::getOrigMeas (cliqIndex ni) const {
    assert (ni < (int)_origMeasures.size()) ;
    return _origMeasures[ni] ;
  }

  inline double lbModel::getCountingNum(int index) {
    assert(index >= 0 && index < getGraph().getNumOfCliques());
    return _countingNums[index];
  }

  inline double lbModel::getPowerNum(int index) {
    assert(index >= 0 && index < getGraph().getNumOfCliques());
    return _powerNums[index];
  }

  inline double lbModel::getBetheCountingNum(int index) {
    assert(index >= 0 && index < getGraph().getNumOfCliques());
    return _BetheCountingNums[index];
  }  

  inline string lbModel::getMeasureName(measIndex meas) const {
    return _mesNames[meas];
  }

  //sets a measure for all cliques
  inline bool lbModel::setMeasureForAllCliques(measIndex measure) {
    assert (measure<(measIndex)_mesVec.size());
    for (int cliq = 0 ; cliq < getGraph().getNumOfCliques() ; cliq++) {
      setMeasureForClique(cliq,measure);
    }
    return true;
  }
      
  //get the measure for a clique
  inline lbAssignedMeasure const& lbModel::getAssignedMeasureForClique(cliqIndex clique) const{
    return *_assignedMeasures[clique];
  }

  inline lbAssignedMeasure& lbModel::getAssignedMeasureForCliqueNonConst(cliqIndex clique) {
    return *_assignedMeasures[clique];
  }

  inline cardVec lbModel::getCardForVars(varsVec const& vars) const {
    cardVec result = cardVec(vars.size());
    for (uint ind=0;ind<(uint)vars.size();ind++){
      result[ind] = _cardsList->getCardForVar(vars[ind]);
    }
    return result;
  }
  
  inline cliquesVec const&  lbModel::getCliquesUsingMeasure(measIndex meas) const {
    return _measureUsage[meas];
  }

  inline int lbModel::getNumOfMeasures() const{
    return _mesVec.size();
  }

  inline lbMeasure const&  lbModel::getMeasure(measIndex meas) const{
    return (*_mesVec[meas]);
  }

  inline lbMeasure_Sptr & lbModel::getMeasureMut (measIndex meas) {
    return _mesVec[meas] ;
  }

  inline bool lbModel::updateMeasure(measIndex meas,lbMeasure const& updatedMeasure,probType smoothingParam,
				     probType epsilon) {
   
    bool ret =  _mesVec[meas]->updateMeasureValues(updatedMeasure,smoothingParam,epsilon);
    measIndicesVec measures(1,meas);
    updateListeners(measures);
    return ret;
  }

  inline measIndex lbModel::getMeasureIndexForClique(cliqIndex cliq) const{
    return _cliqueUsage[cliq];
  }

  inline void lbModel::printVarsAndCardToFile(ostream& out) const {
    _cardsList->printToFile(out);
    out<<"\n\n";
  }

  inline void lbModel::printAllNetToFile(string fileName) const{
    ofstream* out = new ofstream(fileName.c_str());
    //printVarsAndCardToFile(*out);
    //getGraph().printGraphToFastInfFormat(*out);
    printModelToFastInfFormat(*out);
    out->flush();
    out->close();
    delete out;
  }

  inline lbGraphStruct const& lbModel::getGraph() const {
    return *_graph;
  };
 
  inline lbCardsList const& lbModel::getCards() const {
    return *_cardsList;
  };

  inline cardVec lbModel::getCardVec() const {
    cardVec card;

    for (int i = 0; i < getGraph().getNumOfVars(); i++) {
      card.push_back(getCards().getCardForVar(i));
    }
    return card;
  };

  inline int lbModel::getNumOfMeasuresInstances(measIndex meas) const {
    return _measureUsage[meas].size();
  };

  inline bool lbModel::isMeasureAssignedForClique(cliqIndex cliq) const { 
    if ( _cliqueUsage.size() < (uint)cliq+1 )
      return false;
    return(_cliqueUsage[cliq]!=-1); 
  };

  inline int lbModel::setLogParamsToMeasure(measIndex meas,probType const* vec,bool& changed){
    return _mesVec[meas]->setLogParams(vec,0,changed);
  }
}
#endif
