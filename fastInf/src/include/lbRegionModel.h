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

// lbRegionModel.h
// Author: Ian McGraw

#ifndef _LB_REGION_MODEL_H_
#define _LB_REGION_MODEL_H_

#include <lbDefinitions.h>
#include <lbMeasureDispatcher.h>
#include <lbRegionGraph.h>
#include <lbAssignedMeasure.h>
#include <lbCardsList.h>

namespace lbLib {


  typedef vector<lbAssignedMeasure const*> lbAssignedMeasureConstPtrVec;

  class lbRegionModel {
  public:

    /*
     * Constructors and Destructors
     */
    lbRegionModel(lbMeasureDispatcher const& disp);
    lbRegionModel(lbRegionGraph const& graph,
		  cardVec const& cards,
		  lbAssignedMeasurePtrVec const& ampv,
		  lbMeasureDispatcher const& disp);
    lbRegionModel(lbRegionGraph const& graph,
		  cardVec const& cards,
		  lbAssignedMeasureCofwPtrVec* amcpv,
		  lbMeasureDispatcher const& disp);
    explicit lbRegionModel(lbRegionModel const& rgnmodel);
    
    virtual ~lbRegionModel();

    /*
     * Accessors
     */
    inline lbAssignedMeasure_ptr getMessage(arcIndex ai);
    inline lbAssignedMeasurePtrVec getCorrespondingMessages(arcIndexVec const& aiv);

    inline lbAssignedMeasureConstPtrVec getCorrespondingFactors(facIndicesVec const& fiv) const;
    inline lbAssignedMeasureConstPtrVec getAllFactors() const;
    
    inline bool isEmpty() const;

    inline lbRegionGraph & getRegionGraph();
    lbRegionGraph const& getRegionGraph() const;

    inline cardVec getCorrespondingCards(varsVec const& vec) const; 
    inline cardVec const& getCards() const;

    inline lbMeasureDispatcher const& measDispatcher() const;

    /*
     * Mutators
     */
    inline void readModel(ifstream_ptr in);
    inline void setBetheModel(lbAssignedMeasurePtrVec const& ampv);
    inline void setMessage(arcIndex ai, lbAssignedMeasure_ptr message);

    /*
     * Printing
     */
    inline void print(ostream & out) const;
    inline void printFormatted(ostream & out) const;

    /*
     * Debugging
     */
    inline /*virtual*/ bool isValid() const;

  private:

    inline void initializeMessages();

    /*
     * Reading helpers.
     */

    static const string FAC_DELIM;
    static const string VAR_DELIM;
    static const string END_DELIM;

    void readVariables(ifstream_ptr in);
    void readFactors(ifstream_ptr in);

    lbRegionGraph _rgnGraph;
    cardVec _cardVec;
    lbAssignedMeasureCofwPtrVec* _factors;
    lbMeasureDispatcher const& _disp;

    lbAssignedMeasurePtrVec _messages;
  };

  inline lbRegionGraph const& lbRegionModel::getRegionGraph() const {
    return _rgnGraph;
  }

  inline lbRegionGraph & lbRegionModel::getRegionGraph() {
    return _rgnGraph;
  }

  inline lbAssignedMeasure_ptr lbRegionModel::getMessage(arcIndex ai) {
    assert(ai >= 0 && (uint) ai < _messages.size());
    return _messages[ai];
  }

  inline lbAssignedMeasurePtrVec lbRegionModel::getCorrespondingMessages(arcIndexVec const& aiv) {
    lbAssignedMeasurePtrVec vec;
    assert((uint) getRegionGraph().getNumArcs() == _messages.size());
    for (uint i = 0; i < aiv.size(); i++) {
      assert(aiv[i] >= 0 && aiv[i] < getRegionGraph().getNumArcs());
      vec.push_back(_messages[aiv[i]]);
    }

    return vec;
  }

  inline lbAssignedMeasureConstPtrVec lbRegionModel::getCorrespondingFactors(facIndicesVec const& fiv) const {
    lbAssignedMeasureConstPtrVec vec;

    for (uint i = 0; i < fiv.size(); i++) {
      assert(fiv[i] >= 0 && (uint) fiv[i] <= _factors->size());

      lbAssignedMeasureCofwPtr const& amCp = (*_factors)[fiv[i]];
      vec.push_back(amCp.get());
    }

    return vec;
  }

