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

TMCThreadLocal TVirtualMC* TVirtualMC::fgMC = 0;
////////////////////////////////////////////////////////////////////////////////
///
/// Standard constructor
///

TVirtualMC::TVirtualMC(const char *name, const char *title,
                       Bool_t /*isRootGeometrySupported*/)
  : TNamed(name,title),
    fApplication(0),
    fId(0),
    fStack(0),
    fManagerStack(0),
    fDecayer(0),
    fRandom(0),
    fMagField(0),
    fUseExternalGeometryConstruction(kFALSE),
    fUseExternalParticleGeneration(kFALSE)
{
    fApplication = TVirtualMCApplication::Instance();

    if (!fApplication) {
       Fatal("TVirtualMC", "No user MC application is defined.");
    }


    fApplication->Register(this);
    fgMC = this;
    fRandom = gRandom;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMC::TVirtualMC()
  : TNamed(),
    fApplication(0),
    fId(0),
    fStack(0),
    fManagerStack(0),
    fDecayer(0),
    fRandom(0),
    fMagField(0),
    fUseExternalGeometryConstruction(kFALSE),
    fUseExternalParticleGeneration(kFALSE)
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
   return fgMC;
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
/// Process one event (backwards compatibility)
///

void TVirtualMC::ProcessEvent()
{
   Warning("ProcessEvent", "Not implemented.");
}

////////////////////////////////////////////////////////////////////////////////
///
/// Process one event (backwards compatibility)
///

void TVirtualMC::ProcessEvent(Int_t eventId)
{
   ProcessEvent();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return the current step number
///

Int_t TVirtualMC::StepNumber() const
{
   Warning("StepNumber", "Not implemented.");
   return 0;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get the current weight
///

Double_t TVirtualMC::TrackWeight() const
{
   Warning("Weight", "Not implemented.");
   return 1.;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get the current polarization
///

void TVirtualMC::TrackPolarization(Double_t &polX, Double_t &polY,
                              Double_t &polZ) const
{
   Warning("Polarization", "Not implemented.");
   polX = 0.;
   polY = 0.;
   polZ = 0.;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get the current polarization
///

void TVirtualMC::TrackPolarization(TVector3& pol) const
{
   Warning("Polarization", "Not implemented.");
   pol[0] = 0.;
   pol[1] = 0.;
   pol[2] = 0.;
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
/// Set container holding additional information for transported TParticles
///
void TVirtualMC::SetManagerStack(TMCManagerStack* stack)
{
  fManagerStack = stack;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Disables internal dispatch to TVirtualMCApplication::ConstructGeometry()
/// and hence rely on geometry construction being trigeered from outside.
///

void TVirtualMC::SetExternalGeometryConstruction(Bool_t value)
{
   fUseExternalGeometryConstruction = value;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Disables internal dispatch to TVirtualMCApplication::ConstructGeometry()
/// and hence rely on geometry construction being trigeered from outside.
///

void TVirtualMC::SetExternalParticleGeneration(Bool_t value)
{
   fUseExternalParticleGeneration = value;
}

////////////////////////////////////////////////////////////////////////////////
///
/// An interruptible event can be paused and resumed at any time. It must not
/// call TVirtualMCApplication::BeginEvent() and ::FinishEvent()
/// Further, when tracks are popped from the TVirtualMCStack it must be
/// checked whether these are new tracks or whether they have been
/// transported up to their current point.
///

void TVirtualMC::ProcessEvent(Int_t eventId, Bool_t isInterruptible)
{
   Warning("ProcessInterruptibleEvent", "Not implemented.");
}

////////////////////////////////////////////////////////////////////////////////
///
/// That triggers stopping the transport of the current track without dispatching
/// to common routines like TVirtualMCApplication::PostTrack() etc.
///

void TVirtualMC::InterruptTrack()
{
   Warning("InterruptTrack", "Not implemented.");
}
