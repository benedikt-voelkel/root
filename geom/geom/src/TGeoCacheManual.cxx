// @(#)root/vmc:$Id$
// Authors: Benedikt Volkel 06/08/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TGeoCacheManual.h"
#include "TGeoBranchArray.h"
#include "TError.h"

TGeoCacheManual* TGeoCacheManual::fgInstance = nullptr;

TGeoCacheManual::TGeoCacheManual()
  : fCacheSize(0), fNextIndex(0), fCurrentIndex(-1), fMaxLevels(0)
{
  fCache.clear();
  fFreeIndices.clear();
  fIsIndexFree.clear();
}

TGeoCacheManual::~TGeoCacheManual()
{
  ClearCache();
  fgInstance = nullptr;
}

TGeoCacheManual* TGeoCacheManual::Instance()
{
  if(!fgInstance) {
    fgInstance = new TGeoCacheManual();
  }
  return fgInstance;
}

void TGeoCacheManual::Initialize(Int_t initialCapacity, Int_t maxLevel)
{
  if(fCacheSize > 0) {
    Warning("Initialize", "Cache is already initialised. Will be cleared and re-initialised");
    ClearCache();
  }
  fCacheSize = initialCapacity;
  fMaxLevels = (maxLevel > 1) ? maxLevel : DEFAULT_MAX_LEVELS;
  // All indices are free at this point.
  fIsIndexFree.resize(fCacheSize, kTRUE);
  // Reserve enough space so all indices could go here.
  fFreeIndices.reserve(fCacheSize);
  // Resize and fill the cache.
  fCache.resize(fCacheSize);
  for(Int_t i = 0; i < fCacheSize; i++) {
    fCache[i] = TGeoBranchArray::MakeInstance(fMaxLevels);
    fCache[i]->SetUniqueID(i);
  }

}

void TGeoCacheManual::ClearCache()
{
  // Empty cache.
  if(!fCache.empty()) {
    for(auto& c : fCache) {
      delete c;
    }
  }
  fCache.clear();
  fIsIndexFree.clear();
  fFreeIndices.clear();
  fCacheSize = 0;
  fMaxLevels = 0;
  fNextIndex = 0;
}

TGeoBranchArray* TGeoCacheManual::GetNewGeoState()
{
  // Check for free index
  if(!fFreeIndices.empty()) {
    fCurrentIndex = fFreeIndices.back();
    // Pop if free
    fFreeIndices.pop_back();
    // Lock this index
    fIsIndexFree[fCurrentIndex] = kFALSE;
    return fCache[fCurrentIndex];
  }
  // Do controlled resize and already instantiate new TGeoBranchArrays
  if(fNextIndex == fCacheSize) {
    // Reserve enough space so all new indices could go here.
    fFreeIndices.reserve(2*fCacheSize);
    // Unlock all new indices.
    fIsIndexFree.resize(2*fCacheSize, kTRUE);
    // Add new objects to cache.
    fCache.resize(2*fCacheSize, nullptr);
    for(Int_t i = fCacheSize; i < 2*fCacheSize; i++) {
      fCache[i] = TGeoBranchArray::MakeInstance(fMaxLevels);
      fCache[i]->SetUniqueID(i);
    }
    // Update cache size.
    fCacheSize *= 2;
  }
  fCurrentIndex = fNextIndex++;
  // Lock this index.
  fIsIndexFree[fCurrentIndex] = kFALSE;
  return fCache[fCurrentIndex];
}

const TGeoBranchArray* TGeoCacheManual::GetGeoState(Int_t id)
{
  if(id < 0 || id >= fCacheSize) {
    // Not a geo state managed by the TGeoCacheManual
    Fatal("GetGeoState", "Not a geo state managed by the TGeoCacheManual");
    return nullptr;
  }
  // Implicitly unlock this index so it is free for later use.
  if(!fIsIndexFree[id]) {
    fIsIndexFree[id] = kTRUE;
    fFreeIndices.push_back(id);
  }
  return fCache[id];
}
