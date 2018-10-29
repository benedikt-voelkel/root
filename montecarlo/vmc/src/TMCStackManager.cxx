// @(#)root/vmc:$Id$
// Author: Ivana Hrivnacova, 27/03/2002

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
#include "TVirtualMCApplication.h"
#include "TMCStateManager.h"
#include "TMCQueue.h"
#include "TVirtualMCStack.h"
#include "TVirtualMC.h"
#include "TTrack.h"
#include "TGeoCacheManual.h"
#include "TVector3.h"
#include "TError.h"
#include "TGeoManager.h"
#include "TGeoNode.h"

/** \class TMCStackManager
    \ingroup vmc

Interface to a user defined particles stack.
*/

ClassImp(TMCStackManager);


TMCThreadLocal TMCStackManager* TMCStackManager::fgInstance = 0;

////////////////////////////////////////////////////////////////////////////////
/// Default constructor

TMCStackManager::TMCStackManager()
{
   if (fgInstance) {
      Fatal("TMCManager",
            "Attempt to create two instances of singleton.");
   }
   fMCApplication = TVirtualMCApplication::Instance();
   if(!fMCApplication) {
     Fatal("TMCManager", "No user MC application is defined.");
   }
   fMasterStack = nullptr;
   fCurrentTrack = nullptr;
   fCurrentTrackID = -1;
   fLastTrackSuggestedForMoving = kFALSE;
   fMCStateManager = TMCStateManager::Instance();
   fCurrentPosition = TLorentzVector();
   fCurrentMomentum = TLorentzVector();
   fCurrentNavigator = nullptr;
   fTargetMCCached = nullptr;
   fGeoStateCached = nullptr;
   fGeoStateCache = TGeoCacheManual::Instance();
   fGeoStateCache->Initialize();
   fgInstance = this;
}

////////////////////////////////////////////////////////////////////////////////
/// Destructor

