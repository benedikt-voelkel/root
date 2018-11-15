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

#include "TVirtualMCMultiApplication.h"
#include "TVirtualMCMultiStack.h"
#include "TVirtualMC.h"
#include "TGeoManager.h"
#include "TGeoMCBranchArrayContainer.h"
#include "TGeoBranchArray.h"
#include "TParticle.h"
#include "TLorentzVector.h"

/** \class TVirtualMCMultiApplication
    \ingroup vmc

Interface to a user Monte Carlo application using multiple transport engines
for the simulation setup.

*/

ClassImp(TVirtualMCMultiApplication);

////////////////////////////////////////////////////////////////////////////////
///
/// Standard constructor
///

TVirtualMCMultiApplication::TVirtualMCMultiApplication(const char *name,
                                                       const char *title)
  : TVirtualMCApplication(name, title), fTargetMCCached(nullptr),
    fStack(nullptr), fCurrentPosition(TLorentzVector()),
    fCurrentMomentum(TLorentzVector()),
    fBranchArrayContainer(TGeoMCBranchArrayContainer(8, 100)),
    fCachedGeoState(nullptr), fCurrentNavigator(nullptr),
    fIsGeometryConstructed(kFALSE), fIsInitialized(kFALSE)
{
  fMCEngines.clear();
  fSubStackIds.clear();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCMultiApplication::TVirtualMCMultiApplication()
  : TVirtualMCApplication()
{}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TVirtualMCMultiApplication::~TVirtualMCMultiApplication()
{}

////////////////////////////////////////////////////////////////////////////////
///
/// Register a new transport engine
///

void TVirtualMCMultiApplication::RegisterMC(TVirtualMC* newMC)
{
  // Abort if already initialized
  if(fIsInitialized) {
    Fatal("RegisterMC", "Already initialized, engines need to registered " \
                        "before initialization.");
  }
  // First set the current engine
  fMC = newMC;
  // If in concurrent mode, add engine to the list of engines
  // make sure, at least engine names are unique
  for(auto& mc : fMCEngines) {
    if(strcmp(newMC->GetName(), mc->GetName()) == 0) {
      Fatal("RegisterMC", "There is already an engine with name %s.",
                          mc->GetName());
    }
  }
  fMC->SetBranchArrayContainer(&fBranchArrayContainer);
  fMC->SetId(fMCEngines.size());
  // Insert the new TVirtualMC
  fMCEngines.push_back(newMC);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the TVirtualMCMultiStack
///

void TVirtualMCMultiApplication::SetStack(TVirtualMCMultiStack* stack)
{
  if(fStack && fStack != stack) {
    Warning("SetStack", "There is already a TVirtualMCMultiStack " \
                        "which will be overriden now.");
  }
  fStack = stack;
}

/// Set the function called in stepping to check whether a track needs to
/// be moved
void TVirtualMCMultiApplication::RegisterSuggestTrackForMoving(
                       std::function<void(const TVirtualMC*, TVirtualMC*&)> f)
{
  fSuggestTrackForMoving = f;
}

/// Set the function called to decide to which stack a primary is pushed
void TVirtualMCMultiApplication::RegisterSpecifyEngineForTrack(
                       std::function<void(const TParticle*, TVirtualMC*&)> f)
{
  fSpecifyEngineForTrack = f;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Construct geometry gloablly to be used by all engines
///
// \note \todo Do this via a lambda given by the user?
void TVirtualMCMultiApplication::ConstructGlobalGeometry()
{
  if(fIsGeometryConstructed) {
    return;
  }

  ConstructGeometryMulti();
  // Set top volume and close Root geometry if not yet done. Again check whether
  // the geometry was actually done using TGeo(Manager)
  if ( !gGeoManager->IsClosed() ) {
    TGeoVolume* top = dynamic_cast<TGeoVolume*>
                                  (gGeoManager->GetListOfVolumes()->First());
    if(!top) {
      Fatal("ConstructRootGeometry", "No top volume found: Apparently the " \
                                     "geometry was not constructed via TGeo");
    }
    gGeoManager->SetTopVolume(top);
    gGeoManager->CloseGeometry();
  }
  // TODO Do these methods also need their "Multi" counter part?
  MisalignGeometryMulti();
  ConstructOpGeometryMulti();

  fIsGeometryConstructed = kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Do all initialisation steps at once
///
// \note \todo Do this via a lambda given by the user?
void TVirtualMCMultiApplication::InitTransport(std::function<void(TVirtualMC*)>
                                               customInit)
{
  // 1. Construct geometry
  // 2. Initialize VMCs to pick that up
  // 3. Set the stack

  if(!fIsGeometryConstructed) {
    Fatal("InitTransport","Geometry needs to be constructed first");
  }
  // Check whether there is a TVirtualMCMultiStack and fail if not.
  if(!fStack) {
    Fatal("InitTransport","A TVirtualMCMultiStack is required");
  }

  fStack->RegisterSpecifyStackForTrack([this](const TParticle* particle,
                                              Int_t& stackID)
                                  {
                                    stackID = -1;
                                    TVirtualMC* mc = nullptr;
                                    this->fSpecifyEngineForTrack(particle, mc);
                                    if(mc) {
                                      stackID = this->fSubStackIds[mc->GetId()];
                                    }
                                  });

  fSubStackIds.reserve(fMCEngines.size());
  // Initialize engines
  for(auto& mc : fMCEngines) {
    Info("InitMCs", "Initialize engine %s", mc->GetName());
    if(!mc->IsRootGeometrySupported()) {
      Fatal("InitMCs", "Engine %s does not support ROOT geometry",
                       mc->GetName());
    }
    fMC = mc;
    // Create a sub-stack and set this stack to the engine
    Int_t stackID = fStack->AddSubStack();
    fSubStackIds.push_back(stackID);
    mc->SetStack(fStack);

    // Notify to use geometry built using TGeo
    mc->SetRootGeometry();
    // Call the user custom init
    customInit(mc);
    // Further init steps for the MCs
    mc->Init();
    mc->BuildPhysics();
  }
  fIsInitialized = kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Run the transport by steering engines
///

void TVirtualMCMultiApplication::RunTransport(Int_t nofEvents)
{
  // Set initial navigator \todo Move this to a more consistent place.
  fCurrentNavigator = gGeoManager->GetCurrentNavigator();
  // Check dryrun, so far nothing is done.
  if(nofEvents < 1) {
    Info("RunMCs", "Starting dry run.");
    return;
  }
  // Run 1 event nofEvents times
  for(Int_t i = 0; i < nofEvents; i++) {
    // Generate primaries according to the user
    fStack->ResetInternals();
    GeneratePrimariesMulti();
    // Call user begin event action
    BeginEventMulti();

    // Loop as long there are tracks in any engine stack
    while(GetNextEngine()) {
      fMC->ProcessEvent(i);
    }
    // Call user finish event action
    FinishEventMulti();
  }
  // Terminate this run
  TerminateRun();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Choose next engines to be run in the loop
///

Bool_t TVirtualMCMultiApplication::GetNextEngine()
{
  // \note Kind of brute force selection by just checking the engine's stack
  for(UInt_t i = 0; i < fMCEngines.size(); i++) {
    Int_t stackId = fSubStackIds[i];
    if(fStack->HasTracks(stackId)) {
      fMC = fMCEngines[i];
      fStack->UseSubStack(stackId);
      return kTRUE;
    }
  }
  // No track to be processed.
  return kFALSE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Check whether the user's criteria call for moving the track from the
/// current engine
///

Bool_t TVirtualMCMultiApplication::SuggestTrackForMoving()
{
  // Start off with nullptr.
  fTargetMCCached = nullptr;
  // Ask the user implemented lambda what the target engine for this track
  // should be.
  fSuggestTrackForMoving(fMC, fTargetMCCached);
  // If target engine is not specified or target engine is current engine
  // do nothing
  if(!fTargetMCCached || fMC == fTargetMCCached) {
    return kFALSE;
  }
  fMC->TrackPosition(fCurrentPosition);
  fMC->TrackMomentum(fCurrentMomentum);

  // Get an idle geometry state cache and set from current navigator.
  Int_t geoStateIndex = -1;
  fCachedGeoState = fBranchArrayContainer.GetNewGeoState(geoStateIndex);
  fCachedGeoState->InitFromNavigator(fCurrentNavigator);

  // Tell the stack to move the track to stack with id and...
  fStack->MoveCurrentTrack(fSubStackIds[fTargetMCCached->GetId()],
                           fCurrentPosition, fCurrentMomentum,
                           geoStateIndex);
  // ...stop this track
  fMC->StopTrack();
  return kTRUE;
}


////////////////////////////////////////////////////////////////////////////////
///
/// Terminate the run for all engines
///

void TVirtualMCMultiApplication::TerminateRun()
{
  for(auto& mc : fMCEngines) {
    mc->TerminateRun();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Construct user geometry
///

void TVirtualMCMultiApplication::ConstructGeometry()
{
  // Do nothing here
}

////////////////////////////////////////////////////////////////////////////////
///
/// Misalign user geometry
///

Bool_t TVirtualMCMultiApplication::MisalignGeometry()
{
  // Do nothing here
}

////////////////////////////////////////////////////////////////////////////////
///
/// Construct user op geometry
///

void TVirtualMCMultiApplication::ConstructOpGeometry()
{
  // Do nothing here
}

////////////////////////////////////////////////////////////////////////////////
///
/// Further steps for geometry initialisation
///

void TVirtualMCMultiApplication::InitGeometry()
{
  // Check conditions and then call the BeginEventMulti()
  InitGeometryMulti();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Primary generation.
///

void TVirtualMCMultiApplication::GeneratePrimaries()
{
  // Do nothing here.
}

////////////////////////////////////////////////////////////////////////////////
///
/// Begin of an event
///

void TVirtualMCMultiApplication::BeginEvent()
{
  // Nothing to do here.
}

////////////////////////////////////////////////////////////////////////////////
///
/// Begin transportation of a new primary
///

void TVirtualMCMultiApplication::BeginPrimary()
{
  if(fStack->GetTrackTransportStatus() == ETrackTransportStatus::kNew) {
    BeginPrimaryMulti();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Before a new track is transported
///

void TVirtualMCMultiApplication::PreTrack()
{
  if(fStack->GetTrackTransportStatus() == ETrackTransportStatus::kNew) {
    PreTrackMulti();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Call user stepping and check whether track should be moved.
///

void TVirtualMCMultiApplication::Stepping()
{
  /// First check whether the track meets criteria of user to be stopped and
  /// moved.
  /// TODO What to do if it meets criteria? Does the step has to be reset and
  ///      done again in the new engine? Does the former state hence need to be
  ///      cached all the time?
  /// So far it means one step could be lost.
  if(!SuggestTrackForMoving()) {
    SteppingMulti();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// After a track is transported
///

void TVirtualMCMultiApplication::PostTrack()
{
  PostTrackMulti();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Specific post processing when a primary is finished
///

void TVirtualMCMultiApplication::FinishPrimary()
{
  FinishPrimaryMulti();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Specific post processing when a primary is finished
///

void TVirtualMCMultiApplication::FinishEvent()
{
  // Nothing to do here.
}
