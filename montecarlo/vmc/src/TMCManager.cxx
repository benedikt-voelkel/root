// @(#)root/vmc:$Id$
// Author: Benedikt Volkel, 15/01/2019

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TError.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TParticle.h"
#include "TGeoBranchArray.h"
#include "TGeoNavigator.h"
#include "TGeoManager.h"

#include "TVirtualMC.h"
#include "TVirtualMCApplication.h"
#include "TVirtualMCStack.h"
#include "TMCManagerStack.h"
#include "TMCParticleStatus.h"

#include "TMCManager.h"

/** \class TMCManager
    \ingroup vmc

Manager handling multiple TVirtualMC engines.

*/

ClassImp(TMCManager);

TMCThreadLocal TMCManager* TMCManager::fgInstance = 0;

////////////////////////////////////////////////////////////////////////////////
///
/// Default and standard constructor
///

TMCManager::TMCManager()
  : fApplication(nullptr), fCurrentEngineId(-1), fTotalNPrimaries(0),
    fTotalNTracks(0), fUserStack(nullptr),
    fBranchArrayContainer(), fIsInitialized(kFALSE)
{
   if (fgInstance) {
      Fatal("TMCManager", "Attempt to create two instances of singleton.");
   }
   fEngines.clear();
   fStacks.clear();
   fParticles.clear();
   fParticlesStatus.clear();
   fConnectedEnginePointers.clear();
   fgInstance = this;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TMCManager::~TMCManager()
{
  for(auto& s : fStacks) {
    if(s) {
      delete s;
    }
  }
  for(auto& ps : fParticlesStatus) {
    if(ps) {
      delete ps;
    }
  }
  for(auto& mc : fEngines) {
    delete mc;
  }
  fgInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Static access method
///

TMCManager* TMCManager::Instance()
{
  return fgInstance;
}

////////////////////////////////////////////////////////////////////////////////
///
/// A TVirtualMC will register itself via this method during construction
/// if a TMCManager was instanciated before.
/// The TMCManager will assign an ID to the engines.
///

void TMCManager::Register(TVirtualMC* mc)
{
  for(auto& currMC : fEngines) {
    if(strcmp(mc->GetName(), currMC->GetName()) == 0) {
      Fatal("RegisterMC", "There is already an engine with name %s.",
                          currMC->GetName());
    }
  }
  // Set id and register.
  fCurrentEngineId = fEngines.size();
  mc->SetId(fCurrentEngineId);
  fEngines.push_back(mc);
  TMCManagerStack* stack = new TMCManagerStack();
  fStacks.push_back(stack);
  mc->SetStack(stack);
  mc->SetManagerStack(stack);
  // Don't attempt to call TVirtualMCApplication hooks related to geometry
  // construction
  mc->SetExternalGeometryConstruction();
  mc->SetExternalParticleGeneration();
  UpdateEnginePointers();
}

////////////////////////////////////////////////////////////////////////////////
///
/// The user application will register itself via this method when the
/// manager was requested.
///

void TMCManager::Register(TVirtualMCApplication* application)
{
  Info("Register", "Register user application and construct geometry");
  fApplication = application;
  // TODO Can these 3 functions can be called directly here? No VMC has been
  //      created yet.
  fApplication->ConstructGeometry();
  fApplication->MisalignGeometry();
  fApplication->ConstructOpGeometry();
  if ( ! gGeoManager->IsClosed() ) {
    // TODO Just close the geometry? Or also set the top volume assuming that
    //      the volume added first should be the top volume?
    //TGeoVolume *top = (TGeoVolume*)gGeoManager->GetListOfVolumes()->First();
    //gGeoManager->SetTopVolume(top);
    gGeoManager->CloseGeometry();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return the number of registered engines.
///

Int_t TMCManager::NEngines() const
{
  return fEngines.size();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get registered engine pointers
///

void TMCManager::GetEngines(std::vector<TVirtualMC*>& engines) const
{
  engines.clear();
  engines.resize(fEngines.size(), nullptr);
  for(Int_t i = 0; i < fEngines.size(); i++) {
    engines[i] = fEngines[i];
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return the number of registered engines.
///

TVirtualMC* TMCManager::GetEngine(Int_t id) const
{
  if(id < 0 || id >= fEngines.size()) {
    Fatal("GetEngine", "Unknown engine ID.");
  }
  return fEngines[id];
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get engine ID by its name
///

Int_t TMCManager::GetEngineId(const char* engineName) const
{
  for(Int_t i = 0; i < fEngines.size(); i++) {
    if(strcmp(engineName, fEngines[i]->GetName()) == 0) {
      return i;
    }
  }
  Warning("GetEngineId", "Unknown engine %s.", engineName);
  return -1;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get the current engine pointer
///

TVirtualMC* TMCManager::GetCurrentEngine() const
{
  return fEngines[fCurrentEngineId];
}

////////////////////////////////////////////////////////////////////////////////
///
/// Connect a pointer which is updated whenever the engine is changed
///

void TMCManager::ConnectEnginePointer(TVirtualMC** mc)
{
  fConnectedEnginePointers.push_back(mc);
  if(fCurrentEngineId > -1) {
    *mc = fEngines[fCurrentEngineId];
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Connect a pointer which is updated whenever the engine is changed
///

void TMCManager::ConnectEnginePointer(TVirtualMC*& mc)
{
  ConnectEnginePointer(&mc);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set user stack
///

void TMCManager::SetUserStack(TVirtualMCStack* stack)
{
  fUserStack = stack;
}

////////////////////////////////////////////////////////////////////////////////
///
/// User interface to forward particle to specifiic engine.
/// It is assumed that the TParticle is owned by the user. It will not be
/// modified by the TMCManager.
///

void TMCManager::ForwardTrack(Int_t toBeDone, Int_t trackId, Int_t parentId,
                              TParticle* particle, Int_t engineId)
{
  if(engineId < 0 || engineId >= fEngines.size()) {
    Fatal("ForwardTrack", "Engine ID %i out of bounds. Have %i engines.",
                           engineId, fEngines.size());
  }
  if(fParticles.size() <= trackId) {
     fParticles.resize(trackId + 1, nullptr);
     fParticlesStatus.resize(trackId + 1, nullptr);
  }
  fParticles[trackId] = particle;
  fParticlesStatus[trackId] = new TMCParticleStatus();
  fParticlesStatus[trackId]->fId = trackId;
  fParticlesStatus[trackId]->fParentId = parentId;
  fParticlesStatus[trackId]->InitFromParticle(particle);
  fTotalNTracks++;
  if(particle->IsPrimary()) {
    fTotalNPrimaries++;
  }

  if(toBeDone > 0) {
    if(particle->IsPrimary()) {
      fStacks[engineId]->PushPrimaryTrackId(trackId);
    } else {
      fStacks[engineId]->PushSecondaryTrackId(trackId);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// User interface to forward particle to specifiic engine.
/// It is assumed that the TParticle is owned by the user. It will not be
/// modified by the TMCManager.
/// Assume current engine Id
///

void TMCManager::ForwardTrack(Int_t toBeDone, Int_t trackId, Int_t parentId,
                              TParticle* particle)
{
  ForwardTrack(toBeDone, trackId, parentId, particle, fCurrentEngineId);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Transfer a track from engine with engineOriginId to engineTargetId
///

void TMCManager::TransferTrack(Int_t engineOriginId, Int_t engineTargetId)
{
  if(engineOriginId == engineTargetId) {
    return;
  }

  if(engineOriginId < 0 || engineOriginId >= fEngines.size() ||
     engineTargetId < 0 || engineTargetId >= fEngines.size()) {
    Fatal("TransferTrack", "Engine IDs out of bounds. Have %i engines. Requested "\
                           "origin engine ID was %i and target ID was %i",
                           fEngines.size(), engineOriginId, engineTargetId);
  }
  // Get information on current track and extract status from transporting engine
  Int_t trackId = fStacks[engineOriginId]->GetCurrentTrackNumber();
  TMCParticleStatus* status = fParticlesStatus[trackId];

  fEngines[engineOriginId]->TrackPosition(status->fCurrentPosition);
  fEngines[engineOriginId]->TrackMomentum(status->fCurrentMomentum);
  fEngines[engineOriginId]->TrackPolarization(status->fCurrentPolarization);
  status->fCurrentStepNumber = fEngines[engineOriginId]->StepNumber();
  status->fCurrentTrackLength = fEngines[engineOriginId]->TrackLength();
  status->fCurrentWeight = fEngines[engineOriginId]->TrackWeight();

  Int_t geoStateIndex = -1;
  TGeoBranchArray* geoState = fBranchArrayContainer.GetNewGeoState(geoStateIndex);
  geoState->InitFromNavigator(gGeoManager->GetCurrentNavigator());
  status->fGeoStateIndex = geoStateIndex;

  // Push only the particle ID
  if(fParticles[trackId]->IsPrimary()) {
    fStacks[engineTargetId]->PushPrimaryTrackId(trackId);
  } else {
    fStacks[engineTargetId]->PushSecondaryTrackId(trackId);
  }
  fEngines[engineOriginId]->InterruptTrack();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Transfer track assuming the origin is the current engine
///

void TMCManager::TransferTrack(Int_t engineTargetId)
{
  TransferTrack(fCurrentEngineId, engineTargetId);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Apply something to all engines
///

void TMCManager::Apply(std::function<void(TVirtualMC*)> engineLambda)
{
  for(auto& mc : fEngines) {
    fCurrentEngineId = mc->GetId();
    UpdateEnginePointers();
    Info("Apply", "Apply user lambda for engine %s", fEngines[fCurrentEngineId]->GetName());
    engineLambda(mc);
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Initialize the engines
///

void TMCManager::Init(std::function<void(TVirtualMC*)> initFunction)
{
  if(!fUserStack) {
    Fatal("Run", "Missing user stack pointer.");
  }
  if(fEngines.empty()) {
    Fatal("Run", "No engines registered");
  }

  for(Int_t i = 0; i < fEngines.size(); i++) {
    // Must have geometry handling via TGeo
    if(!fEngines[i]->IsRootGeometrySupported()) {
      Fatal("Run", "Engine %s does not support geometry built via ROOT's TGeoManager");
    }
    // To be able to forward info to the user stack
    fStacks[i]->SetUserStack(fUserStack);
    // Connect the engine's stack to the centrally managed vectors
    fStacks[i]->ConnectTrackContainers(&fParticles, &fParticlesStatus,
                                       &fBranchArrayContainer, &fTotalNPrimaries,
                                       &fTotalNTracks);
    // Set to current engine and call user init procedure
    fCurrentEngineId = fEngines[i]->GetId();
    UpdateEnginePointers();
    initFunction(fEngines[i]);
  }

  // Initialize the fBranchArrayContainer to manage and cache TGeoBranchArrays
  fBranchArrayContainer.InitializeFromGeoManager(gGeoManager);

  fIsInitialized = kTRUE;

  // Send warning if only one engine ==> overhead
  if(fEngines.size() == 1) {
    Warning("Run", "Only one engine registered. That will lead to overhead in "\
                   "the simulation run due to additional hooks and dispatches "\
                   "to the TMCManager.");
  }
}


////////////////////////////////////////////////////////////////////////////////
///
/// Run the event loop
///

void TMCManager::Run(Int_t nEvents)
{
  if(!fIsInitialized) {
    Fatal("Run", "Engines have not yet been initialized.");
  }

  // Run 1 event nEvents times
  for(Int_t i = 0; i < nEvents; i++) {
    Info("Run", "Start event %i", i+1);
    PrepareNewEvent();
    fApplication->BeginEvent();
    // Loop as long as there are tracks in any engine stack
    while(GetNextEngine()) {
      fEngines[fCurrentEngineId]->ProcessEvent(i, kTRUE);
    }
    fApplication->FinishEvent();
  }
  TerminateRun();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Choose next engines to be run in the loop
///

void TMCManager::PrepareNewEvent()
{
  fBranchArrayContainer.FreeGeoStates();
  // Reset in event flag for all engines and clear stacks
  for(Int_t i = 0; i < fStacks.size(); i++) {
    fStacks[i]->ResetInternals();
  }
  for(Int_t i = 0; i < fParticles.size(); i++) {
    if(fParticlesStatus[i]) {
      delete fParticlesStatus[i];
      fParticlesStatus[i] = nullptr;
    }
    fParticles[i] = nullptr;
  }

  // GeneratePrimaries centrally
  fApplication->GeneratePrimaries();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Choose next engines to be run in the loop
///

Bool_t TMCManager::GetNextEngine()
{
  // TODO Can the selection of the next engine be more efficient?
  for(Int_t i = 0; i < fStacks.size(); i++) {
    if(fStacks[i]->GetStackedNtrack() > 0) {
      fCurrentEngineId = i;
      UpdateEnginePointers();
      return kTRUE;
    }
  }
  // No tracks to be processed.
  return kFALSE;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Update all engine pointers connected to the TMCManager
///

void TMCManager::UpdateEnginePointers()
{
  for(TVirtualMC**& mcPtr : fConnectedEnginePointers) {
    *mcPtr = fEngines[fCurrentEngineId];
  }
  // Make sure TVirtualMC::GetMC() returns the current engine.
  TVirtualMC::fgMC = fEngines[fCurrentEngineId];
}

////////////////////////////////////////////////////////////////////////////////
///
/// Terminate the run for all engines
///

void TMCManager::TerminateRun()
{
  for(auto& mc : fEngines) {
    mc->TerminateRun();
  }
}
