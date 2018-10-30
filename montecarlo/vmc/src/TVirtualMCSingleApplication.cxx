// @(#)root/vmc:$Id$
// Author: Benedikt Volkel, 30/10/1018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TVirtualMCSingleApplication.h"
#include "TMCStackManager.h"
#include "TVirtualMC.h"
#include "TError.h"

/** \class TVirtualMCSingleApplication
    \ingroup vmc

Interface to a user Monte Carlo application using a single transport engine
for the entire simulation setup.

*/

ClassImp(TVirtualMCSingleApplication);

////////////////////////////////////////////////////////////////////////////////
///
/// Standard constructor
///

TVirtualMCSingleApplication::TVirtualMCSingleApplication(const char *name,
                                                         const char *title)
  : TVirtualMCApplication(name,title)
{
  // Just giving the TMCStackManager the pointer to the only engine there is.
  fMCStackManager->RegisterSpecifyEngineForTrack(
                              [](TTrack* track, TVirtualMC*& mc) {mc = fMC;});
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCSingleApplication::TVirtualMCSingleApplication()
  : TVirtualMCApplication()
{}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TVirtualMCSingleApplication::~TVirtualMCSingleApplication()
{}

////////////////////////////////////////////////////////////////////////////////
///
/// Register a VMC to this TVirtualMCApplication.
///

void TVirtualMCSingleApplication::RegisterMC(TVirtualMC* mc)
{
  // If there is already a transport engine, fail since only one is allowed.
  if(fMC) {
    Fatal("RegisterMC", "Attempt to register a second TVirtualMC which " \
                        "is not allowed");
  }
  fMC = mc;
}
