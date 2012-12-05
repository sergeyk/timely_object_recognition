/*
 * cofw_ptr - simple copy-on-first-write pointer.
 *
 * Once the pointer is copied, ALL subsequent accesses
 * to the data (non-const and const) will use the copy.
 *
 */
#ifndef COFW_PTR_H
#define COFW_PTR_H

template <class X>
class cofw_ptr {
public:

  //Ctor 1
  /* ASSUMES that p will always remain in scope for the lifetime
     of this cofw_ptr (and all cofw_ptrs copied from this one): */
  explicit cofw_ptr(const X* p = NULL) throw()
    : _originalPtr(p), _copiedPtr(NULL), _deleteCopiedPtr(true) {}

  //Ctor 2
  /* This ctor allows the cofw_ptr to be initialized with the copy, so
     that no future duplications are necessary.
     ASSUMES that cp will always remain in scope for the lifetime
     of this cofw_ptr (and all cofw_ptrs copied from this one): */
  explicit cofw_ptr(X* cp, bool DONT_COPY) throw()
    : _originalPtr(NULL), _copiedPtr(cp), _deleteCopiedPtr(false) {}

  //Ctor 3
  /* ASSUMES that cp will always remain in scope for the lifetime
     of this cofw_ptr (and all cofw_ptrs copied from this one): */
  cofw_ptr(const cofw_ptr& cp) throw() {
    this->copyFrom(cp);
  }

  ~cofw_ptr() {
    this->deleteData();
  }

  cofw_ptr& operator=(const cofw_ptr& cp) {
    if (this != &cp) {
      this->deleteData();
      this->copyFrom(cp);
    }
    return *this;
  }

  //Const accessors:
  const X& operator*() const throw() {
    return *(this->pointerToUse());
  }

  const X* get() const throw() {
    return this->pointerToUse();
  }
  
  const X* operator->() const throw() {
    return this->pointerToUse();
  }

  //Copy-on-first-write accessors:
  X& operator*() {
    ensureCopy();
    return *_copiedPtr;
  }

  X* get() {
    ensureCopy();
    return _copiedPtr;
  }
  
  X* operator->() {
    ensureCopy();
    return _copiedPtr;
  }

  void setDeleteCopiedPointer() {
    if (builtByCtor2()) //otherwise, irrelevant
      _deleteCopiedPtr = true;
  }


private:
  const X* _originalPtr;
  X* _copiedPtr;
  bool _deleteCopiedPtr;


  const X* pointerToUse() const {
    if (_copiedPtr != NULL)
      return _copiedPtr;
    
    return _originalPtr;
  }
  
  void ensureCopy() {
    if (_copiedPtr != NULL)
      return;

    if (_originalPtr == NULL) //nothing to copy
      return;

    //create a copy of what _originalPtr points to:
    _copiedPtr = _originalPtr->clone();
  }

  void deleteData() {
    if (_deleteCopiedPtr && _copiedPtr != NULL) {
      delete _copiedPtr;
      _copiedPtr = NULL;
    }
  }

  bool builtByCtor2() const {//true iff _copiedPtr was created independently of _originalPtr
    return (_originalPtr == NULL && _copiedPtr != NULL);
  }

  void copyFrom(const cofw_ptr& cp) {
    if (cp.builtByCtor2()) {//cp built by ctor 2
      _originalPtr = NULL;
      _copiedPtr = cp._copiedPtr;
      _deleteCopiedPtr = false; //cp responsible for deletion
    }
    else {//cp built by ctor 1
      _originalPtr = cp.pointerToUse();
      _copiedPtr = NULL;
      _deleteCopiedPtr = true;
    }
  }
  
};

#endif // COFW_PTR_H
