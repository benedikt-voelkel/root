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
#include "TMCManagerSingle.h"
#include "TError.h"
#include "TVirtualMCApplication.h"

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
   // This creates a basic manager
   new TMCManagerSingle(this);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCApplication::TVirtualMCApplication()
  : TNamed()
{
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TVirtualMCApplication::~TVirtualMCApplication()
{
}

////////////////////////////////////////////////////////////////////////////////
///
/// Static access method
///

TVirtualMCApplication* TVirtualMCApplication::Instance()
{
  return TVirtualMCManager::GetApplicationStatic();
}