  inline lbAssignedMeasureConstPtrVec lbRegionModel::getAllFactors() const {
    lbAssignedMeasureConstPtrVec vec;

    for (uint i = 0; i < _factors->size(); i++) {
      lbAssignedMeasureCofwPtr const& amCp = (*_factors)[i];
      vec.push_back(amCp.get());
    }
    
    return vec;
  }

  inline bool lbRegionModel::isEmpty() const {
    return (getRegionGraph().isEmpty() &&
	    _factors->empty() &&
	    _messages.empty() &&
	    _cardVec.empty());
  }

  /*
   * We ensure a few things here.  First the region graph is valid.
   * Then we ensure family preservation.
   */

  inline bool lbRegionModel::isValid() const {
    if (!getRegionGraph().isValid()) {
      NOT_REACHED;
      return false;
    }

    if ((uint) getRegionGraph().getNumArcs() != _messages.size()) {
      NOT_REACHED;
      return false;
    }

    if (!getRegionGraph().isEmpty() &&
	(uint) getRegionGraph().getHighestVarIndex() >= _cardVec.size()) {
      NOT_REACHED;
      return false;
    }

    for (uint i = 0; i < _cardVec.size(); i++) {
      if (_cardVec[i] <= 0) {
	NOT_REACHED;
	return false;
      }
    }

    // Ensure family preservation (measures are in regions with 
    // the variables in their scopes) 
    for (nodeIndex ni = 0; ni < getRegionGraph().getNumNodes(); ni++) {
      lbRegion const& rgn = getRegionGraph().getRegion(ni);

      facIndicesVec fiv = rgn._facIndices;

      for (int i = 0; (uint) i < fiv.size(); i++) { 
	if (fiv[i] < 0 || (uint) fiv[i] >= _factors->size()) {
	  NOT_REACHED;
	  return false;
	}

	lbAssignedMeasureCofwPtr const& amCp = (*_factors)[fiv[i]];
	lbAssignedMeasure const* factor = amCp.get();
	if (factor == NULL) {
	  return false;
	}

	varsVec vars = factor->getVars();
	lbRegion tmprgn(vars, facIndicesVec());
      
	if (!tmprgn.isSubsetOf(rgn)) {
          /*cout << "[DEBUG] tmprgn:" << endl ;
          tmprgn.print(cout) ;
          cout << "[DEBUG] rgn:" << endl ;
          rgn.print(cout) ;*/
	  NOT_REACHED;
	  return false;
	}
      }
    }

    return true;
  }

  inline cardVec lbRegionModel::getCorrespondingCards(varsVec const& vec) const {
    cardVec cvec;
    
    for (uint i = 0; i < vec.size(); i++) {
      assert(vec[i] >= 0 && (uint) vec[i] < _cardVec.size());
      cvec.push_back(_cardVec[vec[i]]);
    }
    
    return cvec;
  }

  inline cardVec const& lbRegionModel::getCards() const {
    return _cardVec;
  }

  inline lbMeasureDispatcher const& lbRegionModel::measDispatcher() const {
    return _disp;
  }

  inline void lbRegionModel::setMessage(arcIndex ai, lbAssignedMeasure_ptr message) {
      assert(ai >= 0 && (uint) ai < _messages.size());
      if (_messages[ai]) {
         delete _messages[ai];
      }
      _messages[ai] = message;
  }
  
  inline void lbRegionModel::print(ostream & out) const {
    getRegionGraph().print(out);
  }

