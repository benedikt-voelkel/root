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
#include "TTrack.h"
#include "TVector3.h"

/** \class TVirtualMCStack
    \ingroup vmc

Interface to a user defined particles stack.
*/

ClassImp(TVirtualMCStack);

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCStack::TVirtualMCStack()
  : TObject()
{}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TVirtualMCStack::~TVirtualMCStack()
{}

  ////////////////////////////////////////////////////////////////////////////////
  ///
  /// If geometry states are managed by this stack as well, this should be
  /// overriden.
  ///

  void TVirtualMCStack::PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                                  Double_t px, Double_t py, Double_t pz,
                                  Double_t e, Double_t vx, Double_t vy,
                                  Double_t vz, Double_t tof, Double_t polx,
                                  Double_t poly, Double_t polz,
                                  Int_t geoStateIndex, TMCProcess mech,
                                  Int_t& ntr, Double_t weight, Int_t is)
{
  PushTrack(toBeDone, parent, pdg, px, py, pz, e, vx, vy, vz, tof, polx, poly,
            polz, geoStateIndex, mech, ntr, weight, is);
}

////////////////////////////////////////////////////////////////////////////////
///
/// If geometry states are managed by this stack as well, this should be
/// overriden.
///

TParticle* TVirtualMCStack::PopNextTrack(Int_t&  itrack, Int_t& geoStateIndex)
{
  geoStateIndex = -1;
  return PopNextTrack(itrack);
}
