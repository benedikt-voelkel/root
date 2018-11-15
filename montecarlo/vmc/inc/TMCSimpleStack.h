// @(#)root/vmc:$Id$
// Authors: Benedikt Volkel, 14/11/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TMCSimpleStack
#define ROOT_TMCSimpleStack

#include "Rtypes.h"

// Class TMCSimpleStack
// ---------------------
// Small container to manage stacks for different engines. It stacks tracks and
// counts primaries.
//
template <class T>
class TMCSimpleStack
{
  public:
    /// Default constructor
    TMCSimpleStack()
      : fElements(new T[1]), fNElements(0), fMemorySize(1),
        fMemorySizeInit(1), fIsOwner(kTRUE)
    {}
    /// Constructor reserving memory
    TMCSimpleStack(Int_t reservedSize)
      : fElements(new T[reservedSize]), fNElements(0),
        fMemorySize(reservedSize), fMemorySizeInit(reservedSize),
        fIsOwner(kTRUE)
    {}
    /// Construct from data but leave the destruction to the user
    TMCSimpleStack(const T* data, Int_t size, Bool_t isOwner)
      : fElements(data), fNElements(size), fMemorySize(size), fIsOwner(isOwner)
    {}



    /// Destructor
    ~TMCSimpleStack()
    {
      // Only delete data if it was allocated by this stack
      if(fIsOwner) {
        delete[] fElements;
      }
    }

    /// Push a track pointer
    void Push(const T element)
    {
      if(fNElements == fMemorySize) {
        // If maximum meory is reached and not allocated by this object, fail.
        assert(fIsOwner && "Memory not allocated by Vector itself and " \
                           "maximum memory size reached");
        // Double the memory size, shift elements
        fMemorySize = fMemorySize<<1;
        T* newElements = new T[fMemorySize];
        for(Int_t i = 0; i < fNElements; i++) {
          newElements[i] = fElements[i];
        }
        delete[] fElements;
        fElements = newElements;
      }
      // Append the element
      fElements[fNElements] = element;
      fNElements++;
    }
    /// Get number of all tracks
    Int_t Size() const
    {
      return fNElements;
    }
    /// Pop any track (at index)
    T& Pop()
    {
      // Check if stack is empty
      assert(fNElements == 0 && "No elements on the stack");
      return fElements[--fNElements];
    }
    /// Pop first element matching criteria given by passed function. This
    /// function must take an object of type T as its only argument and it must
    /// return a boolean value.
    template <typename Function>
    T& Pop(Function f, T& defaultValue)
    {
      for(Int_t i = fNElements - 1; i >= 0; i--) {
        // If element was found, swap it with the last one and then just pop.
        // \note Use std::move? Because if there is a complex object this could
        // mean expensive copies.
        if(f(fElements[i])) {
          T element = fElements[i];
          for(Int_t j = i; j < fNElements - 1; j++) {
            fElements[j] = fElements[j+1];
          }
          fElements[fNElements-1] = element;
          return Pop();
        }
      }
      return defaultValue;
    }
    /// Reset this stack
    void Reset()
    {
      assert(fIsOwner && "Cannot reset since not the owner of the memory");
      fNElements = 0;
      fMemorySize = fMemorySizeInit;
      delete[] fElements;
    }

  private:
    /// Pointer of elements
    T* fElements;
    /// Total number of elements
    Int_t fNElements;
    /// Size in memory
    Int_t fMemorySize;
    /// Size in memory kept constant to reset
    Int_t fMemorySizeInit;
    /// Flag if allocated
    Bool_t fIsOwner;

    ClassDef(TMCSimpleStack,1)
};

#endif /* ROOT_TMCSimpleStack */
