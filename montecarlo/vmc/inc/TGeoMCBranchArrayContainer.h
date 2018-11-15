// @(#)root/eg:$Id$
// Author: Benedikt Volkel  26/04/99

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TGeoCacheManual: Cache geometry states for further use              //
// \todo Needs to be able to provide interfaces for different           //
// navigators in a flexible manner (e.g. via lambdas???). So far only   //
// the TGeoNavigator is supported.                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TGeoMCBranchArrayContainer
#define ROOT_TGeoMCBranchArrayContainer

#include <vector>

#include "Rtypes.h"

class TGeoBranchArray;
class TGeoManager;

class TGeoMCBranchArrayContainer
{
  public:
    /// Default constructor
    TGeoMCBranchArrayContainer();
    /// Destructor
    ~TGeoMCBranchArrayContainer();

    /// Initialize manually specifying initial number of internal
    /// TGeoBranchArray objects
    void Initialize(Int_t maxlevels=100, Int_t size=8);
    /// Initialize from TGeoManager to extract maxlevels
    void InitializeFromGeoManager(TGeoManager* man, Int_t size=8);
    /// Clear the internal cache
    void ResetCache();

    /// Return the size/number of elements
    UInt_t Size() const;

    /// Get a TGeoBranchArray to set to current geo state.
    TGeoBranchArray* GetNewGeoState(Int_t& index);
    /// Get a TGeoBranchArray to read the current state from.
    TGeoBranchArray const * GetGeoState(UInt_t id);
    /// Pop end hence delete TGeoBranchArray associated to this id
    void FreeGeoState(UInt_t id);
    /// Free all geo states at once but keep the container size
    void FreeGeoStates();

  private:
    TGeoMCBranchArrayContainer(const TGeoMCBranchArrayContainer&);
    TGeoMCBranchArrayContainer& operator=(const TGeoMCBranchArrayContainer&);

  private:
    /// Cache states via TGeoBranchArray
    std::vector<TGeoBranchArray*> fCache;
    /// Size of the cache
    UInt_t fCacheSize;
    /// Size of the cache used initially in case of reset
    UInt_t fInitCacheSize;
    /// Next free index
    UInt_t fNextIndex;
    /// Memeber to cache an index internally
    UInt_t fCurrentIndex;
    /// Maximum level of node array inside a chached state.
    UInt_t fMaxLevels;
    /// Provide indices in fCachedStates which are already popped and can be
    /// re-populated again.
    std::vector<Int_t> fFreeIndices;
    /// Flags to (un)lock indices
    std::vector<Bool_t> fIsIndexFree;
    /// Flag if initialized
    Bool_t fIsInitialized;

    ClassDefNV(TGeoMCBranchArrayContainer,1)
};

#endif /* ROOT_TGeoMCBranchArrayContainer */