TMCStackManager::~TMCStackManager()
{
  // Nothing to do at the moment
}

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
  fMasterStack->PushTrack(0, parent, pdg, px, py, pz, e, vx, vy, vz, tof, polx,
                          poly, polz, mech, ntr, weight, is);

  // \note \todo That is a workaround to set the track ID given the current interfaces
  // of TVirtualMCStack. In the future the TVirtualMCStack should be responsible
  // for doing that
  Int_t trackNumberCache = fMasterStack->GetCurrentTrackNumber();
  fMasterStack->SetCurrentTrack(ntr);
  TTrack* track = fMasterStack->GetCurrentTrack();
  track->Id(ntr);
  // Set the geometry state index associated with this track
  track->GeoStateIndex(geoStateIndex);
  // Reset to actual current track
  fMasterStack->SetCurrentTrack(trackNumberCache);

  // /Info("PushTrack", "Pushed track with VMC id %i", ntr);
  // If track is flagged toBeDone it needs to be forwarded to the queue of the
  // responsible engine
  if(toBeDone) {
    /// \todo Make that more efficient, maybe already create a particle pointer here?!
    PushToQueue(track);
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Find the right queue to push this track to
///

void TMCStackManager::PushToQueue(TTrack* track)
{
  //Info("PushToQueue", "Push track %i to queue.", track->Id());
  TVirtualMC* mc = nullptr;
  // \todo Only rely on concurrent flag here?
  if(!fMCStateManager->GetConcurrentMode() || fMCEngines.size() == 1) {
    fMCEngines.front()->GetQueue()->PushTrack(track);
    return;
  }
  // Call function defined by the user to decide where to push a track to
  fSpecifyEngineForTrack(track, mc);
  if(!mc) {
    Fatal("PushToQueue", "No engine specified");
  }
  mc->GetQueue()->PushTrack(track);
}


void TMCStackManager::NotifyOnFinishedEvent()
{
  // Nothing to do at the moment
  //if(fStackExportMode == EStackExportMode::kPerEvent) {
    //Info("NotifyOnFinishedEvent","Export stack to ROOT file");
    //ExportStack();
  //}
}


////////////////////////////////////////////////////////////////////////////////
///
/// Forward tracks on fMasterStack to queues of registered engines
///

void TMCStackManager::InitializeQueuesWithPrimaries(Bool_t isNewEvent)
{
  /*if(isNewEvent) {
    NotifyOnFinishedEvent();
  }*/
  if(fMasterStack->GetNtrack() < 1) {
    Fatal("InitializeQueuesWithPrimaries", "No primaries found on user VMC stack");
  }
  //Info("InitializeQueuesWithPrimaries","Push primaries to queues");
  Int_t trackNumber;
  TTrack* track = nullptr;
  // If that point is reached, concurrent mode is enabled and the decision of
  // which queue to push the tracks to gets more complex
  while((track = fMasterStack->PopNextTrack(trackNumber))) {
    // \note \todo Need to set track number here since the TVirtualMCStack does not
    // provide this (yet)
    track->Id(trackNumber);
    // Also, since tracks might be pushed to the fMasterStack by the directly
    // by the user, this track cannot have a geoStateIndex. Set it to -1. In any
    // case for a real primary there wouldn't be an Initialized state at that
    // point, at least in general.
    track->GeoStateIndex(-1);
    PushToQueue(track);
  }
  // \todo Use information prvided by the TMCStateManager in the future
  // \note Also, setting this here means that
}

// So far a wrapper around the user stack method
Int_t TMCStackManager::GetNtrack() const
{
  return fMasterStack->GetNtrack();
}

// So far a wrapper around the user stack method
Int_t TMCStackManager::GetNprimary() const
{
  return fMasterStack->GetNprimary();
}

// So far a wrapper around the user stack method
Int_t TMCStackManager::GetCurrentTrackNumber() const
{
  return fMasterStack->GetCurrentTrackNumber();
}

// So far a wrapper around the user stack method
Int_t TMCStackManager::GetCurrentParentTrackNumber() const
{
  return fMasterStack->GetCurrentParentTrackNumber();
}

// So far a wrapper around the user stack method
void TMCStackManager::SetCurrentTrack(Int_t trackID)
{
  fCurrentTrackID = trackID;
  fMasterStack->SetCurrentTrack(trackID);
  fCurrentTrack = fMasterStack->GetCurrentTrack();
}

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
/// Check whether track and engine parameters meeting criteria inclusively
///

void TMCStackManager::SuggestTrackForMoving(TVirtualMC* currentMC)
{
  //-----------------------------------------------------------
  //-----------------------------------------------------------
  fTargetMCCached = nullptr;
  //fTargetMCCached = currentMC;
  //-----------------------------------------------------------
  //-----------------------------------------------------------
  //fTargetMCCached = currentMC;
  fSuggestTrackForMoving(currentMC, fTargetMCCached);
  //if(targetEngine && currentMC != targetEngine) {
  if(fTargetMCCached) {
    //Info("SuggestTrackForMoving","Track moved to engine %s", targetEngine->GetName());
    MoveTrack(currentMC, fTargetMCCached->GetQueue());
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Move track from a given engine to a target queue of another engine
///

void TMCStackManager::MoveTrack(TVirtualMC* currentMC, TMCQueue* targetQueue)
{

  // Extract spatial and momentum state
  // \not Time of flight is encoded using TVirtualMC::TrackPosition(TLorentzVector)
  currentMC->TrackPosition(fCurrentPosition);
  currentMC->TrackMomentum(fCurrentMomentum);
  // /fCurrentTrack = fMasterStack->GetCurrentTrack();
  // \todo Rather update which implies adding another point to this track internally
  // and keeping the old point as well to reconstruct where engine changes happened.
  fCurrentTrack->AddPointMomentum(fCurrentPosition, fCurrentMomentum);
  //currentTrack->SetProductionVertex(fCurrentPosition);
  //currentTrack->SetMomentum(fCurrentMomentum);


  // Push index of TGeoNavigator and set it for this track
  //-----------------------------------------------------------
  //-----------------------------------------------------------
  fGeoStateCached = fGeoStateCache->GetNewGeoState();
  fGeoStateCached->InitFromNavigator(fCurrentNavigator);
  fCurrentTrack->GeoStateIndex(fGeoStateCached->GetUniqueID());
  //-----------------------------------------------------------
  //-----------------------------------------------------------

  //Int_t trackIdManager = fMasterStack->GetCurrentTrackNumber();
  //if(trackIdManager != currentTrack->Id()) {
    //Warning("MoveTrack", "track id in stack manager: %i and track id of track: %i", trackIdManager, currentTrack->Id() );
  //}
  //Int_t pdgEngine = currentMC->TrackPid();
  //Int_t pdgTrack = currentTrack->GetPdgCode();
  //if( pdgEngine != pdgTrack) {
    //Warning("MoveTrack", "pdg from engine: %i and padg from track: %i", pdgEngine, pdgTrack );
  //}
  /*auto it = fTrackIdPDGMap.find(currentTrack->Id());
  if(it == fTrackIdPDGMap.end()) {
    fTrackIdPDGMap[currentTrack->Id()] = pdgEngine;
  } else if(it->second != fTrackIdPDGMap[currentTrack->Id()]) {
    Warning("MoveTrack", "pdg of track %i has changed from %i to %i", it->second, pdgEngine );
    fTrackIdPDGMap[currentTrack->Id()] = pdgEngine;
  }*/
  //if()
  //-----------------------------------------------------------
  //-----------------------------------------------------------
  targetQueue->PushTrack(fCurrentTrack);
  currentMC->StopTrack();
  //-----------------------------------------------------------
  //-----------------------------------------------------------
  // /Info("MoveTrack", "Track %i with geo state %i will be moved from %s", fCurrentTrack->Id(), geoState->GetUniqueID(), currentMC->GetName());
   // \todo So far only stacking track ids, but also be able to see from where to where in the end
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the function called in stepping to check whether a track needs to be moved
///
void TMCStackManager::RegisterSuggestTrackForMoving(
                            std::function<void(TVirtualMC*, TVirtualMC*&)> f)
{
  fSuggestTrackForMoving = f;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the function called in stepping to check whether a track needs to be moved
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
  // Additionally, register engine to this Manager for stack/queue handling
  fMCEngines.push_back(mc);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Register the user VMC stack to the manager
///

void TMCStackManager::RegisterStack(TVirtualMCStack* stack)
{
  if(fMasterStack) {
    Warning("RegisterStack", "There is already a stack which will now be replaced and deleted");
    delete fMasterStack;
  }
  fMasterStack = stack;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Initialize engine queues with primaries
///

Bool_t TMCStackManager::HasPrimaries()
{
  // \todo So far check here again whether there are engines registered
  if(fMCEngines.empty()) {
    Fatal("HasPrimaries", "No MC engines registered");
  }
  if(fMasterStack->GetNtrackToDo() < 1) {
    // No tracks to do on stack so check the individual queues
    for(auto& mc : fMCEngines) {
      if(mc->GetQueue()->GetNtrack() > 0) {
        return kTRUE;
      }
    }
    return kFALSE;
  }
  return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Clear stack
///
void TMCStackManager::ResetStack()
{
  if(fMasterStack) {
    fMasterStack->ResetStack();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Print status
///

void TMCStackManager::Print() const
{
  Info("Print", "Status of stacks and queues");
  std::cout << "\t#tracks: " << fMasterStack->GetNtrack() << std::endl;
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
