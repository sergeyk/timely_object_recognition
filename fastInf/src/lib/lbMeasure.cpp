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

#include <lbMeasure.h>

using namespace lbLib ;

/**
   This Class holds the measure for the graph

   This Object holds the probabilistic characters of a clique.
   and performs simple actions on it
   
   Part of the loopy belief library
   @version November 2002
   @author Ariel Jaimovich
*/



//int lbMeasure::_counter = 0;

/*
  lbMeasure& lbMeasure::operator=(lbMeasure const& oldMeasure){
  cerr<<"in oper =(meas)"<<endl;
  _totalWeightIsUpdated = oldMeasure._totalWeightIsUpdated;
  _card=(cardVec)oldMeasure._card;
  _type=oldMeasure._type;
  _vectorsAreInitialized=true;
  _totalWeight=oldMeasure._totalWeight;
  return *this;
  }
*/
bool lbMeasure::MAX_PRODUCT = false;
bool lbMeasure::LOG_SMOOTH = false;

void lbMeasure::setMaxProduct(bool maxProd) {
  MAX_PRODUCT=maxProd;
}


int lbMeasure::buildCardVec(cardVec const& newCard) {
  _cardSize = newCard.size();
  _card.resize(_cardSize);
  int probLen = 1;

  for (uint i = 0; i < newCard.size(); i++) {
    _card[i] = newCard[i];
    probLen *= _card[i];
  }
  updateStrideVector();
  return probLen;
}

int lbMeasure::buildCardVec(cardVec const& oldCard,
			    internalIndicesVec const& remainingVars){
  //init vars
  _cardSize = remainingVars.size();
  _card = cardVec(_cardSize);
  int probLen=1;
  //build new measure card  
  for (uint ind=0;ind<_cardSize;ind++) {
    int tmp = oldCard[remainingVars[ind]];
    _card[ind]=tmp;
    probLen*=tmp;
  }
  updateStrideVector();
  return probLen;
}

void lbMeasure::buildCardVec(cardVec const& thisCard,
			     cardVec const& otherCard,
			     varsVecPair const& remaining){
  
  uint firstSize = thisCard.size();
  uint commonSize = remaining.size();
  uint finalSize = firstSize + commonSize;
  _cardSize=finalSize;
  //fill new card 
  _card = cardVec(finalSize);
    
  //putting first mes elements :
  uint ind;
  for (ind = 0 ; ind < firstSize ; ind++) {
    _card[ind]=thisCard[ind];
  }

  //putting second mes elements :
  for (ind = firstSize; ind <finalSize ; ind++) {
    _card[ind]=otherCard[remaining[ind-firstSize].first];
  }
  updateStrideVector();
}

void lbMeasure::makeDirected() {
  //cerr << "Making measure directed...\n";
  _isDirected=true;
}

void lbMeasure::makeUndirected() {
  //cerr << "Making measure undirected...\n";
  _isDirected=false;
}

void lbMeasure::updateStrideVector()
{
    // first build a vector of strides
    strideVec stride(_cardSize);
    strideVec partialStride(_cardSize);
    stride[_cardSize - 1] = 1;
    partialStride[_cardSize - 1] = 1;
    for (int i =_cardSize - 2; i >= 0 ;i--) {
	stride[i] = stride[i + 1] * _card[i + 1];
	partialStride[i] = partialStride[i + 1] * _card[i];
    }

    // find maximal card
    int maxCard = 0;
    for (int i=0 ; i<(int)_cardSize ; i++ )
        if ( _card[i]>maxCard )
            maxCard = _card[i];

    // update stride tables
    _stride.resize(_cardSize);
    _partialStride.resize(_cardSize);
    for (int i=0 ; i<(int)_cardSize ; i++ )
        for (int c=0 ; c<maxCard ; c++ ) {
            _stride[i].push_back(stride[i]*c);
            _partialStride[i].push_back(partialStride[i]*c);
        }
}


