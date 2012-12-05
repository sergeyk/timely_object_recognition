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

#ifndef _Loopy__Measure
#define _Loopy__Measure


#include <lbDefinitions.h>
#include <lbAssignment.h>
//#include <lbIndexConverter.h>
#include <sstream>
#include <lbRandomProb.h>
#include <lbValue.h>

namespace lbLib {


  
  const measIndex ALL_MEASURES = -1;
  
  /*!
     Struct holding a parameter name, value and description
  */
  struct lbParam {
    /*!
       Constructor
       \param name The parameter name
       \param val The parameter value
       \param description The text description of the parameter
    */
    lbParam(string name,probType val,string description) :
      _name(name),
      _val(val),
      _description(description) {
    };

    /*!
       Getting the parameter name
       \return a string with the parameter name
    */
    string getName() const{
      return _name;
    };
      
    /*!
       Getting the parameter value
       \return a double with the parameter value
    */
    probType getVal() const {
      return _val;
    };

    /*!
       Getting the parameter description
       \return a double with the parameter value
    */
    string getDescription() const{
      return _description;
    };

  private:
    string _name;
    probType _val;
    string _description;
  };

  typedef map<string,lbParam*> paramPtrMap ;
  typedef map<string,lbParam*>::const_iterator paramPtrMapCIter;
  typedef map<string,lbParam*>::iterator paramPtrMapIter;
  
  class lbMeasure; 
  typedef shared_ptr<lbMeasure> lbMeasure_Sptr;
  typedef safeVec<lbMeasure_Sptr> measurePtrVec ;
  typedef measurePtrVec::iterator measurePtrVecIter;
  typedef enum {COMBINE_MULTIPLY, COMBINE_DIVIDE} CombineType;
  /*!
     This Class holds the ABSTRACT measure for the graph
     
     This Object holds the probabilistic characters of a clique.
     and performs simple actions on it
     
     Part of the fastInf library
     \version July 2009
     \author Ariel Jaimovich
  */

  class lbMeasure {
	
  public:
    /*!
       Default constructor
     */
    inline lbMeasure();
    
    /*!
       Cosntructor with cardinalities
       \param card is a vector with the cardialities of the variables in this measure
     */
    inline lbMeasure(cardVec const& card);
    
    /*!
       Copy constructor
     */
    inline explicit lbMeasure(lbMeasure const& oldMeasure);

    /*!
       Distractor
     */
    inline virtual ~lbMeasure();

    // Accessing or changing values in measure 

    /*!
       Main function of a measure: return the entry for an assignment to the variables
       since, this measure is template, it also needs to get a vector of variables, with
       respect to which it will return the value.
       \param assign is the assinment for which we want the value
       \param vars are the variables for which we are interested in this instantiation
       \return the value corresponding to the assignment of the variables 
     */
    virtual probType valueOfFull(lbBaseAssignment const& assign,
				 varsVec const& vars) const =0;
    /*!
      Same as valueOfFull, only returns the log value of the entry
      \sa valueOfFull
       \param assign is the assinment for which we want the value
       \param vars are the variables for which we are interested in this instantiation
       \return the log value corresponding to the assignment of the variables 
     */
    virtual probType logValueOfFull(lbBaseAssignment const& assign,
				    varsVec const& vars) const =0;

    /*!
      Set the entry value in this potential (as determined for a
      specific assginment for the variables given)
      \param assign is the assinment for which we want the value
      \param vars are the variables for which we are interested in this instantiation
      \param value is the value to be assigned in the potential 
    */
    virtual void setValueOfFull(lbBaseAssignment const& assign,
				varsVec const& vars,
				probType value)  = 0;

    /*!
      Same as setValueOfFull, only sets the log value of the entry
      \sa setValueOfFull
      \param assign is the assinment for which we want the value
      \param vars are the variables for which we are interested in this instantiation
    */
    virtual void setLogValueOfFull(lbBaseAssignment const& assign,
				   varsVec const& vars,
				   probType value) = 0;

    /*!
      Gets a random value of a specific variable. Asserts that this
      measure is defined over one variable.
      To be used in Gibbs Sampling
      \return The value of the variable that was sampled
     */

    virtual varValue randomValFromTable() = 0;
    
