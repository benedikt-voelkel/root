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
#include "TError.h"
#include "TVirtualMC.h"

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
  : TNamed()
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
/// Register a VMC to this TVirtualMCApplication.
///

Int_t TVirtualMCApplication::RegisterMC(TVirtualMC* mc)
{
  // If there is already a transport engine, fail since only one is allowed.
  if(fMC) {
    Fatal("RegisterMC", "Attempt to register a second TVirtualMC which " \
                        "is not allowed");
  }
  fMC = mc;
  // There is only 1 VMC in this case so ID is known to be 0.
  return 0;
}
