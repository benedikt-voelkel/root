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

#include "TVirtualMCConcurrentApplication.h"
#include "TMCManager.h"
#include "TMCStateManager.h"
#include "TMCStackManager.h"

/** \class TVirtualMCConcurrentApplication
    \ingroup vmc

Interface to a user Monte Carlo application.

*/

ClassImp(TVirtualMCConcurrentApplication);

////////////////////////////////////////////////////////////////////////////////
///
/// Standard constructor
///

TVirtualMCConcurrentApplication::TVirtualMCConcurrentApplication(const char *name,
                                             const char *title)
  : TVirtualMCApplication(name, title)
{
   fMCStateManager->SetConcurrentMode();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCConcurrentApplication::TVirtualMCConcurrentApplication()
  : TVirtualMCApplication()
{
   fMCStateManager->SetConcurrentMode();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Further steps for geometry initialisation
///

void TVirtualMCConcurrentApplication::InitGeometry()
{
  // Check conditions and then call the BeginEventConcurrent()
  InitGeometryConcurrent();
  return;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Begin of an event
///

void TVirtualMCConcurrentApplication::BeginEvent()
{
  // Check conditions and then call the BeginEventConcurrent()
  BeginEventConcurrent();
  return;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Begin transportation of a new primary
///

void TVirtualMCConcurrentApplication::BeginPrimary()
{
  // Check conditions and then call the BeginEventConcurrent()
  BeginPrimaryConcurrent();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Before a new track is transported
///

void TVirtualMCConcurrentApplication::PreTrack()
{
  // Check conditions and then call the BeginEventConcurrent()
  PreTrackConcurrent();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Also invoke the stepping of the TMCManager
///

void TVirtualMCConcurrentApplication::Stepping()
{
  // Call the user stepping actions
  //Info("Stepping", "Do custom user stepping");
  SteppingConcurrent();
  // After that the Stepping of the TMCManager must be called e.g. to check whether
  // stacks must be updated
  //Info("Stepping", "Do stepping of TMCManager");
  fMCManager->Stepping();
}

////////////////////////////////////////////////////////////////////////////////
///
/// After a track is transported
///

void TVirtualMCConcurrentApplication::PostTrack()
{
  // Check conditions and then call the BeginEventConcurrent()
  PostTrackConcurrent();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Specific post processing when a primary is finished
///

void TVirtualMCConcurrentApplication::FinishPrimary()
{
  // Check conditions and then call the BeginEventConcurrent()
  FinishPrimaryConcurrent();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Specific post processing when a primary is finished
///

void TVirtualMCConcurrentApplication::FinishEvent()
{
  // Check conditions and then call the BeginEventConcurrent()
  FinishEventConcurrent();
}
