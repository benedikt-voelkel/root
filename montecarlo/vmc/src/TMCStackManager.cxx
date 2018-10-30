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

#include "TMCStackManager.h"
#include "TMCQueue.h"
#include "TVirtualMCStack.h"
#include "TVirtualMC.h"
#include "TTrack.h"
#include "TGeoCacheManual.h"
#include "TVector3.h"
#include "TError.h"
#include "TGeoManager.h"

/** \class TMCStackManager
    \ingroup vmc

Interface to a user defined particles stack.
*/

ClassImp(TMCStackManager);


TMCThreadLocal TMCStackManager* TMCStackManager::fgInstance = 0;

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TMCStackManager::TMCStackManager()
{
   if (fgInstance) {
      Fatal("TMCStackManager",
            "Attempt to create two instances of singleton.");
   }

   fGlobalStack = nullptr;
   fCurrentTrack = nullptr;
   fCurrentTrackID = -1;
   fCurrentPosition = TLorentzVector();
   fCurrentMomentum = TLorentzVector();
   fCurrentNavigator = nullptr;
   fTargetMCCached = nullptr;
   fGeoStateCached = nullptr;
   fGeoStateCache = TGeoCacheManual::Instance();
   fGeoStateCache->Initialize();
   fTrackTransportStatus.clear();
   fgInstance = this;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TMCStackManager::~TMCStackManager()
{
  // Nothing to do at the moment
}

////////////////////////////////////////////////////////////////////////////////
///
/// Static access method
///

TMCStackManager* TMCStackManager::Instance()
{
  if(!fgInstance) {
    new TMCStackManager();
  }
  return fgInstance;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Wrapper around TVirtualMCStack::PushTrack
///

void TMCStackManager::PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                                Double_t px, Double_t py, Double_t pz,
                                Double_t e, Double_t vx, Double_t vy,
                                Double_t vz, Double_t tof, Double_t polx,
                                Double_t poly, Double_t polz,
                                Int_t geoStateIndex, TMCProcess mech,
                                Int_t& ntr, Double_t weight, Int_t is)
{
  // Needs not to be done anymore since it will be forwarded to the queue of the
  // responsible engine
  // \todo If a track is pushed while transporting directly forward it to a queue, set toBeDone=0
  fGlobalStack->PushTrack(0, parent, pdg, px, py, pz, e, vx, vy, vz, tof, polx,
                          poly, polz, mech, ntr, weight, is);

  // \note \todo That is a workaround to set the track ID given the current interfaces
  // of TVirtualMCStack. In the future the TVirtualMCStack should be responsible
  // for doing that
  Int_t trackNumberCache = fGlobalStack->GetCurrentTrackNumber();
  fGlobalStack->SetCurrentTrack(ntr);
  TTrack* track = fGlobalStack->GetCurrentTrack();
  track->Id(ntr);
  // Set the geometry state index associated with this track
  track->GeoStateIndex(geoStateIndex);
  // Reset to actual current track
  fGlobalStack->SetCurrentTrack(trackNumberCache);
  if(ntr >= fTrackTransportStatus.size()) {
    fTrackTransportStatus.resize(2*ntr + 1, ETrackTransportStatus::kFinished);
  }

  // /Info("PushTrack", "Pushed track with VMC id %i", ntr);
  // If track is flagged toBeDone it needs to be forwarded to the queue of the
  // responsible engine
  if(toBeDone) {
    fTrackTransportStatus[ntr] = ETrackTransportStatus::kNew;
    /// \todo Make that more efficient, maybe already create a particle pointer here?!
    PushToQueue(track);
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Find the right queue to push this track to and push it
///

void TMCStackManager::PushToQueue(TTrack* track)
{
  //Info("PushToQueue", "Push track %i to queue.", track->Id());
  TVirtualMC* mc = nullptr;
  // Call function defined by the user to decide where to push a track to
  fSpecifyEngineForTrack(track, mc);
  if(!mc) {
    Fatal("PushToQueue", "No engine specified");
  }
  mc->GetQueue()->PushTrack(track);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Total number of tracks
///

Int_t TMCStackManager::GetNtrack() const
{
  return fGlobalStack->GetNtrack();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Total number of primary tracks
///

Int_t TMCStackManager::GetNprimary() const
{
  return fGlobalStack->GetNprimary();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Current track
///

TTrack* TMCStackManager::GetCurrentTrack() const
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

Int_t TMCStackManager::GetCurrentTrackNumber() const
{
  return fGlobalStack->GetCurrentTrackNumber();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Number of the parent of the current track
///

Int_t TMCStackManager::GetCurrentParentTrackNumber() const
{
  return fGlobalStack->GetCurrentParentTrackNumber();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return the transport status of the current track
///

ETrackTransportStatus TMCStackManager::GetTrackTransportStatus() const
{
  return fTrackTransportStatus[fCurrentTrack->Id()];
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the current track id from the outside and forward this to the
/// user's TVirtualMCStack
///

void TMCStackManager::SetCurrentTrack(Int_t trackID)
{
  fCurrentTrackID = trackID;
  fGlobalStack->SetCurrentTrack(trackID);
  fCurrentTrack = fGlobalStack->GetCurrentTrack();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the function called in stepping to check whether a track needs to
/// be moved
///

void TMCStackManager::RegisterSuggestTrackForMoving(
                            std::function<void(TVirtualMC*, TVirtualMC*&)> f)
{
  fSuggestTrackForMoving = f;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the function called to decide to which queue a primary is pushed
///
void TMCStackManager::RegisterSpecifyEngineForTrack(
                             std::function<void(TTrack*, TVirtualMC*&)> f)
{
  fSpecifyEngineForTrack = f;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Assign a queue for a given engine
///

void TMCStackManager::SetQueue(TVirtualMC* mc)
{
  // Warn and delete if there is already a queue set for this engine
  if(mc->GetQueue()) {
    Warning("SetQueue", "There is already a queue for MC %s which will now be replaced and deleted", mc->GetName());
    delete mc->GetQueue();
  }
  // Set queue
  mc->SetQueue(new TMCQueue());
}

////////////////////////////////////////////////////////////////////////////////
///
/// Register the user TVirtualMCStack
///

void TMCStackManager::RegisterStack(TVirtualMCStack* stack)
{
  if(fGlobalStack) {
    Warning("RegisterStack", "There is already a stack which will now be replaced and deleted");
    delete fGlobalStack;
  }
  fGlobalStack = stack;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Check whether the user's criteria call for moving the track from the
/// current engine
///

void TMCStackManager::SuggestTrackForMoving(TVirtualMC* currentMC)
{
  // Start off with nullptr.
  fTargetMCCached = nullptr;
  // Ask the user implemented lambda what the target engine for this track
  // should be.
  fSuggestTrackForMoving(currentMC, fTargetMCCached);
  // Move track if engine has changed.
  //if(targetEngine && currentMC != targetEngine) {
  if(fTargetMCCached) {
    MoveTrack(currentMC, fTargetMCCached->GetQueue());
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Move track from a given engine to a target queue of another engine
///

void TMCStackManager::MoveTrack(TVirtualMC* currentMC, TMCQueue* targetQueue)
{
  // Extract position and momentum from the current engine and update current
  // track.
  currentMC->TrackPosition(fCurrentPosition);
  currentMC->TrackMomentum(fCurrentMomentum);
  fCurrentTrack->AddPositionMomentum(fCurrentPosition, fCurrentMomentum);

  // Get an idle geometry state cache and set from current navigator.
  fGeoStateCached = fGeoStateCache->GetNewGeoState();
  fGeoStateCached->InitFromNavigator(fCurrentNavigator);
  // Connect ID of this geometry state to the current track.
  fCurrentTrack->GeoStateIndex(fGeoStateCached->GetUniqueID());

  // Push current track to queue of next responsible engine...
  targetQueue->PushTrack(fCurrentTrack);
  // ..., stop this track and...
  currentMC->StopTrack();
  // ...update its transport status. This is mainly used to control calls of
  // TVirtualMCApplication::PreTrack() and ::PostTrack().
  fTrackTransportStatus[fCurrentTrack->Id()] = ETrackTransportStatus::kProcessing;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Clear the TVirtualMCStack
///
void TMCStackManager::ResetStack()
{
  if(fGlobalStack) {
    fGlobalStack->ResetStack();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Print status of stack and queues
///

void TMCStackManager::Print() const
{
  Info("Print", "Status of stacks and queues");
  std::cout << "\t#tracks: " << fGlobalStack->GetNtrack() << std::endl;
}
