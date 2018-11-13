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
  : TVirtualMCApplication(name, title)
{}

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
  // First set the current engine
  fMC = newMC;
  // If in concurrent mode, add engine to the list of engines
  // make sure, at least engine names are unique
  for(auto& mc : fMCEngines) {
    if(strcmp(newMC->GetName(), mc->GetName()) == 0) {
      Fatal("RegisterMC", "There is already an engine with name %s.", mc->GetName());
    }
  }
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

////////////////////////////////////////////////////////////////////////////////
///
/// Do all initialisation steps at once
///
// \note \todo Do this via a lambda given by the user?
void TVirtualMCMultiApplication::InitTransport()
{
  // Some user pre init steps
  // At this point the TGeoManager must be there with closed geometry
  if(!gGeoManager || !gGeoManager->IsClosed()) {
    Fatal("InitMCs","Could not find TGeoManager or geometry is still not closed");
  }
  // Check whether there is a TVirtualMCMultiStack and fail if not.
  if(!fStack) {
    Fatal("InitMCs","A TVirtualMCMultiStack si required");
  }

  // Initialize engines
  for(auto& mc : fMCEngines) {
    Info("InitMCs", "Initialize engine %s", mc->GetName());
    // Notify to use geometry built using TGeo
    mc->SetRootGeometry();
    // Further init steps for the MCs
    mc->Init();
    mc->BuildPhysics();
    // Set the stack for this engine
    mc->SetStack(fStack);
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Run the transport by steering engines
///

void TVirtualMCMultiApplication::RunTransport(Int_t nofEvents)
{
  // Set initial navigator \todo Move this to a more consistent place.
  fStack->SetCurrentNavigator(gGeoManager->GetCurrentNavigator());
  // Check dryrun, so far nothing is done.
  if(nofEvents < 1) {
    Info("RunMCs", "Starting dry run.");
    return;
  }

  // Run 1 event nofEvents times
  for(Int_t i = 0; i < nofEvents; i++) {
    // Generate primaries according to the user
    GeneratePrimariesMulti();
    // Call user begin event action
    BeginEventMulti();
    // Loop as long there are tracks in any TMCQueue
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
  // \note Kind of brute force selection by bjust checking the queue
  for(auto& mc : fMCEngines) {
    if(fStack->HasTracks(mc)) {
      fMC = mc;
      fStack->SetQueue(mc);
      // Set the current navigator for this engine.
      // \todo The following works since right now there is only one navigator.
      fStack->SetCurrentNavigator(gGeoManager->GetCurrentNavigator());
      //UpdateConnectedEnginePointers();
      return kTRUE;
    }
  }
  // No track to be processed.
  return kFALSE;
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
  // Check conditions and then call the BeginEventMulti()
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
  // Call the user stepping actions
  // /Info("Stepping", "Do custom user stepping");
  SteppingMulti();
  fStack->SuggestTrackForMoving(fMC);
}

////////////////////////////////////////////////////////////////////////////////
///
/// After a track is transported
///

void TVirtualMCMultiApplication::PostTrack()
{
  // Check conditions and then call the BeginEventMulti()
  PostTrackMulti();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Specific post processing when a primary is finished
///

void TVirtualMCMultiApplication::FinishPrimary()
{
  // Check conditions and then call the BeginEventMulti()
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
