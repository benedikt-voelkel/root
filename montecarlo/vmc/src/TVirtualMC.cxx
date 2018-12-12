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

#include "TVirtualMC.h"

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
    fStack(0),
    fDecayer(0),
    fRandom(0),
    fMagField(0)
{
    fApplication = TVirtualMCApplication::Instance();

    if (!fApplication) {
       Error("TVirtualMC", "No user MC application is defined.");
    }
    // Register this VMC to the running TVirtualMCApplication and get
    // assigned ID
    fApplication->RegisterMC(this);
    fRandom = gRandom;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMC::TVirtualMC()
  : TNamed(),
    fApplication(0),
    fStack(0),
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
}

//
// methods
//

////////////////////////////////////////////////////////////////////////////////
///
/// Static access method
///

TVirtualMC* TVirtualMC::GetMC() {
   return TVirtualMCApplication::GetMCStatic();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Support this method for backwards compatibility in case the user wants to
/// use this interface
///

void TVirtualMC::TrackPosition(Double_t &x, Double_t &y, Double_t &z,
                               Double_t &t) const {
   t = TrackTime();
   TrackPosition(x, y, z);
 }

////////////////////////////////////////////////////////////////////////////////
///
/// Support this method for backwards compatibility in case the user wants to
/// use this interface
///

void TVirtualMC::TrackPosition(Float_t &x, Float_t &y, Float_t &z,
                               Float_t &t) const {
   t = static_cast<Float_t>(TrackTime());
   TrackPosition(x, y, z);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set particles stack.
///

void TVirtualMC::SetStack(TVirtualMCStack* stack)
{
   fStack = stack;
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

////////////////////////////////////////////////////////////////////////////////
///
/// Set the VMC id
///

void TVirtualMC::SetId(UInt_t id)
{
   fId = id;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Process one event (backwards compatibility)
///

void TVirtualMC::ProcessEvent()
{
   Info("ProcessEvent", "Not implemented.");
}

////////////////////////////////////////////////////////////////////////////////
///
/// Process one event (backwards compatibility)
///

void TVirtualMC::ProcessEvent(Int_t eventId)
{
   ProcessEvent();
}
