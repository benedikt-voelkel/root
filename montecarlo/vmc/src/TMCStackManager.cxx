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
#include "TMCQueue.h"
#include "TVirtualMCStack.h"
#include "TVirtualMC.h"
#include "TMCContainer.h"
#include "TParticle.h"
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
   fMasterStack = nullptr;
   fCurrentTrack = nullptr;
   fCurrentTrackID = -1;
   fLastTrackSuggestedForMoving = kFALSE;
   fgInstance = this;
}

////////////////////////////////////////////////////////////////////////////////
/// Destructor

TMCStackManager::~TMCStackManager()
{}


TParticle* TMCStackManager::PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                        Double_t px, Double_t py, Double_t pz, Double_t e,
                        Double_t vx, Double_t vy, Double_t vz, Double_t tof,
                        Double_t polx, Double_t poly, Double_t polz,
                        TMCProcess mech, Int_t& ntr, Double_t weight,
                        Int_t is)
{
  // Needs not to be done anymore since it will be forwarded to the queue of the
  // responsible engine
  // \todo If a track is pushed while transporting directly forward it to a queue, set toBeDone=0
  TParticle* track = fMasterStack->PushTrack(0, parent, pdg, px, py, pz, e, vx, vy, vz,
                            tof, polx, poly, polz, mech, ntr, weight, is);
  // If the track is toBeDone it needs to be forwarded to the queue of the
  // responsible engine
  if(toBeDone) {
    /// \todo Make that more efficient, maybe already create a particle pointer here?!
    PushToQueue(track);
  }
  return track;
}

// \todo Does it make sence to encode the production in the particle object?
// \note \todo Unify the usage of the first mother as the actual parent
TParticle* TMCStackManager::PushTrack(Int_t toBeDone, TParticle* particle,
                                TMCProcess mech, Int_t& ntr)
{
  TVector3 v;
  particle->GetPolarisation(v);
  return PushTrack(toBeDone, particle->GetFirstMother(), particle->GetPdgCode(), particle->Px(), particle->Py(),
            particle->Pz(), particle->Energy(), particle->Vx(), particle->Vy(), particle->Vz(),
            particle->T(), v.X(), v.Y(), v.Z(), mech, ntr, particle->GetWeight(), particle->GetStatusCode());
}

////////////////////////////////////////////////////////////////////////////////
///
/// Find the right queue to push this track to
///

