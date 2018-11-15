// @(#)root/vmc:$Id$
// Authors: Benedikt Volkel 30/10/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TGeoMCBranchArrayContainer.h"
#include "TGeoManager.h"
#include "TGeoBranchArray.h"
#include "TError.h"

ClassImp(TGeoMCBranchArrayContainer);

TGeoMCBranchArrayContainer::TGeoMCBranchArrayContainer()
  : fCacheSize(0), fInitCacheSize(0), fNextIndex(0), fCurrentIndex(-1),
    fMaxLevels(0), fIsInitialized(kFALSE)
{
}

TGeoMCBranchArrayContainer::~TGeoMCBranchArrayContainer()
{
  ResetCache();
}

void TGeoMCBranchArrayContainer::Initialize(Int_t maxLevels, Int_t size)
{
  fCacheSize = size;
  fMaxLevels = maxLevels;
  if(fIsInitialized) {
    ResetCache();
  }
  // All indices are free at this point.
  fIsIndexFree.resize(fCacheSize, kTRUE);
  // Reserve enough space so all indices could go here.
  fFreeIndices.reserve(fCacheSize);
  // Resize and fill the cache.
  fCache.resize(fCacheSize);
  for(UInt_t i = 0; i < fCacheSize; i++) {
    fCache[i] = TGeoBranchArray::MakeInstance(fMaxLevels);
    fCache[i]->SetUniqueID(i);
  }
  fIsInitialized = kTRUE;
}

void TGeoMCBranchArrayContainer::InitializeFromGeoManager(TGeoManager* man, Int_t size)
{
  Initialize(man->GetMaxLevels(), size);
}

void TGeoMCBranchArrayContainer::ResetCache()
{
  // Empty cache.
  for(auto& c : fCache) {
    if(c) {
      delete c;
    }
  }
  fCache.clear();
  fIsIndexFree.clear();
  fFreeIndices.clear();
  // Set to initial values specified by the user previously.
  fNextIndex = 0;
  fCurrentIndex = -1;
  fIsInitialized = kFALSE;
}

TGeoBranchArray* TGeoMCBranchArrayContainer::GetNewGeoState(Int_t& index)
{
  //Info("GetNewGeoState", "Cache size: %i", fCacheSize);
  // Check for free index
  if(!fFreeIndices.empty()) {
    fCurrentIndex = fFreeIndices.back();
    // Pop if free
    fFreeIndices.pop_back();
    // Lock this index
    fIsIndexFree[fCurrentIndex] = kFALSE;
    //Info("GetNewGeoState", "New GeoStateIndex (old): %i", fCurrentIndex);
    index = fCurrentIndex;
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
    for(UInt_t i = fCacheSize; i < 2 * fCacheSize; i++) {
      fCache[i] = TGeoBranchArray::MakeInstance(fMaxLevels);
      fCache[i]->SetUniqueID(i);
    }
    // Update cache size.
    fCacheSize *= 2;
  }
  fCurrentIndex = fNextIndex++;
  // Lock this index.
  fIsIndexFree[fCurrentIndex] = kFALSE;
  //Info("GetNewGeoState", "New GeoStateIndex: %i", fCurrentIndex);
  index = fCurrentIndex;
  return fCache[fCurrentIndex];
}

TGeoBranchArray const * TGeoMCBranchArrayContainer::GetGeoState(UInt_t id)
{
  if(id >= fCacheSize) {
    // Not a geo state managed by the TGeoMCBranchArrayContainer
    Fatal("GetGeoState", "ID %u is not an index referring to TGeoBranchArray " \
                         "managed by this TGeoMCBranchArrayContainer", id);
  }
  // NOTE For now do cross check whether this is a locked and managed index.
  if(fIsIndexFree[id]) {
    Fatal("GetGeoState", "Id %u refers to an unused geo state", id);
  }
  return fCache[id];
}

void TGeoMCBranchArrayContainer::FreeGeoState(UInt_t id)
{
  if(id >= fCacheSize) {
    return;
  }
  // Unlock this index so it is free for later use.
  if(!fIsIndexFree[id]) {
    // Don't need to delete, TGeoBranchArray can be reused.
    // Free this index
    fIsIndexFree[id] = kTRUE;
    fFreeIndices.push_back(id);
  }
}

void TGeoMCBranchArrayContainer::FreeGeoStates()
{
  for(UInt_t i = 0; i < fCacheSize; i++) {
    FreeGeoState(i);
  }
}
