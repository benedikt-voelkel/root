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

#include "TMCStackManager.h"
#include "TMCQueue.h"
#include "TVirtualMCStack.h"
#include "TVirtualMC.h"
#include "TMCContainer.h"
#include "TParticle.h"
#include "TVector3.h"
#include "TError.h"

/** \class TMCStackManager
    \ingroup vmc

Interface to a user defined particles stack.
*/

ClassImp(TMCStackManager);


TMCThreadLocal TMCStackManager* TMCStackManager::fgInstance = 0;

////////////////////////////////////////////////////////////////////////////////
/// Default constructor

TMCStackManager::TMCStackManager()
  : TObject()
{
   if (fgInstance) {
      Fatal("TMCManager",
            "Attempt to create two instances of singleton.");
   }
   fMasterStack = nullptr;
   fCurrentTrack = nullptr;
   fCurrentTrackID = -1;
   fgInstance = this;
}

////////////////////////////////////////////////////////////////////////////////
/// Destructor

TMCStackManager::~TMCStackManager()
{}


void TMCStackManager::PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                        Double_t px, Double_t py, Double_t pz, Double_t e,
                        Double_t vx, Double_t vy, Double_t vz, Double_t tof,
                        Double_t polx, Double_t poly, Double_t polz,
                        TMCProcess mech, Int_t& ntr, Double_t weight,
                        Int_t is)
{
  fMasterStack->PushTrack(toBeDone, parent, pdg, px, py, pz, e, vx, vy, vz,
                          tof, polx, poly, polz, mech, ntr, weight, is);
  //if(toBeDone) {
    /// \todo Make that more efficient, maybe already create a particle pointer here?!
    //PushToQueue(fMasterStack->PopTrack(ntr));
  //}
}

// \todo Does it make sence to encode the production in the particle object?
// \note \todo Unify the usage of the first mother as the actual parent
void TMCStackManager::PushTrack(Int_t toBeDone, TParticle* particle,
                                TMCProcess mech, Int_t& ntr)
{
  TVector3 v;
  particle->GetPolarisation(v);
  PushTrack(toBeDone, particle->GetFirstMother(), particle->GetPdgCode(), particle->Px(), particle->Py(),
            particle->Pz(), particle->Energy(), particle->Vx(), particle->Vy(), particle->Vz(),
            particle->T(), v.X(), v.Y(), v.Z(), mech, ntr, particle->GetWeight(), particle->GetStatusCode());
}

void TMCStackManager::BufferSelectionParameters(TMCContainer* currentContainer)
{
  // The only parameter the selection is based upon so far
  fCurrentVolId = currentContainer->fMC->CurrentVolID(fCurrentVolCopyNo);
}

void TMCStackManager::SuggestTrackForMoving(TMCContainer* currentContainer)
{
  // Store the parameters of the current track and container
  BufferSelectionParameters(currentContainer);
  // Add/try some general checks to see whether more sofisticated ones are necessary
  if(TrackFitsExclusively(currentContainer)) {
    return;
  }
  if(TrackFitsInclusively(currentContainer)) {
    return;
  }

  // First check if the track fits exclusively...
  for(auto& con : fMCContainers) {
    // Don't check the current container again
    if(con == currentContainer) {
      continue;
    }
    if(TrackFitsExclusively(con)) {
      MoveTrack(currentContainer->fMC, con->fQueue);
      // Since the track has been moved the current engine needs to stop further transport
      // \todo Should the decision be left to the caller?
      currentContainer->fMC->StopTrack();
      return;
    }
  }
  // ...if not, do inclusive comparison
  for(auto& con : fMCContainers) {
    // Don't check the current container again
    if(con == currentContainer) {
      continue;
    }
    if(TrackFitsInclusively(con)) {
      MoveTrack(currentContainer->fMC, con->fQueue);
      // Since the track has been moved the current engine needs to stop further transport
      // \todo Should the decision be left to the caller?
      currentContainer->fMC->StopTrack();
      return;
    }
  }

  // Normally, we shouldn't end up here since a general check whether the entire
  // simulated volume is covered should have been done before a run is triggered
  // ==> \todo
  Fatal("SuggestTrackForMoving", "No engine fits the criteria for the current track %i which was supposed to be moved from engine %s", fMasterStack->GetCurrentTrackNumber(), currentContainer->fName);
}

Bool_t TMCStackManager::TrackFitsExclusively(TMCContainer* container) const
{
  /// \todo \note The only criterium to be met is the current volume id, also not caring about the copy number
  if(container->fSelectionCriteria->FitsExclusively(fCurrentVolId)) {
    return kTRUE;
  }
  return kFALSE;
}

Bool_t TMCStackManager::TrackFitsInclusively(TMCContainer* container) const
{
  /// \todo \note The only criterium to be met is the current volume id, also not caring about the copy number
  if(container->fSelectionCriteria->FitsInclusively()) {
    return kTRUE;
  }
  return kFALSE;
}

void TMCStackManager::MoveTrack(TVirtualMC* currentMC, TMCQueue* targetQueue)
{
  TLorentzVector position;
  TLorentzVector momentum;
  // Time of flight is encoded using TVirtualMC::TrackPosition(TLorentzVector)
  currentMC->TrackPosition(position);
  currentMC->TrackMomentum(momentum);
  TParticle* currentTrack = fMasterStack->GetCurrentTrack();
  targetQueue->PushTrack(new TParticle(currentTrack->GetPdgCode(),
                                  currentTrack->GetStatusCode(),
                                  currentTrack->GetFirstMother(),
                                  -1,
                                  currentTrack->GetFirstDaughter(),
                                  currentTrack->GetLastDaughter(),
                                  position, momentum));
   // \todo So far only stacking track ids, but also be able to see from where to where in the end
   //fTrackIdsMoved.push_back(fMasterStack->GetCurrentTrackNumber());
}

void TMCStackManager::SetQueue(TMCContainer* container)
{
  // Warn and delete if there is already a queue set for this container
  if(container->fQueue) {
    Warning("SetQueue", "There is already a queue for MC %s which will now be replaced and deleted", container->fName);
    delete container->fQueue;
  }
  container->fQueue = new TMCQueue();
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
}

////////////////////////////////////////////////////////////////////////////////
///
/// Initialize engine queues with primaries
///

Bool_t TMCStackManager::InitializeQueuesForPrimaries()
{
  // Return false if no primaries on global stack
  if(fMasterStack->GetNtrackToDo() < 1) {
    return kFALSE;
  }
  // \todo So far check here again whether there are containers registered
  if(fMCContainers.empty()) {
    return kFALSE;
  }
  TParticle* track = nullptr;
  Int_t trackId;
  while(( track = fMasterStack->PopNextTrack(trackId) )) {
    // Now, checks are requried for where to put this primary
    // For testing resons we will just forward it to the first queue
    fMCContainers.front()->fQueue->PushTrack(track);
  }

  return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Static access method
///

TMCStackManager* TMCStackManager::Instance()
{
  if(fgInstance) {
    new TMCStackManager();
  }
  return fgInstance;
}