    /*!
      Gets the assignment with the highest potential in this measure.
      \param vars are the variables for which we are interested in
      this instantiation
      \return The assignment to the variables that get the highest potential.
     */
    virtual lbAssignment_ptr getMAPassign (const varsVec & vars) const = 0;

    // Actions on Measure that yield new measure 

    /*!
      Duplicates a copy of this measure
      \return A smart pointer to the duplicated measure
     */
    virtual lbMeasure_Sptr duplicate() const =0;
 
    /*!
     For measure that have complex inner representation,
     duplicate values is used for inference purposes
     \return A smart pointer to a new measure (usually of a different
     class) that has the same values 
    */
    virtual lbMeasure_Sptr duplicateValues() const =0;

    // Actions on Measure that change  measure

    /*!
      Reads a measure from a file:
      The measure should be tab delimited and look like this:
      meas_name	2	2 2	0.4 0.6 1.4 0.3 
      Where the columns are:
      1) Name
      2) Number of variables
      3) Cardinality of variables
      4) Values for assignments in increasing manner (i.e., 00 01 10
      11)
      \param in is the ifstream pointer that contains the file buffer
      \param sharedMap is a reference to shared parameters list (to create a
      parameter that can be shared between two entries - for learning
      purposes)
      \param idleMap is a reference to a list of idle parameters that can be
      used to create paramters that won't be changed in learning
      (usually denoting 'dont care' parameters with value=0
      \param readLogValues is a flag (default = false) that can be
      used to read parameters in log value. 
     */
    virtual void readOneMeasure(ifstream_ptr in,
				paramPtrMap & sharedMap,
				paramPtrMap & idleMap,
				bool readLogValues = false)=0;

    /*! 
      Used to update values of a potential to a partial/full evidence
      where the values for assignments that don't agree with the
      evidence are zeroed.
      \param givenAssign is the evidence
      \param vars are the variables for which we are interested in
      this instantiation
      \param original is a copy of the original measure in case we
      need to unzero values back to their original value
      \return true if at least one value has changed
     */
    virtual bool updateAssign(lbAssignment const& givenAssign,
			      varsVec const& vars,
			      lbMeasure const& original)=0 ;

    /*! 
      Used to update values of a potential to a partial/full evidence
      where the values for assignments that don't agree with the
      evidence are zeroed.
      \param givenAssign is the evidence
      \param vars are the variables for which we are interested in
      this instantiation
      \return true if at least one value has changed
    */    
    virtual bool updateAssign(lbAssignment const& givenAssign,
			      varsVec const& vars) = 0;
			      
    /*!
      Adds the values of another measure to the values of this one
      /param meas is the other measure
      \return true if the addition was succesful
     */
    virtual bool addMeasure(lbMeasure const& meas) =0;

    /*!
      Normalize a measure so its entries sum to 1
     */
    virtual void normalize() =0;

    /*!
      Normalize a directed measure (where the last variable in the
      list is considered the child) so that each conditional vector
      sums to 1
    */
    virtual void normalizeDirected() =0;

    /*!
      Calculate the total weight (sum of all entries in the measure)
      \return the total weight of the measure
     */
    virtual probType totalWeight()=0;

    /*!
      Raises the values of this measure to a power
      \param power is the power (possibly not integer)
      \return true if the operation was succesful
     */
    virtual bool raiseToThePower(probType power) = 0;

    /*!
      Marginalize this measure into a new measure. This method can
      assign the resulting marginalization into the new measure or to
      multiply it with existing values
      \sa marginalizeWithAssign
      \param newMeasure is the measure into which we are marginalzing
      \param newVec are the variables which we want to remain in the
      marginalized measure
      \param oldVec is the vector of variables over which the existing
      measure is defined
      \param multiply indicates whether we want the result assigned as is
      into newMeasure or multiplied into it (default = false)
      \return true if the action was succesful
     */
    virtual bool marginalize(lbMeasure& newMeasure,
				    varsVec const& newVec,
				    varsVec const& oldVec,
				    bool multiply = false) const = 0;
    
