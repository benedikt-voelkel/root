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

#include "TVirtualMCStack.h"
#include "TParticle.h"
#include "TVector3.h"

/** \class TVirtualMCStack
    \ingroup vmc

Interface to a user defined particles stack.
*/

ClassImp(TVirtualMCStack);

////////////////////////////////////////////////////////////////////////////////
/// Default constructor

TVirtualMCStack::TVirtualMCStack()
  : TObject()
{}

////////////////////////////////////////////////////////////////////////////////
/// Destructor

TVirtualMCStack::~TVirtualMCStack()
{}

TParticle* TVirtualMCStack::PushTrack(Int_t toBeDone, Int_t parent, TParticle* particle,
                                Double_t tof, TMCProcess mech, Int_t& ntr, Int_t is)
{
  TVector3 v;
  particle->GetPolarisation(v);
  return PushTrack(toBeDone, parent, particle->GetPdgCode(), particle->Px(), particle->Py(),
            particle->Pz(), particle->Energy(), particle->Vx(), particle->Vy(), particle->Vz(),
            tof, v.X(), v.Y(), v.Z(), mech, ntr, particle->GetWeight(), is);
}
