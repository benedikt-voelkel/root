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

#include "TMCManager.h"
#include "TVirtualMC.h"
#include "TVirtualMCApplication.h"
#include "TMCStateManager.h"
#include "TMCStackManager.h"
#include "TError.h"
#include "TGeoManager.h"

/** \class TMCManager
    \ingroup vmc

Interface to a user Monte Carlo application.

*/

ClassImp(TMCManager);

TMCThreadLocal TMCManager* TMCManager::fgInstance = 0;
TVirtualMC* TMCManager::fCurrentMCEngine = nullptr;

////////////////////////////////////////////////////////////////////////////////
///
/// Standard/default constructor
///

TMCManager::TMCManager()
{
   if (fgInstance) {
      Fatal("TMCManager",
            "Attempt to create two instances of singleton.");
   }
   fMCApplication = TVirtualMCApplication::Instance();
   if(!fMCApplication) {
     Fatal("TMCManager", "No user MC application is defined.");
   }
   fMCStackManager = TMCStackManager::Instance();
   fMCStateManager = TMCStateManager::Instance();
   fNEventsProcessed = 0;
   // Set true byb default. If in concurrent mode, the TMCManager will take care
   // that primaries are only generated once per event using this flag.
   fNeedPrimaries = kTRUE;

   fgInstance = this;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TMCManager::~TMCManager()
{
   fgInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Register a new VMC
///

void TMCManager::RegisterMC(TVirtualMC* newMC)
{
  // Only allow for one engine to be registered if in concurrent mode
  if(!fMCStateManager->GetConcurrentMode() && fCurrentMCEngine) {
    Fatal("RegisterMC", "Trying to register another engine while not in concurrent mode");
  }
  // First set the current engine
  fCurrentMCEngine = newMC;
  // Update values of all user connected TVirtualMC pointers
  UpdateConnectedEnginePointers();
  // Set queue for this engine
  fMCStackManager->SetQueue(newMC);
  // If not in cincurrent mode, nothing else to do
  if(!fMCStateManager->GetConcurrentMode()) {
    return;
  }
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
/// Set all pointers registered to current engine
///

void TMCManager::UpdateConnectedEnginePointers()
{
  for(auto& mcaddress : fOutsideMCPointerAddresses) {
    *mcaddress = fCurrentMCEngine;
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Terminate the run for all engines
///

void TMCManager::TerminateRun()
{
  for(auto& mc : fMCEngines) {
    mc->TerminateRun();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return current pointer to current engine
///

TVirtualMC* TMCManager::GetMC()
{
   return fCurrentMCEngine;
}

void TMCManager::ConnectToCurrentMC(TVirtualMC*& mc)
{
  fOutsideMCPointerAddresses.push_back(&mc);
  // IF there is already an engine registered, already set it
  if(fCurrentMCEngine) {
    mc = fCurrentMCEngine;
  }
}

Bool_t TMCManager::GetNextEngine()
{
  // If there are still tracks on the current stack do nothing
  if(fCurrentMCEngine->GetQueue()->GetNtrack() > 0) {
    Info("GetNextEngine", "There are still %i particles in queue of %s", fCurrentMCEngine->GetQueue()->GetNtrack(), fCurrentMCEngine->GetName());
    return kTRUE;
  }
  // \note Kind of brute force selection
  for(auto& mc : fMCEngines) {
    if(mc->GetQueue()->GetNtrack() > 0) {
      fCurrentMCEngine = mc;
      Info("GetNextEngine", "There are %i particles in queue of %s", fCurrentMCEngine->GetQueue()->GetNtrack(), fCurrentMCEngine->GetName());
      return kTRUE;
    }
  }
  // All stacks have no tracks to be processed
  return kFALSE;
}

Bool_t TMCManager::NeedPrimaries() const
{
  return fNeedPrimaries;
}

//__________________________________________________________________________
void TMCManager::InitMCs()
{
  // Only available in concurrent mode, abort if not the case
  if(!fMCStateManager->GetConcurrentMode()) {
    Fatal("InitMCs", "Not in concurrent mode");
  }
  // Some user pre init steps
  // At this point the TGeoManager must be there with closed geometry
  if(!gGeoManager || !gGeoManager->IsClosed()) {
    Fatal("InitMCs","Could not find TGeoManager or geometry is still not closed");
  }

  // Initialize engines
  for(auto& mc : fMCEngines) {
    Info("InitMCs", "Initialize engine %s", mc->GetName());
    // Notify to use geometry built using TGeo
    mc->SetRootGeometry();
    // Further init steps for the MCs
    mc->Init();
    mc->BuildPhysics();
  }
}

//__________________________________________________________________________
void TMCManager::RunMCs(Int_t nofEvents)
{
  /// Run MC.
  /// \param nofEvents Number of events to be processed

  // Only available in concurrent mode, abort if not the case
  if(!fMCStateManager->GetConcurrentMode()) {
    Fatal("RunMCs", "Not in concurrent mode");
  }
  // Some user code
  //PreRun();
  // First see list of TGeoNavigator objects registered to TGeoManager
  Info("RunMCs", "There are %i navigators registered to TGeoManager", gGeoManager->GetListOfNavigators()->GetEntries());
  // Run 1 event nofEvents times to cover TGeant3 and TGeant4 per event
  if(nofEvents < 1) {
    //fDryRun = kTRUE;
    Info("RunMCs", "Starting dry run.");
  } else {
    for(Int_t i = 0; i < nofEvents; i++) {
      Info("RunMCs", "Start event %i", i);
      // Generate primaries according to the user to fill the stack which was
      // registered to the TMCStackManager before
      // \todo Rather use a global state manager to handle that
      fNeedPrimaries = kTRUE;
      fMCApplication->GimmePrimaries();
      // Important to set flag to false so that the call from the engines to TVirtualMCApplication::GimmePrimaries will be ignored
      fNeedPrimaries = kFALSE;
      /// Basically forward what is on the stack to the queues of the single engines
      if(!fMCStackManager->HasPrimaries()) {
        Warning("RunMCs", "No primaries found for event %i. Skip and try next event", i);
        continue;
      }
      while(GetNextEngine()) {
        Info("RunMCs", "Running engine %s", fCurrentMCEngine->GetName());
        // \note experimental Update the address for all pointers the user has registered
        // for accessing the current engine
        UpdateConnectedEnginePointers();
        fCurrentMCEngine->ProcessEvent(i);
      }
      fNEventsProcessed++;
    }
  }
  TerminateRun();
  Print();
  fMCStackManager->Print();
  //PostRun();
}

void TMCManager::Stepping()
{
  Info("Stepping", "Stepping for engine %s", fCurrentMCEngine->GetName());
  // Only for more than 1 engine \todo Use a flag for this
  if(fMCEngines.size() > 1) {
    fMCStackManager->SuggestTrackForMoving(fCurrentMCEngine);
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Print status of TMCManager and registered engines
///

void TMCManager::Print() const
{
  Info("Print", "Status of registered engines");
  for(auto& mc : fMCEngines) {
    std::cout << "==== Engine: " << mc->GetName() << "====\n";
  }
  std::cout << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Static access method
///

TMCManager* TMCManager::Instance()
{
  if(!fgInstance) {
    new TMCManager();
  }
  return fgInstance;
}
