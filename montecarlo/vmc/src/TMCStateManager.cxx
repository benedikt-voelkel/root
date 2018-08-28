// @(#)root/vmc:$Id$
// Authors: Benedikt Volkel 06/08/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
#include "TError.h"

#include "TMCStateManager.h"

/** \class TMCStateManager
    \ingroup vmc

Singleton for global state management

*/

ClassImp(TMCStateManager);

TMCThreadLocal TMCStateManager* TMCStateManager::fgInstance = 0;

TMCStateManager::TMCStateManager()
{
  fgInstance = this;
  // Set the non-concurrent mode as default
  fIsConcurrentMode = kFALSE;
}

void TMCStateManager::EnterState(EVMCApplicationState state)
{
  // remember that state...
  fProcessedStates.push_back(fCurrentState);
  // ... and update to new one
  fCurrentState = state;
}

EVMCApplicationState TMCStateManager::GetCurrentState() const
{
  return fCurrentState;
}

void TMCStateManager::RequireState(EVMCApplicationState state) const
{
  if(fCurrentState != state) {
    Fatal("TMCStateManager::RequireState", "Wrong state... Exit");
    exit(1);
  }
}

void TMCStateManager::SetConcurrentMode(Bool_t isConcurrentMode)
{
  fIsConcurrentMode = isConcurrentMode;
}

Bool_t TMCStateManager::GetConcurrentMode() const
{
  return fIsConcurrentMode;
}

TMCStateManager* TMCStateManager::Instance()
{
  if(!fgInstance) {
    new TMCStateManager();
  }
  return fgInstance;
}
