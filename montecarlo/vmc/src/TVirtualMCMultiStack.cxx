// @(#)root/vmc:$Id$
// Author: Benedikt Volkel, 30/10/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

 #include <iostream>

#include "TVirtualMCMultiStack.h"
#include "TVirtualMC.h"
#include "TTrack.h"
#include "TGeoStateCache.h"
#include "TVector3.h"
#include "TError.h"
#include "TGeoManager.h"

/** \class TVirtualMCMultiStack
    \ingroup vmc

Interface to a user defined particles stack.
*/

ClassImp(TVirtualMCMultiStack);

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCMultiStack::TVirtualMCMultiStack()
  : TVirtualMCStack(),
    fTracks(new TObjArray(100)), fPrimaries(0), fCurrentTrack(nullptr),
    fCurrentTrackID(-1), fCurrentParentTrackID(-1),
    fCurrentPosition(TLorentzVector()), fCurrentMomentum(TLorentzVector()),
    fTargetMCCached(nullptr), fCachedGeoState(nullptr),
    fGeoStateCache(TGeoStateCache::Instance()), fCurrentNavigator(nullptr),
    fCurrentStack(nullptr), fCurrentNPrimaries(nullptr)
{
    fGeoStateCache->Initialize();
    fTrackTransportStatus.clear();
    fVMCToStackMap.clear();
    fVMCToPrimariesMap.clear();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TVirtualMCMultiStack::~TVirtualMCMultiStack()
{
  if(fCurrentNPrimaries) {
    delete fCurrentNPrimaries;
  }
  delete fTracks;

}


////////////////////////////////////////////////////////////////////////////////
///
/// Wrapper around TVirtualMCStack::PushTrack
///
/// Without geoStateIndex
void TVirtualMCMultiStack::PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                                     Double_t px, Double_t py, Double_t pz,
                                     Double_t e, Double_t vx, Double_t vy,
                                     Double_t vz, Double_t tof, Double_t polx,
                                     Double_t poly, Double_t polz,
                                     TMCProcess mech, Int_t& ntr,
                                     Double_t weight, Int_t is)
{
  // Just forward to PushTrack including a geoStateIndex
  PushTrack(toBeDone, parent, pdg, px, py, pz,e, vx, vy, vz, tof, polx, poly,
            polz, -1, mech, ntr, weight, is);
}
/// With geoStateIndex
void TVirtualMCMultiStack::PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                                     Double_t px, Double_t py, Double_t pz,
                                     Double_t e, Double_t vx, Double_t vy,
                                     Double_t vz, Double_t tof, Double_t polx,
                                     Double_t poly, Double_t polz,
                                     Int_t geoStateIndex, TMCProcess mech,
                                     Int_t& ntr, Double_t weight, Int_t is)
{
  // Call user implemented method
  TTrack* track = PushUserTrack(parent, pdg, px, py, pz, e, vx, vy, vz, tof,
                                 polx, poly, polz, geoStateIndex, mech, weight,
                                 is);
   // Set the track id (for backwards compatibility)
   ntr = track->Id();
   // If no parent, that's considered to be a primary
   if(!track->GetParent()) {
     fPrimaries++;
   }

   // Check whether this track needs to be done
   if(toBeDone) {
     // Provide a transport status for this track and push to correct stack.
     InsertTrackTransportStatus(ntr, ETrackTransportStatus::kNew);
     PushToInternalStack(track);
   } else {
     InsertTrackTransportStatus(ntr, ETrackTransportStatus::kProcessing);
   }
   // Sorted by ID so resize if more memory is required.
   fTracks->AddAtAndExpand(track, ntr);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop the next track
///

TParticle* TVirtualMCMultiStack::PopNextTrack(Int_t&  itrack)
{
  Int_t geoStateIndex = -1;
  return PopNextTrack(itrack, geoStateIndex);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop the next track with geoStateIndex
///

TParticle* TVirtualMCMultiStack::PopNextTrack(Int_t&  itrack,
                                              Int_t& geoStateIndex)
{
  if(fCurrentStack->Size() == 0) {
    return nullptr;
  }

  TTrack* track = fCurrentStack->Pop();
  itrack = track->Id();
  geoStateIndex = track->GeoStateIndex();
  // Decrement number of primaries if there is no parent
  if(!track->GetParent()) {
    *fCurrentNPrimaries--;
  }
  return track;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop i'th primar, that does not mean that this primariy also has ID==i
///

TParticle* TVirtualMCMultiStack::PopPrimaryForTracking(Int_t i)
{
  Int_t geoStateIndex = -1;
  Int_t itrack = -1;
  return PopPrimaryForTracking(i, itrack, geoStateIndex);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop i'th primar, that does not mean that this primariy also has ID==i.
/// including actual index and geoStateIndex
///

TParticle* TVirtualMCMultiStack::PopPrimaryForTracking(Int_t i, Int_t& itrack,
                                                       Int_t& geoStateIndex)
{
  // Completely ignore the index i, that is meaningless since the user does not
  // know how the stack is handled internally.
  // See comment in header TVirtualMCMultiStack.h
  if(fCurrentStack->Size() == 0) {
    itrack = -1;
    geoStateIndex = -1;
    return nullptr;
  }
  TTrack* track = nullptr;
  track = fCurrentStack->Pop([](const TTrack* t){
                                 if(!t->GetParent()) {
                                   return kTRUE;
                                 }
                                 return kFALSE;
                               }, track);
  // Checking should not be necessary since it's kept track of primaries.
  // \todo Test and remove check.
  if(track) {
    itrack = track->Id();
    geoStateIndex = track->GeoStateIndex();
    (*fCurrentNPrimaries)--;
  }
  return track;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get number of tracks which are only those to be tracked.
///

Int_t TVirtualMCMultiStack::GetNtrack() const
{
  return fCurrentStack->Size();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get the number of primaries in current stack
///

Int_t TVirtualMCMultiStack::GetNprimary() const
{
  return *fCurrentNPrimaries;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Find the right engine stack to push this track to and push it
///

void TVirtualMCMultiStack::PushToInternalStack(TTrack* track)
{
  TVirtualMC* mc = nullptr;
  // Call function defined by the user to decide where to push a track to
  fSpecifyEngineForTrack(track, mc);
  if(!mc) {
    Fatal("PushToInternalStack", "No engine specified");
  }
  fVMCToStackMap[mc].Push(track);
  // Imcrement number of primaries for this stack
  if(!track->GetParent()) {
    fVMCToPrimariesMap[mc]++;
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Insert a new transport status for a given track ID
///

void TVirtualMCMultiStack::InsertTrackTransportStatus(Int_t trackID,
                                                   ETrackTransportStatus status)
{
  // Resize to twice the size
  if(trackID > fTrackTransportStatus.size()) {
    fTrackTransportStatus.resize(2 * fTrackTransportStatus.size() + 1,
                                 ETrackTransportStatus::kFinished);
  }
  fTrackTransportStatus[trackID] = status;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Current track
///

TParticle* TVirtualMCMultiStack::GetCurrentTrack() const
{
  if(fCurrentTrack) {
    return fCurrentTrack;
  }
  Fatal("GetCurrentTrack", "There is no current track set");
  // \note Fatal will abort but without return statement we will most certainly
  // get a compiler warning...
  return fCurrentTrack;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Current track number
///

Int_t TVirtualMCMultiStack::GetCurrentTrackNumber() const
{
  return fCurrentTrackID;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Number of the parent of the current track
///

Int_t TVirtualMCMultiStack::GetCurrentParentTrackNumber() const
{
  return fCurrentParentTrackID;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return the transport status of the current track
///

ETrackTransportStatus TVirtualMCMultiStack::GetTrackTransportStatus() const
{
  return fTrackTransportStatus[fCurrentTrackID];
}

////////////////////////////////////////////////////////////////////////////////
///
/// Check if there are tracks for specific TVirtualMC
///

Bool_t TVirtualMCMultiStack::HasTracks(TVirtualMC* mc) const
{
  const auto it = fVMCToStackMap.find(mc);
  if(it != fVMCToStackMap.end() && it->second.Size() > 0) {
    return kTRUE;
  }
  return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the current track id from the outside and forward this to the
/// user's TVirtualMCStack
///

void TVirtualMCMultiStack::SetCurrentTrack(Int_t trackID)
{
  TTrack* track = dynamic_cast<TTrack*>(fTracks->At(trackID));
  if(track) {
    fCurrentTrackID = trackID;
    fCurrentTrack = track;
    fCurrentParentTrackID = fCurrentTrack->GetFirstMother();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the function called in stepping to check whether a track needs to
/// be moved
///

void TVirtualMCMultiStack::RegisterSuggestTrackForMoving(
                            std::function<void(TVirtualMC*, TVirtualMC*&)> f)
{
  fSuggestTrackForMoving = f;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the function called to decide to which stack a primary is pushed
///

void TVirtualMCMultiStack::RegisterSpecifyEngineForTrack(
                             std::function<void(TTrack*, TVirtualMC*&)> f)
{
  fSpecifyEngineForTrack = f;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Add a internal stack for an engine
///

void TVirtualMCMultiStack::AddEngineStack(TVirtualMC* mc)
{
  // Check if there is already a stack for this engine
  if(fVMCToStackMap.find(mc) == fVMCToStackMap.end()) {
    // \todo Fix initial size
    fVMCToStackMap.emplace(mc, 100);
  }
  mc->SetStack(this);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Add a stack for a given engine and set engine stack
///

void TVirtualMCMultiStack::SetEngineStack(TVirtualMC* mc)
{
  // Set the current stack and primary counter
  fCurrentStack = &fVMCToStackMap[mc];
  fCurrentNPrimaries = &fVMCToPrimariesMap[mc];
}

////////////////////////////////////////////////////////////////////////////////
///
/// Check whether the user's criteria call for moving the track from the
/// current engine
///

void TVirtualMCMultiStack::SuggestTrackForMoving(TVirtualMC* currentMC)
{
  // Start off with nullptr.
  fTargetMCCached = nullptr;
  // Ask the user implemented lambda what the target engine for this track
  // should be.
  fSuggestTrackForMoving(currentMC, fTargetMCCached);
  // Move track if engine has changed.
  if(fTargetMCCached && currentMC != fTargetMCCached) {
  // /if(fTargetMCCached) {
    currentMC->TrackPosition(fCurrentPosition);
    currentMC->TrackMomentum(fCurrentMomentum);
    fCurrentTrack->AddPositionMomentum(fCurrentPosition, fCurrentMomentum);

    // Get an idle geometry state cache and set from current navigator.
    fCachedGeoState = fGeoStateCache->GetNewGeoState();
    fCachedGeoState->InitFromNavigator(fCurrentNavigator);

    // Connect ID of this geometry state to the current track.
    fCurrentTrack->GeoStateIndex(fCachedGeoState->GetUniqueID());

    // Push current track to stack of next responsible engine...
    fVMCToStackMap[fTargetMCCached].Push(fCurrentTrack);
    // ...increment number of primaries
    if(!fCurrentTrack->GetParent()) {
      fVMCToPrimariesMap[fTargetMCCached]++;
    }
    // ..., stop this track and...
    currentMC->StopTrack();
    // ...update its transport status. This is mainly used to control calls of
    // TVirtualMCApplication::PreTrack() and ::PostTrack().
    fTrackTransportStatus[fCurrentTrack->Id()] = ETrackTransportStatus::kProcessing;
  }
}



void TVirtualMCMultiStack::ResetInternals()
{
  // Clear cached track pointers, user has to do actual de-allocation
  fTracks->Clear();
  fCurrentTrack = nullptr;
  fCurrentTrackID = -1;
  fCurrentParentTrackID = -1;
  fTrackTransportStatus.clear();
  for(auto& q : fVMCToStackMap) {
    q.second.Reset();
  }
  fCurrentStack = nullptr;
}
