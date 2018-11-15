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
#include "TVector3.h"
#include "TError.h"
#include "TTrack.h"

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
    fTracks(new TObjArray(100)), fNPrimaries(0), fCurrentTrack(nullptr),
    fCurrentTrackID(-1), fCurrentParentTrackID(-1),
    fCurrentStack(nullptr), fCurrentNPrimaries(nullptr)
{
    fTrackTransportStatus.clear();
    fStacks.clear();
    fPrimariesStackCounts.clear();
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
     fNPrimaries++;
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
/// Check whether stack with specified ID is present
///

Bool_t TVirtualMCMultiStack::HasStackId(Int_t id) const
{
  if(id < 0 || id >= fStacks.size()) {
    return kFALSE;
  }
  return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Find the right engine stack to push this track to and push it
///

void TVirtualMCMultiStack::PushToInternalStack(TTrack* track)
{
  Int_t id = -1;
  // Call function defined by the user to decide where to push a track to
  fSpecifyStackForTrack(track, id);
  if(!HasStackId(id)) {
    Fatal("PushToInternalStack", "There is no stack with id %i", id);
  }
  fStacks[id].Push(track);
  // Imcrement number of primaries for this stack
  if(!track->GetParent()) {
    fPrimariesStackCounts[id]++;
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

Bool_t TVirtualMCMultiStack::HasTracks(Int_t id) const
{
  if(HasStackId(id)) {
    if(fStacks[id].Size() > 0) {
      return kTRUE;
    }
  }
  return kFALSE;
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
/// Set the function called to decide to which stack a primary is pushed
///

void TVirtualMCMultiStack::RegisterSpecifyStackForTrack(
                             std::function<void(TTrack*, Int_t&)> f)
{
  fSpecifyStackForTrack = f;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Add a internal stack for an engine
///

void TVirtualMCMultiStack::AddStack(Int_t id)
{
  // Check if ID is valid
  if(id >= 0) {
    // Check whether stack vector needs to be expanded
    if(id >= fStacks.size()) {
      fStacks.resize(2*fStacks.size() + 1);
    }
    // Assign new TMCSimpleStack object (don't care if there was another one
    // before since it's is the user's responsibility to delete TTrack pointers)
    fStacks[id] = TMCSimpleStack<TTrack*>(100);
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Add a stack for a given engine and set engine stack
///

void TVirtualMCMultiStack::UseStack(Int_t id)
{
  if(!HasStackId(id)) {
    Fatal("UseStack", "No stack with id %i", id);
  }
  // Set the current stack and primary counter
  fCurrentStack = &fStacks[id];
  fCurrentNPrimaries = &fPrimariesStackCounts[id];
}

////////////////////////////////////////////////////////////////////////////////
///
/// Check whether the user's criteria call for moving the track from the
/// current engine
///

void TVirtualMCMultiStack::MoveCurrentTrack(Int_t stackId,
                                            const TLorentzVector& newPosition,
                                            const TLorentzVector& newMomentum,
                                            Int_t geoStateIndex)
{
  // Update position and momentum
  fCurrentTrack->AddPositionMomentum(newPosition, newMomentum);
  // Connect ID of this geometry state to the current track.
  fCurrentTrack->GeoStateIndex(geoStateIndex);

  // Push current track to stack of next responsible engine...
  fStacks[stackId].Push(fCurrentTrack);
  // ...increment number of primaries
  if(!fCurrentTrack->GetParent()) {
    fPrimariesStackCounts[stackId]++;
  }
  // ...update its transport status. This is mainly used to control calls of
  // TVirtualMCApplication::PreTrack() and ::PostTrack().
  fTrackTransportStatus[fCurrentTrack->Id()] =
                                            ETrackTransportStatus::kProcessing;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Reset internals, clear engine stack and fTracks and reset buffered values
///

void TVirtualMCMultiStack::ResetInternals()
{
  // Clear cached track pointers, user has to do actual de-allocation
  fTracks->Clear();
  fCurrentTrack = nullptr;
  fCurrentTrackID = -1;
  fCurrentParentTrackID = -1;
  fTrackTransportStatus.clear();
  for(auto& q : fStacks) {
    q.Reset();
  }
  fCurrentStack = nullptr;
}