void TMCStackManager::PushToQueue(TParticle* track)
{
  // Invokig the TGeoManager \todo Might cost too much time in the end to
  // find volume by spatial coordinates
  TGeoNode* node = gGeoManager->FindNode(track->Vx(), track->Vy(), track->Vz());
  if(node) {
    fCurrentVolId = node->GetVolume()->GetNumber();
  }
  // First check if the track fits exclusively to criteria of a certain TMCContainer
  for(auto& con : fMCContainers) {
    if(TrackFitsExclusively(con)) {
      con->fQueue->PushTrack(track);
      Info("PushToQueue", "Push track %i to queue of engine %s", track->ID(), con->fName);
      return;
    }
  }
  // After that do inclusive check for all other engines
  for(auto& con : fMCContainers) {
    if(TrackFitsInclusively(con)) {
      con->fQueue->PushTrack(track);
      Info("PushToQueue", "Push track %i to queue of engine %s", track->ID(), con->fName);
      return;
    }
  }
  Fatal("PushToQueue", "No engine fits the criteria for the track %i", track->ID());
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


void TMCStackManager::BufferSelectionParameters(TMCContainer* currentContainer)
{
  // The only parameter the selection is based upon so far
  fCurrentVolId = currentContainer->fMC->CurrentVolID(fCurrentVolCopyNo);
}

/*void TMCStackManager::BufferCurrentTrackParameters(TMCContainer* currentContainer)
{
  currentContainer->TrackPosition(fCurrentTrackPosition);
  currentContainer->TrackMomentum(fCurrentTrackPosition);
  TParticle* fCurrentTrack = fMasterStack->GetCurrentTrack();
}*/

////////////////////////////////////////////////////////////////////////////////
///
/// Check whether track and engine parameters meeting criteria inclusively
///

void TMCStackManager::SuggestTrackForMoving(TMCContainer* currentContainer)
{
  // Called from TMCManager::Stepping for each step

  // Check whther a track is leaving the current volume
  if(currentContainer->fMC->IsTrackExiting()) {
    fLastTrackSuggestedForMoving = kTRUE;
    return;
  }
  // The last track left a volume, parameters were buffered, now check selection
  // criteria in new volume
  if(fLastTrackSuggestedForMoving) {
    // Store the parameters of the current track and container
    BufferSelectionParameters(currentContainer);
    // Add/try some general checks to see whether more sofisticated ones are necessary
    // maybe even at the level of the TMCManager so that this method has not even to
    // be called

    // First check if the track fits exclusively to criteria of a certain TMCContainer
    for(auto& con : fMCContainers) {
      if(TrackFitsExclusively(con)) {
        // If another engine fits, stop track at the current one
        if(con != currentContainer) {
          MoveTrack(currentContainer->fMC, con->fQueue);
          // Since the track has been moved the current engine needs to stop further transport
          // \todo Should the decision be left to the caller?
          currentContainer->fMC->StopTrack();
        }
        fLastTrackSuggestedForMoving = kFALSE;
        return;
      }
    }

    // After that do inclusive check for all other engines
    for(auto& con : fMCContainers) {
      if(TrackFitsInclusively(con)) {
        // If another engine fits, stop track at the current one
        if(con != currentContainer) {
          MoveTrack(currentContainer->fMC, con->fQueue);
          // Since the track has been moved the current engine needs to stop further transport
          // \todo Should the decision be left to the caller?
          currentContainer->fMC->StopTrack();
        }
        fLastTrackSuggestedForMoving = kFALSE;
        return;
      }
    }

    // Normally, we shouldn't end up here since a general check whether the entire
    // simulated volume is covered should have been done before a run is triggered
    // ==> \todo
    Fatal("SuggestTrackForMoving", "No engine fits the criteria for the current track %i which was supposed to be moved from engine %s", fMasterStack->GetCurrentTrackNumber(), currentContainer->fName);
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Check whether track and engine parameters meeting criteria exclusively
///

Bool_t TMCStackManager::TrackFitsExclusively(TMCContainer* container) const
{
  /// \todo \note The only criterium to be met is the current volume id, also not caring about the copy number
  if(container->fSelectionCriteria->FitsExclusively(fCurrentVolId)) {
    return kTRUE;
  }
  return kFALSE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Check whether track and engine parameters meeting criteria inclusively
///

Bool_t TMCStackManager::TrackFitsInclusively(TMCContainer* container) const
{
  /// \todo \note The only criterium to be met is the current volume id, also not caring about the copy number
  if(container->fSelectionCriteria->FitsInclusively()) {
    return kTRUE;
  }
  return kFALSE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Move track from a given engine to a target queue of another engine
///

void TMCStackManager::MoveTrack(TVirtualMC* currentMC, TMCQueue* targetQueue)
{

  TLorentzVector position;
  TLorentzVector momentum;
  // Time of flight is encoded using TVirtualMC::TrackPosition(TLorentzVector)
  currentMC->TrackPosition(position);
  currentMC->TrackMomentum(momentum);
  TParticle* currentTrack = fMasterStack->GetCurrentTrack();
  // \todo Also push the state of the navigator so that it can be popped directly
  // when this track is transported further to save time
  Info("MoveTrack", "Track %i will be moved from %s", fMasterStack->GetCurrentTrackNumber(), currentMC->GetName());
  targetQueue->PushTrack(new TParticle(fMasterStack->GetCurrentTrackNumber(),
                                  currentTrack->GetPdgCode(),
                                  currentTrack->GetStatusCode(),
                                  currentTrack->GetFirstMother(),
                                  -1,
                                  currentTrack->GetFirstDaughter(),
                                  currentTrack->GetLastDaughter(),
                                  momentum, position));
   // \todo So far only stacking track ids, but also be able to see from where to where in the end
   //fTrackIdsMoved.push_back(fMasterStack->GetCurrentTrackNumber());
}

////////////////////////////////////////////////////////////////////////////////
///
/// Assign a queue for a given engine
///

void TMCStackManager::SetQueue(TMCContainer* container)
{
  // Warn and delete if there is already a queue set for this container
  if(container->fQueue) {
    Warning("SetQueue", "There is already a queue for MC %s which will now be replaced and deleted", container->fName);
    delete container->fQueue;
  }
  // Set queue
  container->fQueue = new TMCQueue();
  container->fMC->SetQueue(container->fQueue);
  // Additionally, register container to this Manager for stack/queue handling
  fMCContainers.push_back(container);
}

void TMCStackManager::RegisterStack(TVirtualMCStack* stack)
{
  if(fMasterStack) {
    Warning("RegisterStack", "There is already a stack which will now be replaced and deleted");
    delete fMasterStack;
  }
  fMasterStack = stack;
}

void TMCStackManager::SetCurrentTrack(Int_t trackID)
{
  fCurrentTrackID = trackID;
  fMasterStack->SetCurrentTrack(trackID);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Initialize engine queues with primaries
///

Bool_t TMCStackManager::HasPrimaries()
{
  // \todo So far check here again whether there are containers registered
  if(fMCContainers.empty()) {
    Fatal("HasPrimaries", "No TMCContainers registered");
    return kFALSE;
  }
  if(fMasterStack->GetNtrackToDo() < 1) {
    // No tracks to do on stack so check the individual queues
    for(auto& con : fMCContainers) {
      if(con->fQueue->GetNtrack() > 0) {
        return kTRUE;
      }
    }
    return kFALSE;
  }
  TParticle* track = nullptr;
  Int_t trackId;
  Info("InitializeQueuesForPrimaries", "Push primaries to queues");

  while(( track = fMasterStack->PopNextTrack(trackId) )) {
    // \todo Setting the track id manually should not be necessary
    track->SetID(trackId);
    PushToQueue(track);
    //fMCContainers.back()->fQueue->PushTrack(track);
  }
  return kTRUE;
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