  inline void lbRegionModel::printFormatted(ostream & out) const {
    out << "# Format: variable name \\t variable cardinality" << endl;
    out << VAR_DELIM << endl;

    for (uint i = 0; i < _cardVec.size(); i++) {
      out << "V" << i << "\t" << _cardVec[i] << endl;
    }

    out << END_DELIM << endl << endl;

    out << "# Format: factor name \\t num vars involved";
    out << "\\t cardinalities \\ values 000 001 010..." << endl;
    out << FAC_DELIM << endl;

    for (measIndex mi =  0 ; (uint) mi < _factors->size(); mi++) {
      out << "F" << mi << "\t";

      lbAssignedMeasureCofwPtr const& amCp = (*_factors)[mi];
      const lbAssignedMeasure* factor = amCp.get();
      
      varsVec assn = factor->getVars();
      out << assn.size() << "\t";
      for (uint i = 0; i < assn.size(); i++) {
	out << assn[i] << " ";
      }
      out << "\t";

      factor->getMeasure().printToFastInfFormat(out);
    }

    out << END_DELIM << endl << endl;

    getRegionGraph().printFormatted(out);
  }


  inline void lbRegionModel::setBetheModel(lbAssignedMeasurePtrVec const& ampv) {
    assert(isEmpty());

    varIndicesVecVec vivv;
    for (uint i = 0; i < ampv.size(); i++) {
      varsVec vars = ampv[i]->getVars();
      cardVec cards = ampv[i]->getMeasure().getCards();
      assert(vars.size() == cards.size());

      vivv.push_back(vars);
      
      for (uint j = 0; j < vars.size(); j++) {
	rVarIndex curVar = vars[j];

	if ((uint) curVar >= _cardVec.size()) {
	  _cardVec.resize((int) curVar + 1, 0);
	}

	assert(_cardVec[curVar] == 0 || _cardVec[curVar] == cards[j]);
	_cardVec[curVar] = cards[j];
      }

      _factors->push_back( lbAssignedMeasureCofwPtr(ampv[i]->duplicate(),true) );
      _factors->back().setDeleteCopiedPointer(); //so that new measure will be deleted
    }
    
    getRegionGraph().setBetheRegions(vivv);
    initializeMessages();
    assert(isValid());
  }

  inline void lbRegionModel::readModel(ifstream_ptr in) {
    swallowComments(*in);
    readVariables(in);
    swallowComments(*in);
    readFactors(in);
    swallowComments(*in);
    getRegionGraph().readGraph(*in);
    initializeMessages();
    assert(isValid());
  }

  inline void lbRegionModel::readVariables(ifstream_ptr in) {
    char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
    (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
    string str(buffer.get());
    assert(str == VAR_DELIM);
    
    int card;
    while(!(str == END_DELIM)) {
      (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
      str = string(buffer.get());
      istringstream iss(str);

      iss >> str; // Discard name
      if (str[0]!='@') {
	iss >> card;
	_cardVec.push_back(card);
      }
    }
  }

  inline void lbRegionModel::readFactors(ifstream_ptr in) {
    char_ptr buffer(new char[lbDefinitions::MAX_BUF_SIZE]);
    (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
    string str(buffer.get());
    assert(str == FAC_DELIM);
    
    while((*in).peek() != '@') {
      (*in).getline(buffer.get(), lbDefinitions::MAX_BUF_SIZE, lbDefinitions::DELIM);
      str = string(buffer.get());
      // We either got a factor name (which we won't use) or the end
      
      lbAssignedMeasure_ptr fac;
      fac = lbAssignedMeasure::readOneMeasure(in, _disp);
      _factors->push_back( lbAssignedMeasureCofwPtr(fac,true) );
      _factors->back().setDeleteCopiedPointer(); //so that new measure will be deleted

      assert((*in).good() && !(*in).eof());
    }

    (*in).getline(buffer.get(),lbDefinitions::MAX_BUF_SIZE);
    string strend(buffer.get());
    assert(strend == END_DELIM);
  }
  

  inline void lbRegionModel::initializeMessages() {
    assert(_messages.size() == 0);

    for (arcIndex ai = 0; ai < getRegionGraph().getNumArcs(); ai++) {
      arc a = getRegionGraph().getArc(ai);
      lbRegion const& rgn = getRegionGraph().getRegion(a.second);
      cardVec card = getCorrespondingCards(rgn._varIndices);

      lbMeasure_Sptr mes = _disp.getNewMeasure(card, false /* random bool */);
      lbAssignedMeasure_ptr assnmes = new lbAssignedMeasure(mes, rgn._varIndices);
      _messages.push_back(assnmes);
    }
  }
};

#endif