    /*!
      Marginalize this measure into a new measure. The difference from
      marginalize is that here we get an assignment of variables that
      are observed (So we don't need to sum them)
      \sa marginalize
      \param newMeasure is the measure into which we are marginalzing
      \param newVec are the variables which we want to remain in the
      marginalized measure
      \param oldVec is the vector of variables over which the existing
      measure is defined
      \param assign is the evidence that needs to be assigned
      \param multiply indicates whether we want the result assigned as is
      into newMeasure or multiplied into it (default = false)
      \return true if the action was succesful
     */    
    virtual bool marginalizeWithAssign(lbMeasure& newMeasure,
					      varsVec const& newVec,
					      varsVec const& oldVec,
					      lbBaseAssignment const& assign,
					      bool multiply = false) const = 0;
    
    /*!
      Multiplies or divides this measure by another measure.
      \param newTable is the table into which the result is assigned
      \param table is the 'right' table to be multipied/divided
      \param leftVec is the vars for this measure
      \param rightVec are the vars for the 'left' measure
      \param ct is an enum defining whether this will be
      multiplication or division
      \return true if the operation was succesful
     */
    virtual bool combineMeasures(lbMeasure& newtable,
					const lbMeasure& table,
					const varsVec& leftVec,
					const varsVec& rightVec,
					CombineType ct) const = 0;

    /*!
      Updates the values of this measure from the values of another
      measure (optionally, into a weighted average between new and old
      measures)
      \param other is the other measure from which we update the
      values
      \param smoothingParam is the weight of this measure in the average
      (0.0 take other measure values, 1.0 don't touch values)
      \param epsilon a sensitivity thresold under which values should not be
      replaced
      \return true if the measure has changed
     */
    virtual bool updateMeasureValues(lbMeasure const& other,probType smoothingParam,
				     probType epsilon = -1) =0;

    /*!
      Makes all entries in this measure to be zero
     */
    virtual void makeZeroes() = 0;

    /*!
      Randomize all entries in this measure
     */
    virtual void makeRandom() = 0;

    /*!
      Make this measure uniform by making all entries equal to one
     */
    virtual void makeUniform() = 0;

    virtual void makeSparse(double sparsity) = 0;

    /*!
      Substruct another measure from this one
      \param other the measure to be substructed from this one
     */
    virtual void subtractMeasure(lbMeasure const& other)=0;

    /*!
      Set measure to work with Max instead of Sum in all
      marginalizations
      \param maxProduct is true if you want to work with max (false
      for sum product)
     */
    static void setMaxProduct(bool maxProduct);
    
    /*!
      Gets working mode (sum/max product)
      \return true if we work with max and false if we work with sum
    */
    static bool getMaxProduct() { return MAX_PRODUCT; }

    /*!
      Set the smoothing to be done in log space (when updating params)
      \param logSmooth is true if we want the smoothing to be done in
      log space
     */
    static void setLogSmooth(bool logSmooth) { LOG_SMOOTH = logSmooth; }

    /*!
      Get the smoothing mode (log/nonlog) of updating measures
      \return true if the update is done in log space
     */
    static bool getLogSmooth() { return LOG_SMOOTH; }

    /*!
      Adds the log quiotient of two other measures to this one
      \param nominator the measure in the nominator
      \param denominator the measure in the denominator
     */
    virtual void addLogOfQuotient(lbMeasure const& nominator,lbMeasure const& denominator) = 0;

    /*!
      Enable shared params for learning
      \param mode the sharing in derivative (sum / avarage)
     */
    virtual void enableSharedParams(lbSharedParamMode mode)=0 ;

    /*!
      Enable shared params for learning
      \param mode the sharing in derivative (sum / avarage)
     */
    virtual void enableIdleParams()=0;

    /*!
      Add idle params
     */
    virtual void addIdleParams(assignmentPtrVec const & vec,varsVec const& var)=0;
    
    /*!
      Add shared params
    */
    virtual void addSharedParams(assignmentPtrVec const & vec,varsVec const& vars) =0;

    /*!
      Make this measure directed (the last var is the son)
      (Default is undirected)
     */
    virtual void makeDirected();

    /*!
      Make this measure undirected
      (Default is undirected)
     */
    virtual void makeUndirected();

    virtual void subtractSumOfConditional(lbMeasure const & meas)=0;

    /*!
      Divide this measure by another
      \param otherM the measure in which we are dividing
     */
    virtual void divideMeasureByAnother(lbMeasure const& otherM)=0;

     virtual void divideFromReplace(lbMeasure const& otherM)=0;

     /*!
       Multiply this measure by another one (multiply all values, not
       matrix-wise
       \param otherM the measure with which we multiply
      */
    virtual void multiplyMeasureByAnother(lbMeasure const& otherM) = 0;

