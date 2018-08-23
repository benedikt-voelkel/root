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
#include "TMCStackManager.h"
#include "TMCSelectionCriteria.h"
#include "TMCContainer.h"
#include "TError.h"
#include "TGeoManager.h"

/** \class TMCManager
    \ingroup vmc

Interface to a user Monte Carlo application.

*/

ClassImp(TMCManager);

TMCThreadLocal TMCManager* TMCManager::fgInstance = 0;
TVirtualMC* TMCManager::fCurrentMCEngine = nullptr;
TMCContainer* TMCManager::fCurrentMCContainer = nullptr;

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
   fNEventsProcessed = 0;
   fIsConcurrentMode = kFALSE;
   fNeedPrimaries = kFALSE;

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
/// Enable/Disable the concurrent mode
///
void TMCManager::SetConcurrentMode(Bool_t isConcurrent)
{
  // If there are already engines registered, the mode cannot be changed anymore
  // If there is at least one engine, fCurrentMCEngine is set
  if(fCurrentMCEngine) {
    Fatal("SetConcurrentMode", "Trying to change concurrent mode while there are already registered engines");
  }
  fIsConcurrentMode = isConcurrent;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Register a new VMC
///

void TMCManager::RegisterMC(TVirtualMC* mc)
{
  if(!fIsConcurrentMode && fCurrentMCEngine) {
    Fatal("RegisterMC", "Trying to register another engine while not in concurrent mode");
  }
  // First set the current engine
  fCurrentMCEngine = mc;
  // If not in cincurrent mode, nothing else to do
  if(!fIsConcurrentMode) {
    return;
  }
  // If in concurrent mode, add engine to the list of engines
  // make sure, at least engine names are unique
  for(auto& con : fMCContainers) {
    if(strcmp(con->fName, mc->GetName()) == 0) {
      Fatal("RegisterMC", "There is already an engine with name %s.", mc->GetName());
    }
  }
  // Emplace a new TMCContainer providing the TVirtualMC pointer...
  fMCContainers.push_back(new TMCContainer(mc));
  // ...and provide static pointerto current container in addition to the engine pointer
  fCurrentMCContainer = fMCContainers.back();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Terminate the run for all engines
///

void TMCManager::TerminateRun()
{
  for(auto& con : fMCContainers) {
    con->fMC->TerminateRun();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get the corresponding selection criteria
///

TMCSelectionCriteria* TMCManager::GetSelectionCriteria(const char* name)
{
   for(auto con : fMCContainers) {
     if(strcmp(con->fName, name) == 0) {
       return con->fSelectionCriteria;
     }
   }
   Fatal("GetSelectionCriteria", "There is no engine with name %s.", name);
   return nullptr;
}

TMCSelectionCriteria* TMCManager::GetSelectionCriteria()
{
   return fCurrentMCContainer->fSelectionCriteria;
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
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the current MCContainer by name and pointer value
///

void TMCManager::SetCurrentMCContainer(TMCContainer* container)
{
  for(auto& con : fMCContainers) {
    if(con == container) {
      fCurrentMCContainer = con;
      fCurrentMCEngine = con->fMC;
      return;
    }
  }
}

void TMCManager::SetCurrentMCContainer(const char* name)
{
  for(auto& con : fMCContainers) {
    if(strcmp(con->fName, name) == 0) {
      fCurrentMCContainer = con;
      fCurrentMCEngine = con->fMC;
      return;
    }
  }
}

Bool_t TMCManager::GetNextEngine()
{
  // If there are still tracks on the current stack do nothing
  if(fCurrentMCContainer->fQueue->GetNtrack() > 0) {
    Info("GetNextEngine", "There are still %i particles in queue of %s", fCurrentMCContainer->fQueue->GetNtrack(), fCurrentMCContainer->fName);
    return kTRUE;
  }
  // \note Kind of brute force selection
  for(auto& con : fMCContainers) {
    if(con->fQueue->GetNtrack() > 0) {
      fCurrentMCContainer = con;
      fCurrentMCEngine = fCurrentMCContainer->fMC;
      Info("GetNextEngine", "There are %i particles in queue of %s", fCurrentMCContainer->fQueue->GetNtrack(), fCurrentMCContainer->fName);
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
  // Some user pre init steps
  // At this point the TGeoManager must be there with closed geometry
  if(!gGeoManager || !gGeoManager->IsClosed()) {
    Fatal("InitMCs","Could not find TGeoManager or geometry is still not closed");
  }
  // \todo Check whether the TMCSelectionCriteria have conflicts among each other

  // Initialize containers
  for(auto& con : fMCContainers) {
    Info("InitMCs", "Initialize engine %s", con->fName);
    // Selection criteria
    con->fSelectionCriteria->Initialize(gGeoManager);
    // Notify to use geometry built using TGeo
    con->fMC->SetRootGeometry();
    // create queue for engine container and register container to TMCStackManager
    fMCStackManager->SetQueue(con);
    // Further init steps for the MCs
    con->fMC->Init();
    con->fMC->BuildPhysics();
  }
}

//__________________________________________________________________________
void TMCManager::RunMCs(Int_t nofEvents)
{
  /// Run MC.
  /// \param nofEvents Number of events to be processed

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
      fMCApplication->GeneratePrimaries();
      fNeedPrimaries = kFALSE;
      /// Basically forward what is on the stack to the queues of the single engines
      if(!fMCStackManager->HasPrimaries()) {
        Warning("RunMCs", "No primaries found for event %i. Skip and try next event", i);
        continue;
      }
      while(GetNextEngine()) {
        Info("RunMCs", "Running engine %s", fCurrentMCContainer->fName);
        // \note experimental Update the address for all pointers the user has registered
        // for accessing the current engine
        for(auto& mcaddress : fOutsideMCPointerAddresses) {
          *mcaddress = fCurrentMCContainer->fMC;
        }
        fCurrentMCContainer->fMC->ProcessEvent(i);
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
  Info("Stepping", "Stepping for engine %s", fCurrentMCContainer->fName);
  // Only for more than 1 engine \todo Use a flag for this
  if(fMCContainers.size() > 1) {
    fMCStackManager->SuggestTrackForMoving(fCurrentMCContainer);
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Print status of TMCManager and registered engines
///

void TMCManager::Print() const
{
  Info("Print", "Status of registered engines");
  for(auto& con : fMCContainers) {
    std::cout << "==== Engine: " << con->fName << "====\n";
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
