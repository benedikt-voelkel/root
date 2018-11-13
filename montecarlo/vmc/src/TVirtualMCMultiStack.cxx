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
#include "TMCQueue.h"
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
{
   fCurrentTrack = nullptr;
   fCurrentTrackID = -1;
   fCurrentPosition = TLorentzVector();
   fCurrentMomentum = TLorentzVector();
   fCurrentNavigator = nullptr;
   fTargetMCCached = nullptr;
   fCachedGeoState = nullptr;
   fGeoStateCache = TGeoStateCache::Instance();
   fGeoStateCache->Initialize();
   fTrackTransportStatus.clear();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TVirtualMCMultiStack::~TVirtualMCMultiStack()
{
  // Nothing to do at the moment
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
                                TMCProcess mech, Int_t& ntr, Double_t weight,
                                Int_t is)
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

   // Check whether this track needs to be done
   if(toBeDone) {
     // Provide a transport status for this track and push to correct TMCQueue.
     InsertTrackTransportStatus(ntr, ETrackTransportStatus::kNew);
     PushToQueue(track);
   } else {
     InsertTrackTransportStatus(ntr, ETrackTransportStatus::kProcessing);
   }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop the next track
///

TParticle* TVirtualMCMultiStack::PopNextTrack(Int_t&  itrack)
{
  TTrack* track = fCurrentQueue->PopNextTrack();
  itrack = track->Id();
  return track;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop the next track with geoStateIndex
///

TParticle* TVirtualMCMultiStack::PopNextTrack(Int_t&  itrack,
                                              Int_t& geoStateIndex)
{
  TTrack* track = fCurrentQueue->PopNextTrack();
  itrack = track->Id();
  geoStateIndex = track->GeoStateIndex();
  return track;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Find the right queue to push this track to and push it
///

void TVirtualMCMultiStack::PushToQueue(TTrack* track)
{
  //Info("PushToQueue", "Push track %i to queue.", track->Id());
  TVirtualMC* mc = nullptr;
  // Call function defined by the user to decide where to push a track to
  fSpecifyEngineForTrack(track, mc);
  if(!mc) {
    Fatal("PushToQueue", "No engine specified");
  }
  fVMCToQueueMap[mc].PushTrack(track);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Insert a new transport status for a given track ID
///

void TVirtualMCMultiStack::InsertTrackTransportStatus(Int_t trackID,
                                                   ETrackTransportStatus status)
{
  if(trackID > fTrackTransportStatus.size()) {
    fTrackTransportStatus.resize(fTrackTransportStatus.size() + 1, ETrackTransportStatus::kFinished);
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
  const auto it = fVMCToQueueMap.find(mc);
  if(it != fVMCToQueueMap.end() && it->second.GetNtrack() > 0) {
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
  SetCurrentUserTrack(trackID);
  fCurrentTrackID = trackID;
  fCurrentTrack = GetCurrentUserTrack();
  fCurrentParentTrackID = fCurrentTrack->GetFirstMother();
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
/// Set the function called to decide to which queue a primary is pushed
///

void TVirtualMCMultiStack::RegisterSpecifyEngineForTrack(
                             std::function<void(TTrack*, TVirtualMC*&)> f)
{
  fSpecifyEngineForTrack = f;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Assign a queue for a given engine
///

void TVirtualMCMultiStack::SetQueue(TVirtualMC* mc)
{
  // Set the current TMCQueue
  fCurrentQueue = &fVMCToQueueMap[mc];
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
  //if(targetEngine && currentMC != targetEngine) {
  if(fTargetMCCached) {
    currentMC->TrackPosition(fCurrentPosition);
    currentMC->TrackMomentum(fCurrentMomentum);
    fCurrentTrack->AddPositionMomentum(fCurrentPosition, fCurrentMomentum);

    // Get an idle geometry state cache and set from current navigator.
    fCachedGeoState = fGeoStateCache->GetNewGeoState();
    fCachedGeoState->InitFromNavigator(fCurrentNavigator);

    // Connect ID of this geometry state to the current track.
    fCurrentTrack->GeoStateIndex(fCachedGeoState->GetUniqueID());

    // Push current track to queue of next responsible engine...
    fVMCToQueueMap[fTargetMCCached].PushTrack(fCurrentTrack);
    // ..., stop this track and...
    currentMC->StopTrack();
    // ...update its transport status. This is mainly used to control calls of
    // TVirtualMCApplication::PreTrack() and ::PostTrack().
    fTrackTransportStatus[fCurrentTrack->Id()] = ETrackTransportStatus::kProcessing;
  }
}