    /*!
      Multiply all entries in this measure by a number
      \param num the number in which we multiply
     */
    virtual void multiplyMeasureByNumber(probType num)=0;


    virtual void multiplyByConditionalSumOfMeasure(lbMeasure const& measure)=0;

    virtual void replaceValues(probType from, probType to) = 0;

    // Operators
 
    inline virtual lbMeasure& operator=(lbMeasure const& oldMeasure);
    
    virtual bool operator==(lbMeasure const& otherMeasure)=0 ;
    
    virtual probType operator() (lbBaseAssignment const& assign,
				 varsVec const& vars) const =0;   
    
    virtual void print(ostream& out) const=0;

    /*!
      Prints a measure to fastInf format
      \sa readOneMeasure
      \param out the stream to which we want to print
      \param normalizeValues whether we want the printed values to sum
      to one (default = false)
      \param prec the precision for printing (default = 5 digits after
      the floating point)
      \param ptinLogValues whether we want to print parameters in log
      value (default = false)
      
     */
    virtual void printToFastInfFormat(ostream& out,bool normalizeValues = false,int prec = 5,
				      bool printLogValues = false) const=0;

    /*!
      Check whether this and other measure are differnt
      \param other is the measure we are comparing to
      \param type is the comparison type (sum/max/kl)
      \param epsilon is the threshold for difference
      \return true if measures are different
     */
    virtual bool isDifferent(lbMeasure const& other,
			     const lbMessageCompareType type,
			     probType epsilon) const=0;
    
    virtual inline cardVec const& getCards() const;

    /*!
      \return the sum of all cardinalities
     */
    virtual inline int getSize() const;

    virtual inline bool isDirected() const;
      

    // Get put measures in vector 

    /*!
      Get number of entries in the table
     */
    virtual int getParamNum() const = 0;

    /*!
      Calculate the derivative of the likelihood
      For markov random fields the first derivative is equal to the
      difference between expected marginals and observed counts
      Here we put all the dervatives in a vector (to be used in
      learning methods)
      \param res is the vector with calculated derivatives for this
      model
      \param index is the index in which we insert the derivatives of
      this measure
      \param Estimated contains the estimated marginals
      \param Empirical contains the empirical counts
      \return the index where the next measure should insert its derivatives
     */
    virtual int calcDeriv(probType* res,int index,
			  lbMeasure const& Estimated,
			  lbMeasure const& Empirical) const = 0;


    /*!
      Add values of this measure to a vector
      \param vec the vector into which we insert the values
      \param index the index from which we start inserting the values
      \param logValues whetehr we insert real balues or logValues
      \return the index where the next measure should insert its derivatives      
     */
    virtual int extractValuesAddToVector(probType* vec,int index,bool logValues) const =0;

    /*!
      Same as extractValuesAddToVector only here we use only log values 
      \sa extractValuesAddToVector 
    */
    virtual int extractLogParamsAddToVector(probType* vec,int index) const=0;
    
    /*!
      Convert measure to probability vector
     */
    virtual probVector measure2Vec() const=0;

    /*!
      Update measure from prob vector (with log values)
     */
    virtual int setLogParams(probType const* vec,int index,bool& changed)=0;
    
    // sparsity

    virtual void setSparsityThresh(double sparseLevel) const = 0;

  protected: 
    /*!
      Implementation of isDifferent where we examine the l_infinity norm
      \sa isDifferent
    */
    virtual bool isDifferentMAX(lbMeasure const& other,probType epsilon) const=0;
    
    /*!
      Implementation of isDifferent where we examine the l_infinity norm on
      log values
      \sa isDifferent
    */
    virtual bool isDifferentLogMAX(lbMeasure const& other,probType epsilon) const=0;
    
    /*!
      Implementation of isDifferent where we examine the avarage distance
      \sa isDifferent
    */
    virtual bool isDifferentAVG(lbMeasure const& other,probType epsilon) const=0;

    /*!
      Implementation of isDifferent where we examine the avarage
      distance on log values
      \sa isDifferent
    */
    virtual bool isDifferentLogAVG(lbMeasure const& other,probType epsilon) const=0;

    /*!
      Implementation of isDifferent where we examine the KL divergence
      \sa isDifferent
    */
    virtual bool isDifferentKL(lbMeasure const& other,probType epsilon) const=0;

