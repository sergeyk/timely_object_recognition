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

#ifndef _PATTFIND_GENERAL_HH
#define _PATTFIND_GENERAL_HH


#include "Multinomial.h"
#include "BGModelScorer.h"

/* #include "SmartPointer.h" */

#include <iostream>
#include <map>
#include <vector>
#include <string>
/* #include "DirichletPrior.h" */
/* #include "Gaussian.h" */

//using namespace std;

#define Vec vector
#define DEBUG 0


/// general template definitions //////////////////////////////////////////////////////

typedef uint uint;

// template<class T > class Vec : public vector<T> {
//  public:
// //   typename vector<T>::const_iterator const_iterator;
// //   typename vector<T>::iterator iterator;
  
//   Vec(): vector<T>(){}
//   Vec(int size) :vector<T>(size){}
//   Vec(int size, T t) :vector<T>(size,t){}
//   Vec(vector<T> const & vt) : vector<T>(vt) {}
//   T& operator[] (uint i) {
//     assert( i >= 0 && i < size() );
//     return vector<T>::operator[](i);
//   }
//   T operator[] (uint i) const
//   {
//     assert( i >= 0 && i < size() );
//     //const long double & val =  vector<long double>::operator[](i);
//     //return val;
//     return vector<T>::operator[](i);
//   }
// };



template <class T1, class T2 >
inline
istream & operator>>(istream & in, pair<T1,T2> & p ) {
  string tmp;
  in>>tmp;
  assert(tmp=="<");
  in>>p.first;
  in>>tmp;
  assert(tmp == ",");
  in>>p.second;
  in>>tmp;
  assert(tmp == ">");
  return in;
}

template <class T1 , class T2 >
inline
ostream & operator<<(ostream & os, pair<T1,T2> const & p ) {
  os<<"< "<<p.first<<" , "<<p.second<<" > ";
  return os;
}


template <class T>
inline
ostream & operator<<(ostream & os, const safeVec<T>& v) {
  os<<"{ ";
  for (uint i=0;i<v.size();i++)
    os<<v[i]<<" ";
  os<<"} ";
  return os;
}

template <>
inline
ostream & operator<<(ostream & os, const safeVec<unsigned char>& v) {
  os<<"{ ";
  for (uint i=0;i<v.size();i++)
    os<<(int)v[i]<<" ";
  os<<"} ";
  return os;
}

/**
 * assumes:
 * 1) the vector is build like that: { o1 o2 o3 ...} when o
 * a. has an empty constructor.
 * b. it's toString representation NEVER STARTS with the '}' char
 * c. has an operator >> defined for him.
 */
template <class T>
inline
istream & operator>>(istream & in , safeVec<T>& v) {
  string s;
  char c;
  in>>s;
  assert( !s.compare("{" ));
  do {
    do 
      in.get(c);
    while (isspace(c));
    if ( c=='}')
      break;
    else {
      in.putback(c);
      T tmp;
      in>>tmp;
      v.push_back(tmp);
    }
  }
  while(true);
  return in;
}

template <class T>
inline
bool operator==(Vec<T> const & v1 , safeVec<T> const & v2) {
  if (v1.size() != v2.size()) return false;
  for (uint i=0;i<v1.size();i++)
    if (v1[i] != v2[i]) return false;

  return true;
}

template <class T>
inline
bool operator!=(Vec<T> const & v1 , safeVec<T> const & v2) {
  return (! operator==(v1,v2));
}

// template <class T>
// inline
// safeVec<T> operator +(Vec<T> const & v1 , safeVec<T> const & v2) {
//   assert (v1.size() == v2.size());
//   safeVec<T> res(v1.size());
//   safeVec<T>::iterator r=res.begin();
//   safeVec<T>::const_iterator p2=v2.begin();
//   for(Vec<T>::const_iterator p1=v1.begin();p1 != v1.end();p1++,p2++,r++)
//     *r = *p1 + *p2;
//   return res;
// }

// template <class T>
// safeVec<T> & operator += (Vec<T> & v1 , safeVec<T> const & v2) {
//   assert (v1.size() == v2.size());
//   safeVec<T>::const_iterator p2=v2.begin();
//   for(Vec<T>::iterator p1=v1.begin() ;p1 != v1.end();p1++,p2++)
//     *p1 = *p1 + *p2;
//   return v1;
// }

