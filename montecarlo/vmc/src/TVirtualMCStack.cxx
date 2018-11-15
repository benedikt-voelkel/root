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
#include "TError.h"

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

////////////////////////////////////////////////////////////////////////////////
///
/// Interface for multitracking
/// Must be overriden to provide the pointer to the user stacked particle
///

void  TVirtualMCStack::PushTrack(Int_t toBeDone, Int_t parent,
                                 Int_t pdg, Double_t px, Double_t py,
                                 Double_t pz, Double_t e, Double_t vx,
                                 Double_t vy, Double_t vz, Double_t tof,
                                 Double_t polx, Double_t poly,
                                 Double_t polz, Int_t geoStateIndex,
                                 ETrackTransportStatus transportStatus,
                                 TMCProcess mech, Int_t& ntr,
                                 Double_t weight, Int_t is)
{
  // If not overriden but called warn the user.
  Warning("PushTrack", "Your stack does not manage geo state indices and " \
                       "track transport status.");
  PushTrack(toBeDone, parent, pdg, px, py, pz, e, vx, vy, vz, tof, polx, poly,
            polz, mech, ntr, weight, is);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Don't assume that the primary at index i has ID i, secondaries might sit
/// in between
///

TParticle* TVirtualMCStack::PopPrimaryForTracking(Int_t i, Int_t&  itrack)
{
  // That is cheating but the interface itself is incoherently used already.
  // Normally the following cannot be assumed.
  Warning("PopPrimaryForTracking", "The index %i you requested might not be " \
                       "the real index of this track.", i);
  itrack = i;
  return PopPrimaryForTracking(i);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get the geometry state as TGeoBranchArray
///

Int_t TVirtualMCStack::GetCurrentTrackGeoStateIndex() const
{
  // Not supporting geo state indices so just return -1 to ensure
  // backward compatibility
  return -1;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get the geo state index by track ID
///

Int_t TVirtualMCStack::GetTrackGeoStateIndex(Int_t trackId) const
{
  // Not supporting geo state indices so just return -1 to ensure
  // backward compatibility
  return -1;
}
