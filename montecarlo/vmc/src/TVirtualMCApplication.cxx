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
#include "TMCStackManager.h"
#include "TVirtualMC.h"
#include "TGeoManager.h"

/** \class TVirtualMCApplication
    \ingroup vmc

Interface to a user Monte Carlo application.

*/

ClassImp(TVirtualMCApplication);

TMCThreadLocal TVirtualMCApplication* TVirtualMCApplication::fgInstance = 0;
TMCThreadLocal TVirtualMC* TVirtualMCApplication::fMC = 0;

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
   // There cannot be a TVirtualMC since it must have registered to this
   // TVirtualMCApplication
   fMC = nullptr;
   // Get the TMCStackManager
   fMCStackManager = TMCStackManager::Instance();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCApplication::TVirtualMCApplication()
  : TNamed(), fMCStackManager(TMCStackManager::Instance())
{
   fgInstance = this;
   fMC = nullptr;
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
/// Static access method
///

TVirtualMCApplication* TVirtualMCApplication::Instance()
{
  return fgInstance;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return pointer to the TMCStackManager
///

TMCStackManager* TVirtualMCApplication::GetStackManager() const
{
  return fMCStackManager;
}

////////////////////////////////////////////////////////////////////////////////
///
/// For backwards compatibility provide a static GetMC method.
///

TVirtualMC* TVirtualMCApplication::GetMC()
{
  return fMC;
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