/////////////////////////////


/* template<class T> class ValBasicIterator_tmpl { */
/*  public: */
/*   virtual bool HasNext()  const = 0; */
/*   virtual T const & Next()  = 0; */
/*   virtual ~ValBasicIterator_tmpl(){}; */
/* }; */

template<class T> class ValBasicIterator_tmpl {
 public:
  virtual bool HasNext()  const = 0;
  virtual T  Next()  = 0;
  virtual ~ValBasicIterator_tmpl(){};
};


/* typedef uint  hash_val; */

/* template <class T> */
/* class HashFunction_tmpl { */
/*  public: */
/*   virtual hash_val HashCode(T const & ) = 0; */
/*   virtual ~HashFunction_tmpl<T>() {}; */
/* }; */

/* template<class KmerID> */
/* class KmerIDHashFunction_cl : public HashFunction_tmpl<KmerID> { */
/*   virtual hash_val HashCode(KmerID const & kmerID) { return kmerID.first*kmerID.second;} */
/* }; */

template <class T>
class Cloneable {
 public:
  virtual T * clone() const = 0;
  virtual ~Cloneable() {};
};






/////////////////////// typedef definitions //////////////////////////////////////

union GeneIDUnion {
  int i;
  long double d;
};

typedef GeneIDUnion GeneID;

bool operator< (const GeneIDUnion &  gid1 , const GeneIDUnion & gid2);

ostream & operator<<(ostream & os, const GeneIDUnion &  gid);


/* union Position_cl { */
/*   int i; */
/*   long double d; */
/* }; */
/* typedef Position_cl kpos; */

typedef int kpos;

//#define  Vec vector ;


//typedef RCVector_tmpl<kpos> posVec;
/* typedef RCPtr_tmpl< RCVector_tmpl<kpos> > posVec_sptr; */
typedef safeVec<kpos> posVec;

//#define posVec_sptr RCPtr< safeVec<kpos> > ;




typedef int mint;
typedef safeVec<mint> VecI;


typedef string InvalidArgumentException;

typedef safeVec< safeVec<Vec<long double> > > long double3D ;
typedef safeVec< safeVec<long double> > long double2D;
typedef safeVec< safeVec<int> > int2D;
typedef safeVec< safeVec<long double> > long double2D;
typedef safeVec< safeVec<string> > string2D;

typedef bool base_val;
typedef safeVec< safeVec<base_val> > val2D; 
typedef safeVec<base_val> val1D; 


typedef unsigned char tCode;
typedef safeVec< unsigned char > codeVec;


typedef LibB_AssignmentVec AssignmentVec;
typedef LibB_PosVec  PosVec;



/* #define CODE_SIZE  5; */

//////////////// DNA sequence related definitions and functions ///////////////////////////////

/* ostream & operator<<(ostream & os,  unsigned char ); */

const char letters[] = {'A','C','G','T','N','?'};
const string lettersS[] = {"A","C","G","T","N","?"};

enum counts_type{GENE_COUNTS,TOTAL_COUNTS};
enum scoreType{ML,HG,HGtML,MLtHG};

string operator + (string s,long double d);
string operator + (string s,int i);
string operator + (string s,unsigned char  c);

/* template<class T1, class T2> */
/*   string operator + (string s,pair<T1,T2> const & p) { */
/*   s += "< "+ p.first +" , " + p.second + " >"; */
/*   return s; */
/* } */
void Err(string const & s);
tCode GetCharCode(char c);
/** Hamming disttance between 2 DNA sequence) */
int GetHammDist(string const & s1,string const & s2);


/**
 * score a subsequence according to a given pssm.
 */
long double
ScoreByPSSM(string const & seq,int index ,long double2D const & pssm);
long double
ScoreByPSSM(string const & seq,int index ,Vec<tMultinomial> const & pssm);


string
ReverseSeq(string const & s);
void
ReverseSeq(string const & s ,string & sRev );

void
ReverseCode(codeVec const & pattCode , codeVec & rCode);


codeVec GetCode(string const & s);
string GetSeqeunce(codeVec const &  code);

bool IsMissingVal(tCode val);

#endif
