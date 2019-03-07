// @(#)root/vmc:$Id$
// Author: Benedikt Volkel, 07/03/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include <iostream>

#include "TError.h"
#include "TParticle.h"
#include "TGeoBranchArray.h"
#include "TGeoMCBranchArrayContainer.h"
#include "TMCParticleStatus.h"
#include "TMCManagerStack.h"

/** \class TMCManagerStack
    \ingroup vmc

Concrete implementation of particles stack used by the TMCManager.
*/

ClassImp(TMCManagerStack);

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TMCManagerStack::TMCManagerStack()
   : TVirtualMCStack(), fCurrentTrackId(-1), fUserStack(nullptr), fTotalNPrimaries(nullptr), fTotalNTracks(nullptr),
     fParticles(nullptr), fParticlesStatus(nullptr), fBranchArrayContainer(nullptr)
{
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TMCManagerStack::~TMCManagerStack() {}

////////////////////////////////////////////////////////////////////////////////
///
/// This will just forward the call to the fUserStack's PushTrack
///

void TMCManagerStack::PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg, Double_t px, Double_t py, Double_t pz,
                                Double_t e, Double_t vx, Double_t vy, Double_t vz, Double_t tof, Double_t polx,
                                Double_t poly, Double_t polz, TMCProcess mech, Int_t &ntr, Double_t weight, Int_t is)
{
   // Just forward to user stack
   fUserStack->PushTrack(toBeDone, parent, pdg, px, py, pz, e, vx, vy, vz, tof, polx, poly, polz, mech, ntr, weight,
                         is);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop next track
///

TParticle *TMCManagerStack::PopNextTrack(Int_t &itrack)
{

   if (fPrimariesStack.empty() && fSecondariesStack.empty()) {
      itrack = -1;
      return nullptr;
   }

   std::stack<Int_t> *mcStack = &fPrimariesStack;

   if (fPrimariesStack.empty()) {
      mcStack = &fSecondariesStack;
   }
   itrack = mcStack->top();
   mcStack->pop();
   return fParticles->operator[](itrack);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop i'th primary; that does not mean that this primariy has ID==i
///

TParticle *TMCManagerStack::PopPrimaryForTracking(Int_t i)
{
   Int_t itrack = -1;
   return PopPrimaryForTracking(i, itrack);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop i'th primary; that does not mean that this primariy has ID==i.
/// including actual index
///

TParticle *TMCManagerStack::PopPrimaryForTracking(Int_t i, Int_t &itrack)
{
   // Completely ignore the index i, that is meaningless since the user does not
   // know how the stack is handled internally.
   if (fPrimariesStack.empty()) {
      itrack = -1;
      return nullptr;
   }
   itrack = fPrimariesStack.top();
   fPrimariesStack.pop();
   return fParticles->operator[](itrack);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get number of tracks on current sub-stack
///

Int_t TMCManagerStack::GetNtrack() const
{
   return *fTotalNTracks;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get only the number of currently stacked tracks
///

Int_t TMCManagerStack::GetStackedNtrack() const
{
   return fPrimariesStack.size() + fSecondariesStack.size();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get number of primaries on current sub-stack
///

Int_t TMCManagerStack::GetNprimary() const
{
   return *fTotalNPrimaries;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get number of primaries on current sub-stack
///

Int_t TMCManagerStack::GetStackedNprimary() const
{
   return fPrimariesStack.size();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Current track
///

TParticle *TMCManagerStack::GetCurrentTrack() const
{
   if (fCurrentTrackId < 0) {
      Fatal("GetCurrentTrack", "There is no current track set");
   }
   // That is not actually the current track but the user's TParticle at the
   // vertex.
   return fParticles->operator[](fCurrentTrackId);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Current track number
///

Int_t TMCManagerStack::GetCurrentTrackNumber() const
{
   return fCurrentTrackId;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Number of the parent of the current track
///

Int_t TMCManagerStack::GetCurrentParentTrackNumber() const
{
   return fParticlesStatus->operator[](fCurrentTrackId)->fParentId;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the current track id from the outside and forward this to the
/// user's stack
///

void TMCManagerStack::SetCurrentTrack(Int_t trackId)
{
   if (!HasTrackId(trackId)) {
      Fatal("SetCurrentTrack", "Invalid track ID %i", trackId);
   }
   fCurrentTrackId = trackId;
   fUserStack->SetCurrentTrack(trackId);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get TMCParticleStatus by trackId
///

const TMCParticleStatus *TMCManagerStack::GetParticleStatus(Int_t trackId) const
{
   if (!HasTrackId(trackId)) {
      Fatal("GetParticleStatus", "Invalid track ID %i", trackId);
   }
   return fParticlesStatus->operator[](trackId);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get particle's geometry status by trackId
///

const TGeoBranchArray *TMCManagerStack::GetGeoState(Int_t trackId) const
{
   if (!HasTrackId(trackId)) {
      Fatal("GetParticleStatus", "Invalid track ID %i", trackId);
   }
   Int_t geoStateIndex = fParticlesStatus->operator[](trackId)->fGeoStateIndex;
   if (geoStateIndex > -1) {
      return fBranchArrayContainer->GetGeoState(geoStateIndex);
   }
   return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
///
/// To free the cached geo state which was associated to a track
///

void TMCManagerStack::NotifyOnRestoredGeometry(Int_t trackId)
{
   if (!HasTrackId(trackId)) {
      Fatal("NotifyOnRestoredGeometry", "Invalid track ID %i", trackId);
   }
   Int_t geoStateIndex = fParticlesStatus->operator[](trackId)->fGeoStateIndex;
   if (geoStateIndex > -1) {
      fBranchArrayContainer->FreeGeoState(geoStateIndex);
   }
}

////////////////////////////////////////////////////////////////////////////////
///
/// To free the cached geo state which was associated to a track
///
void TMCManagerStack::NotifyOnRestoredGeometry(const TGeoBranchArray *geoState)
{
   if (geoState) {
      fBranchArrayContainer->FreeGeoState(geoState);
   }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Check whether track trackId exists
///

Bool_t TMCManagerStack::HasTrackId(Int_t trackId) const
{
   if (trackId >= 0 && trackId < fParticles->size() && fParticles->operator[](trackId)) {
      return kTRUE;
   }
   return kFALSE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the user stack
///

void TMCManagerStack::SetUserStack(TVirtualMCStack *stack)
{
   fUserStack = stack;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Connect an engine's stack to the centrally managed vectors
///

void TMCManagerStack::ConnectTrackContainers(std::vector<TParticle *> *particles,
                                             std::vector<TMCParticleStatus *> *tracksStatus,
                                             TGeoMCBranchArrayContainer *branchArrayContainer, Int_t *totalNPrimaries,
                                             Int_t *totalNTracks)
{
   fParticles = particles;
   fParticlesStatus = tracksStatus;
   fBranchArrayContainer = branchArrayContainer;
   fTotalNPrimaries = totalNPrimaries;
   fTotalNTracks = totalNTracks;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Push primary track id to be processed
///

void TMCManagerStack::PushPrimaryTrackId(Int_t trackId)
{
   fPrimariesStack.push(trackId);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Push secondary track id to be processed
///

void TMCManagerStack::PushSecondaryTrackId(Int_t trackId)
{
   fSecondariesStack.push(trackId);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Reset internals, clear engine stack and fParticles and reset buffered values
///

void TMCManagerStack::ResetInternals()
{
   // Reset current stack and track IDs
   fCurrentTrackId = -1;
   while (!fPrimariesStack.empty()) {
      fPrimariesStack.pop();
   }
   while (!fSecondariesStack.empty()) {
      fSecondariesStack.pop();
   }
}
