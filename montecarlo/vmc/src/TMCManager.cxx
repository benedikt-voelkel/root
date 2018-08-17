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

#include "TMCManager.h"
#include "TVirtualMC.h"
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
  : TObject()
{
   if (fgInstance) {
      Fatal("TMCManager",
            "Attempt to create two instances of singleton.");
   }
   fMCStackManager = TMCStackManager::Instance();
   fNEventsProcessed = 0;
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

void TMCManager::RegisterMC(TVirtualMC* mc)
{
  // make sure, at least engine names are unique
  for(auto& con : fMCContainers) {
    if(strcmp(con->fName, mc->GetName()) == 0) {
      Fatal("RegisterMC", "There is already an engine with name %s.", mc->GetName());
    }
  }
  // Push back a new MCContainer
  TMCContainer* container = new TMCContainer(mc);

  fMCContainers.push_back(container);
  // And provide static pointers to current container and engine
  fCurrentMCContainer = fMCContainers.back();
  fCurrentMCEngine = mc;
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

////////////////////////////////////////////////////////////////////////////////
///
/// Return pointer to engine chosen by name
///

TVirtualMC* TMCManager::GetMC(const char* name)
{
  for(auto con : fMCContainers) {
    if(strcmp(con->fName, name) == 0) {
      return con->fMC;
    }
  }
  Fatal("GetMC", "There is no engine with name %s.", name);
  return nullptr;
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
    return kTRUE;
  }
  // \note Kind of brute force selection
  for(auto& con : fMCContainers) {
    if(con->fQueue->GetNtrack() > 0) {
      fCurrentMCContainer = con;
      return kTRUE;
    }
  }
  // All stacks have no tracks to be processed
  return kFALSE;
}

//__________________________________________________________________________
void TMCManager::InitMCs()
{
  // Some user pre init steps
  // At this point the TGeoManager must be there with closed geometry
  if(!gGeoManager || !gGeoManager->IsClosed()) {
    Fatal("InitMCs","Could not find TGeoManager or geometry is still not closed");
  }

  // Initialize containers
  for(auto& con : fMCContainers) {
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
      /// Basically forward what is on the stack to the queues of the single engines
      if(!fMCStackManager->InitializeQueuesForPrimaries()) {
        Warning("RunMCs", "No primaries found for event %i. Skip and try next event", i);
        continue;
      }
      while(GetNextEngine()) {
        Info("RunMCs", "Running engine %s", fCurrentMCEngine->GetName());
        // \note experimental
        for(auto& mcaddress : fOutsideMCPointerAddresses) {
          *mcaddress = fCurrentMCEngine;
        }
        fCurrentMCEngine->ProcessEvent(i);
      }
      fNEventsProcessed++;
    }
  }
  TerminateRun();
  //PostRun();
}

void TMCManager::Stepping()
{
  fMCStackManager->SuggestTrackForMoving(fCurrentMCContainer);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Static access method
///

TMCManager* TMCManager::Instance()
{
  if(fgInstance) {
    new TMCManager();
  }
  return fgInstance;
}
