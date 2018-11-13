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
#include "TVirtualMCMultiStack.h"
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
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCApplication::TVirtualMCApplication()
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
/// For backwards compatibility provide a static GetMC method
///

TVirtualMC* TVirtualMCApplication::GetMCStatic()
{
  return fMC;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return the current transport engine in use
///

TVirtualMC* TVirtualMCApplication::GetMC() const
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

////////////////////////////////////////////////////////////////////////////////
///
/// Register a VMC to this TVirtualMCApplication.
///

void TVirtualMCApplication::RegisterMC(TVirtualMC* mc)
{
  // If there is already a transport engine, fail since only one is allowed.
  if(fMC) {
    Fatal("RegisterMC", "Attempt to register a second TVirtualMC which " \
                        "is not allowed");
  }
  fMC = mc;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Do all initialisation steps at once
///
// \note \todo Do this via a lambda given by the user?
void TVirtualMCApplication::InitTransport()
{
  // Some user pre init steps
  // At this point the TGeoManager must be there with closed geometry
  if(!gGeoManager || !gGeoManager->IsClosed()) {
    Fatal("InitMCs","Could not find TGeoManager or geometry is still not closed");
  }

  Info("InitMCs", "Initialize engine %s", fMC->GetName());
  // Notify to use geometry built using TGeo
  fMC->SetRootGeometry();
  // Further init steps for the MCs
  fMC->Init();
  fMC->BuildPhysics();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Run the transport by steering engines
///

void TVirtualMCApplication::RunTransport(Int_t nofEvents)
{
  // Check dryrun, so far nothing is done.
  if(nofEvents < 1) {
    Info("RunMCs", "Starting dry run.");
    return;
  }
  fMC->ProcessRun(nofEvents);
}
