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

#include "TVirtualMCManager.h"
#include "TError.h"
#include "TVirtualMC.h"
#include "TVirtualMCApplication.h"
#include "TParticle.h"
#include "TVector3.h"
#include "TLorentzVector.h"

/** \class TVirtualMCManager
    \ingroup vmc

Interface to a user Monte Carlo application.

*/

ClassImp(TVirtualMCManager);

TMCThreadLocal TVirtualMCManager* TVirtualMCManager::fgInstance = 0;
TMCThreadLocal TVirtualMCApplication* TVirtualMCManager::fApplicationBase = 0;
TMCThreadLocal TVirtualMC* TVirtualMCManager::fMC = 0;

////////////////////////////////////////////////////////////////////////////////
///
/// Standard constructor
///

TVirtualMCManager::TVirtualMCManager(TVirtualMCApplication* application)
{
   // If a user accidentally gives a nullptr of right type
  if(!application) {
    Fatal("TVirtualMCManager", "Don't accept nullptr for TVirtualMCApplication");
  }
  // Now this can happen in case a user instaciates a specialised version
  // of a TVirtualMCManagerSingle<A> or TVirtualMCManagerMulti<A>
  if (fgInstance) {
    delete fgInstance;
  } else {
    fMC = nullptr;
  }
  if(application != fApplicationBase && fApplicationBase) {
    Warning("TVirtualMCManager", "Your TVirtualMCApplication has changed. It is deleted and replaced");
    delete fApplicationBase;
  }
  fgInstance = this;
  fApplicationBase = application;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCManager::TVirtualMCManager()
{
  if (fgInstance) {
    delete fgInstance;
  }
  fgInstance = this;
  fMC = nullptr;
  fApplicationBase = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TVirtualMCManager::~TVirtualMCManager()
{
   fgInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Static access method
///

TVirtualMCManager* TVirtualMCManager::Instance()
{
  return fgInstance;
}

////////////////////////////////////////////////////////////////////////////////
///
/// For backwards compatibility provide a static GetMC method
///

TVirtualMC* TVirtualMCManager::GetMCStatic()
{
  return fMC;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return the current transport engine in use
///

TVirtualMC* TVirtualMCManager::GetMC() const
{
  return fMC;
}

////////////////////////////////////////////////////////////////////////////////
///
/// For backwards compatibility provide a static GetMC method
///

TVirtualMCApplication* TVirtualMCManager::GetApplicationStatic()
{
  return fApplicationBase;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return the current transport engine in use
///

TVirtualMCApplication* TVirtualMCManager::GetApplication() const
{
  return fApplicationBase;
}
