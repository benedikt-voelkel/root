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

#include "TVirtualMCApplication.h"
#include "TMCStateManager.h"
#include "TMCStackManager.h"
#include "TMCManager.h"
#include "TError.h"
#include "TGeoManager.h"

/** \class TVirtualMCApplication
    \ingroup vmc

Interface to a user Monte Carlo application.

*/

ClassImp(TVirtualMCApplication);

TMCThreadLocal TVirtualMCApplication* TVirtualMCApplication::fgInstance = 0;

////////////////////////////////////////////////////////////////////////////////
///
/// Standard constructor
///

TVirtualMCApplication::TVirtualMCApplication(const char *name,
                                             const char *title)
  : TNamed(name,title)
{
   if (fgInstance) {
      Fatal("TVirtualMCApplication",
            "Attempt to create two instances of singleton.");
   }
   fgInstance = this;
   fMCManager = TMCManager::Instance();
   fMCStackManager = TMCStackManager::Instance();
   fMCStateManager = TMCStateManager::Instance();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCApplication::TVirtualMCApplication()
  : TNamed()
{
   fgInstance = this;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TVirtualMCApplication::~TVirtualMCApplication()
{
   fgInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Wrapping the geometry construction independent of the engines
///

void TVirtualMCApplication::ConstructUserGeometry()
{
  ConstructGeometry();
  // Set top volume and close Root geometry if not yet done
  if ( ! gGeoManager->IsClosed() ) {
    TGeoVolume *top = (TGeoVolume*)gGeoManager->GetListOfVolumes()->First();
    gGeoManager->SetTopVolume(top);
    gGeoManager->CloseGeometry();
  }
  MisalignGeometry();
  ConstructOpGeometry();
}

void TVirtualMCApplication::GimmePrimaries()
{
  // Only push new primaries to the VMC stack if the TMCManager calls this method
  // If called by an engine, nothing happens.
  // Correct forwrding of primaries to the engines is done by the TMCManager together
  // with the TMCStackManager
  if(fMCManager->NeedPrimaries()) {
    GeneratePrimaries();
    fMCStackManager->InitializeQueuesWithPrimaries();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Static access method
///

TVirtualMCApplication* TVirtualMCApplication::Instance()
{
  return fgInstance;
}
