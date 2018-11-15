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
#include "TLorentzVector.h"
#include "TError.h"



#include "TGeoBranchArray.h"

/** \class TVirtualMCMultiStack
    \ingroup vmc

Interface to a user defined particles stack.
*/

ClassImp(TVirtualMCMultiStack);

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCMultiStack::TVirtualMCMultiStack(Int_t initialStackSize)
  : TVirtualMCStack(),
    fInitialStackSize(initialStackSize), fCurrentTrackId(-1),
    fCurrentStackId(-1)
{
    fTracks.clear();
    fStacks.clear();
    fPrimariesStackCounts.clear();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCMultiStack::TVirtualMCMultiStack()
  : TVirtualMCMultiStack(0)
{
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TVirtualMCMultiStack::~TVirtualMCMultiStack()
{
  for(auto& t : fTracks) {
    if(t.fTmpParticle) {
      delete t.fTmpParticle;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Wrapper around TVirtualMCMultiStack::PushTrack
///
/// Without geoState
void TVirtualMCMultiStack::PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                                     Double_t px, Double_t py, Double_t pz,
                                     Double_t e, Double_t vx, Double_t vy,
                                     Double_t vz, Double_t tof, Double_t polx,
                                     Double_t poly, Double_t polz,
                                     TMCProcess mech, Int_t& ntr,
                                     Double_t weight, Int_t is)
{
  // Just forward to PushTrack including setting geo state index to -1
  PushTrack(toBeDone, parent, pdg, px, py, pz,e, vx, vy, vz, tof, polx, poly,
            polz, -1, ETrackTransportStatus::kNew, mech, ntr, weight, is);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Wrapper around TVirtualMCMultiStack::PushTrack
///
/// With geoStateIndex
void TVirtualMCMultiStack::PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                                     Double_t px, Double_t py, Double_t pz,
                                     Double_t e, Double_t vx, Double_t vy,
                                     Double_t vz, Double_t tof, Double_t polx,
                                     Double_t poly, Double_t polz,
                                     Int_t geoStateIndex,
                                     ETrackTransportStatus transportStatus,
                                     TMCProcess mech, Int_t& ntr,
                                     Double_t weight, Int_t is)
{
  // Call user implemented method
  TParticle* particle = PushUserTrack(parent, pdg, px, py, pz, e, vx, vy, vz,
                                      tof, polx, poly, polz, geoStateIndex,
                                      transportStatus, mech, ntr, weight, is);


  // Fatal if non-valid pointer returned.
  if(!particle) {
    Fatal("PushTrack", "No user particle passed");
  }
  // Id must be positive
  if(ntr < 0) {
    Fatal("PushTrack", "Invalid track ID %i", ntr);
  }

   // Insert this track
   InsertTrack(toBeDone, particle, ntr, parent, geoStateIndex, transportStatus);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Insert a new transport status for a given track ID
///

void TVirtualMCMultiStack::InsertTrack(Bool_t toBeDone, TParticle* particle,
                                       Int_t usertrackID, Int_t parentTrackID,
                                       Int_t geoStateIndex,
                                       ETrackTransportStatus transportStatus)
{
  // Resize  if necessary to twice the current size
  if(usertrackID >= fTracks.size()) {
    fTracks.resize(2 * usertrackID + 1);
  }
  // Check if there is already a particle/track with this id
  if(fTracks[usertrackID].fUserParticle) {
    Fatal("InsertTrack", "There is already a track with id %i", usertrackID);
  }

  fTracks[usertrackID].fUserParticle = particle;
  fTracks[usertrackID].fTmpParticle = new TParticle(*particle);
  fTracks[usertrackID].fId = usertrackID;
  fTracks[usertrackID].fParentId = parentTrackID;
  fTracks[usertrackID].fGeoStateIndex = geoStateIndex;
  fTracks[usertrackID].fTransportStatus = transportStatus;

  if(toBeDone) {
    Int_t stackId = -1;
    // Call function defined by the user to decide where to push a track to
    fSpecifyStackForTrack(particle, stackId);
    if(!HasSubStackId(stackId)) {
      Fatal("PushToInternalStack", "There is no stack with id %i", stackId);
    }
    fStacks[stackId].push(usertrackID);
    if(fTracks[usertrackID].fUserParticle->IsPrimary()) {
      fPrimariesStackCounts[stackId]++;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop the next track
///

TParticle* TVirtualMCMultiStack::PopNextTrack(Int_t&  itrack)
{
  // Relying on the existence of *fCurrentStack and don't go via
  // PopNextTrackFromSubStack checking again existence of certain stack index

  return PopNextTrackFromSubStack(fCurrentStackId, itrack);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop the next track
///

TParticle* TVirtualMCMultiStack::PopNextTrackFromSubStack(Int_t stackId,
                                                          Int_t&  itrack)
{
  if(!HasSubStackId(stackId)) {
    Fatal("PopNextTrackFromSubStack", "There is no stack with id %i", stackId);
  }
  if(fStacks[stackId].empty()) {
    itrack = -1;
    return nullptr;
  }

  itrack = fStacks[stackId].top();
  fStacks[stackId].pop();

  TParticle* particle = fTracks[itrack].fTmpParticle;

  if(particle->IsPrimary()) {
    fPrimariesStackCounts[stackId]--;
  }
  return particle;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop i'th primar, that does not mean that this primariy also has ID==i
///

TParticle* TVirtualMCMultiStack::PopPrimaryForTracking(Int_t i)
{
  Int_t itrack = -1;
  return PopPrimaryForTracking(i, itrack);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop i'th primar, that does not mean that this primariy also has ID==i.
/// including actual index
///

TParticle* TVirtualMCMultiStack::PopPrimaryForTracking(Int_t i, Int_t& itrack)
{
  // Relying on the existence of *fCurrentStack and don't go via
  // PopPrimaryForTrackingFromSubStack checking again existence of certain
  // stack index

  // Completely ignore the index i, that is meaningless since the user does not
  // know how the stack is handled internally.
  // See comment in header TVirtualMCMultiStack.h
  return PopNextTrack(itrack);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop i'th primar, that does not mean that this primariy also has ID==i.
/// including actual index
///

TParticle* TVirtualMCMultiStack::PopPrimaryForTrackingFromSubStack(Int_t stackId,
                                                                   Int_t i,
                                                                   Int_t& itrack)
{
  return PopNextTrackFromSubStack(stackId, itrack);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get number of tracks on current sub-stack
///

Int_t TVirtualMCMultiStack::GetNtrack() const
{
  return fStacks[fCurrentStackId].size();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get total number of tracks
///

Int_t TVirtualMCMultiStack::GetTotalNtrack() const
{
  return fTracks.size();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get number of primaries on current sub-stack
///

Int_t TVirtualMCMultiStack::GetNprimary() const
{
  return fPrimariesStackCounts[fCurrentStackId];
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get number of primaries on current sub-stack
///

Int_t TVirtualMCMultiStack::GetTotalNprimary() const
{
  Int_t primaries = 0;
  for(auto& p : fPrimariesStackCounts) {
    primaries += p;
  }
  return primaries;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Current track
///

TParticle* TVirtualMCMultiStack::GetCurrentTrack() const
{
  if(fCurrentTrackId < 0) {
    Fatal("GetCurrentTrack", "There is no current track set");
  }
  // Return the user particle where kinematics is not updated during particle
  // transfer among engines (backwards compatible)
  return fTracks[fCurrentTrackId].fUserParticle;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Current track number
///

Int_t TVirtualMCMultiStack::GetCurrentTrackNumber() const
{
  return fTracks[fCurrentTrackId].fId;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Number of the parent of the current track
///

Int_t TVirtualMCMultiStack::GetCurrentParentTrackNumber() const
{
  return fTracks[fCurrentTrackId].fParentId;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Number of the parent of the current track
///

Int_t TVirtualMCMultiStack::GetCurrentTrackGeoStateIndex() const
{
  return fTracks[fCurrentTrackId].fGeoStateIndex;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Number of the parent of the current track
///

Int_t TVirtualMCMultiStack::GetTrackGeoStateIndex(Int_t trackID) const
{
  if(!HasTrack(trackID)) {
    return -1;
  }
  return fTracks[trackID].fGeoStateIndex;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return the transport status of the current track
///

ETrackTransportStatus TVirtualMCMultiStack::GetTrackTransportStatus() const
{
  return fTracks[fCurrentTrackId].fTransportStatus;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Check if there are tracks for specific TVirtualMC
///

Bool_t TVirtualMCMultiStack::HasTracks(Int_t stackId) const
{
  if(HasSubStackId(stackId)) {
    if(!fStacks[stackId].empty()) {
      return kTRUE;
    }
  }
  return kFALSE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the current track id from the outside and forward this to the
/// user's TVirtualMCMultiStack
///

void TVirtualMCMultiStack::SetCurrentTrack(Int_t trackID)
{
  //PrintTracks();
  if(!HasTrack(trackID)) {
    Fatal("SetCurrentTrack", "Invalid track ID %i", trackID);
  }
  fCurrentTrackId = trackID;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the function called to decide to which stack a primary is pushed
///

void TVirtualMCMultiStack::RegisterSpecifyStackForTrack(
                             std::function<void(const TParticle*, Int_t&)> f)
{
  fSpecifyStackForTrack = f;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Add a internal stack for an engine
///

Int_t TVirtualMCMultiStack::AddSubStack()
{
  fStacks.emplace_back();
  for(Int_t i = 0; i < fStacks.back().size(); i++) {
    fStacks.back().pop();
  }
  fPrimariesStackCounts.push_back(0);
  return fStacks.size() - 1;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Add a stack for a given engine and set engine stack
///

void TVirtualMCMultiStack::UseSubStack(Int_t stackId)
{
  if(!HasSubStackId(stackId)) {
    Fatal("UseSubStack", "No stack with id %i", stackId);
  }
  // Set the current stack and primary counter
  fCurrentTrackId = -1;
  fCurrentStackId = stackId;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Check whether track ID exists
///

Bool_t TVirtualMCMultiStack::HasTrack(Int_t id) const
{
  if(id < 0 || id >= fTracks.size() || !fTracks[id].fUserParticle) {
    return kFALSE;
  }
  return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Check whether stack with specified ID is present
///

Bool_t TVirtualMCMultiStack::HasSubStackId(Int_t id) const
{
  if(id < 0 || id >= fStacks.size()) {
    return kFALSE;
  }
  return kTRUE;
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
  TMCParticleStatus& track = fTracks[fCurrentTrackId];
  track.fTmpParticle->SetProductionVertex(newPosition);
  track.fTmpParticle->SetMomentum(newMomentum);
  // Connect ID of this geometry state to the current track.
  track.fGeoStateIndex = geoStateIndex;
  track.fTransportStatus = ETrackTransportStatus::kProcessing;

  // Push current track to stack of next responsible engine...
  fStacks[stackId].push(fCurrentTrackId);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Reset internals, clear engine stack and fTracks and reset buffered values
///

void TVirtualMCMultiStack::ResetInternals()
{
  // Reset current stack and track IDs
  fCurrentTrackId = -1;
  fCurrentStackId = -1;
  // Clear TMCParticleStatus vector
  for(auto& t : fTracks) {
    if(t.fTmpParticle) {
      delete t.fTmpParticle;
    }
  }
  fTracks.clear();
  // Pop everything which is left on any stack
  for(auto& q : fStacks) {
    for(Int_t i = 0; i < q.size(); i++) {
      q.pop();
    }
  }
}

void TVirtualMCMultiStack::PrintTracks() const
{
  std::cerr << "All tracks:\n";
  for(Int_t i = 0; i < fTracks.size(); i++) {
    std::cerr << "track " << i << "\n"
              << "\tat " << &fTracks[i] << "\n"
              << "\tID: " << fTracks[i].fId << "\n"
              << "\tuser particle at " << fTracks[i].fUserParticle << "\n"
              << "\temp particle at " << fTracks[i].fTmpParticle << std::endl;
  }
}