    virtual int buildCardVec(cardVec const& newCard);
 
    virtual int buildCardVec(cardVec const& oldCard,
			     internalIndicesVec const& remainingVars);
     
    virtual void buildCardVec(cardVec const& thisCard,
			      cardVec const& otherCard,
			      varsVecPair const& remaining);
    /*!
      Update local conversion tables
     */
    void updateStrideVector();

    /*!
      convert an assignment to an index for this measure
     */
    inline probIndex assignToInd(lbBaseAssignment const& assign, varsVec const& vars) const;

    /*!
      convert an partial assignment to an index for this measure
     */
    inline probIndex assignToIndPartial(lbBaseAssignment const& assign, varsVec const& vars) const;

    /*!
      Static data member determining whether all marginalization
      opreations are done in max or sum product
     */
    static bool MAX_PRODUCT;

    /*!
      Static data member determining whether all smoothing is done in
      log space
     */
    static bool LOG_SMOOTH;
    
    bool _totalWeightIsUpdated;
    
    cardVec _card;
    uint _cardSize;

    //local conversion tables from assign to index
    strideVecVec _stride;
    strideVecVec _partialStride;

    // Conditional or not.
    graph_type _type;

    bool _vectorsAreInitialized;
    
    bool _isDirected;
  };

  inline cardVec const& lbMeasure::getCards() const {
    return _card;
  }

  inline int lbMeasure::getSize() const {
    cardVec const& cvec = getCards();
    int s = 1;
    for ( uint c=0 ; c<cvec.size() ; c++ )
      s *= cvec[c];
    return s;
  }

  inline lbMeasure::lbMeasure() {
    _vectorsAreInitialized = false;
    _card=cardVec();
    _cardSize=0;
    _stride=strideVecVec();
    _partialStride=strideVecVec();
    _totalWeightIsUpdated=false;
    _isDirected=false;
  }

  inline lbMeasure::lbMeasure(cardVec const& card) :
    _card(card){
    _vectorsAreInitialized = false;
    _totalWeightIsUpdated=false;
    _cardSize=_card.size();
    _isDirected=false;
    updateStrideVector();
  }

  //copy ctor
  inline lbMeasure::lbMeasure(lbMeasure const& oldMeasure):
    _totalWeightIsUpdated(oldMeasure._totalWeightIsUpdated),
    _card(oldMeasure._card),
    _cardSize(oldMeasure._cardSize),
    _stride(oldMeasure._stride),
    _partialStride(oldMeasure._partialStride),
    _type(oldMeasure._type),
    _vectorsAreInitialized(oldMeasure._vectorsAreInitialized),
    _isDirected(oldMeasure._isDirected) {
  }

  //
  inline lbMeasure& lbMeasure::operator=(lbMeasure const& oldMeasure)
    {
      _totalWeightIsUpdated = oldMeasure._totalWeightIsUpdated;
      _card = oldMeasure._card;
      _cardSize = oldMeasure._cardSize;
      _stride = oldMeasure._stride;
      _partialStride = oldMeasure._partialStride;
      _type = oldMeasure._type;
      _vectorsAreInitialized = oldMeasure._vectorsAreInitialized;
      _isDirected = oldMeasure._isDirected;
      return *this;
  }

  //dtor
  inline lbMeasure::~lbMeasure() {
  }

  inline bool lbMeasure::isDirected() const {
    return _isDirected;
  }
  
  inline probIndex lbMeasure::assignToInd(lbBaseAssignment const& assign,
					  varsVec const& vars) const {
    probIndex index = 0 ;
    for (int i = _cardSize - 1; i >= 0; i--) {
      index += _stride[i][assign.getValueForVar(vars[i])];
      //	  index += _stride[i] * assign.getValueForVar(vars[i]);
    }      
    return index;
  }
  
  inline probIndex lbMeasure::assignToIndPartial(lbBaseAssignment const& assign,
						 varsVec const& vars) const {
    probIndex index = 0 ;
    for (int i = _cardSize - 2; i >= 0; i--) {
      index += _partialStride[i + 1][assign.getValueForVar(vars[i])];
      //	  index += _partialStride[i + 1] * assign.getValueForVar(vars[i]);
    }      
    return index;
  }
  
}; // namespace

#endif
      
