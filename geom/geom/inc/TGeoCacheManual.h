// @(#)root/eg:$Id$
// Author: Rene Brun , Federico Carminati  26/04/99

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

#ifndef ROOT_TGeoCacheManual
#define ROOT_TGeoCacheManual

#include <vector>

#include "Rtypes.h"

class TGeoBranchArray;


class TGeoCacheManual
{
  public:
    ~TGeoCacheManual();
    /// Get pointer to singleton instance
    static TGeoCacheManual* Instance();

    /// Do initialization steps
    void Initialize(Int_t initialCapacity = 8, Int_t maxLevel = 1);
    /// Clear the internal cache
    void ClearCache();

    /// Get a TGeoBranchArray to set to current geo state.
    TGeoBranchArray* GetNewGeoState();
    /// Get a TGeoBranchArray to read the current state from.
    const TGeoBranchArray* GetGeoState(Int_t id);


  private:
    TGeoCacheManual();
    TGeoCacheManual(const TGeoCacheManual&);

  private:
    /// Pointer to singleton instance
    static TGeoCacheManual* fgInstance;

    /// Vector of navigator interfaces
    /// Cache states via TGeoBranchArray
    std::vector<TGeoBranchArray*> fCache;
    /// Size of the cache
    UInt_t fCacheSize;
    /// Next free index
    Int_t fNextIndex;
    /// Memeber to cache an index internally
    Int_t fCurrentIndex;
    /// Maximum level of node array inside a chached state.
    Int_t fMaxLevels;
    /// Provide indices in fCachedStates which are already popped and can be
    /// re-populated again.
    std::vector<Int_t> fFreeIndices;
    /// Flags to (un)lock indices
    std::vector<Bool_t> fIsIndexFree;
    /// Constant default number of levels
    static constexpr Int_t DEFAULT_MAX_LEVELS = 100;

};


#endif /* ROOT_TGeoCacheManual */
