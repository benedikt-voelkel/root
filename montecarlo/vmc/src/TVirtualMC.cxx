// @(#)root/vmc:$Id$
// Authors: Ivana Hrivnacova, Rene Brun , Federico Carminati 13/04/2002

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include <iostream>

#include "TVirtualMC.h"
#include "TVirtualMCApplication.h"
#include "TMCManager.h"


/** \class TVirtualMC
    \ingroup vmc

Abstract Monte Carlo interface

Virtual MC provides a virtual interface to Monte Carlo.
It enables the user to build a virtual Monte Carlo application
independent of any actual underlying Monte Carlo implementation itself.

A user will have to implement a class derived from the abstract
Monte Carlo application class, and provide functions like
ConstructGeometry(), BeginEvent(), FinishEvent(), ... .
The concrete Monte Carlo (Geant3, Geant4) is selected at run time -
when processing a ROOT macro where the concrete Monte Carlo is instantiated.
*/

ClassImp(TVirtualMC);

////////////////////////////////////////////////////////////////////////////////
///
/// Standard constructor
///
TVirtualMC::TVirtualMC(const char *name, const char *title,
                       Bool_t /*isRootGeometrySupported*/)
  : TNamed(name,title),
    fApplication(0),
    fQueue(0),
    fDecayer(0),
    fRandom(0),
    fMagField(0)
{
      fApplication = TVirtualMCApplication::Instance();

      if (!fApplication) {
         Error("TVirtualMC", "No user MC application is defined.");
      }
      TMCManager::Instance()->RegisterMC(this);

      fRandom = gRandom;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMC::TVirtualMC()
  : TNamed(),
    fApplication(0),
    fQueue(0),
    fDecayer(0),
    fRandom(0),
    fMagField(0)
{
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TVirtualMC::~TVirtualMC()
{
  std::cout << "VMC is destrcuted" << std::endl;
}

//
// methods
//

////////////////////////////////////////////////////////////////////////////////
///
/// Static access method
///

TVirtualMC* TVirtualMC::GetMC() {
   return TMCManager::GetMC();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set particles stack.
///

void TVirtualMC::SetQueue(TMCQueue* queue)
{
   fQueue = queue;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set external decayer.
///

void TVirtualMC::SetExternalDecayer(TVirtualMCDecayer* decayer)
{
   fDecayer = decayer;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set random number generator.
///

void TVirtualMC::SetRandom(TRandom* random)
{
   gRandom = random;
   fRandom = random;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set magnetic field.
///

void TVirtualMC::SetMagField(TVirtualMagField* field)
{
   fMagField = field;
}
