// @(#)root/vmc:$Id$
// Authors: Benedikt Volkel, 14/11/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TVirtualMCFastSim
#define ROOT_TVirtualMCFastSim

// Class TVirtualMCFastSim
// ---------------------
// Abstract class for fast simulation engines.
//

#include <vector>

#include "Rtypes.h"

#include "TMCSimpleStack.h"

class TTrack;

class TVirtualMCFastSim
{
  public:
    /// Default constructor
    TVirtualMCFastSim() = default;

    /// Destructor
    virtual ~TVirtualMCFastSim() = default;

    /// Compute fast simulation, maybe alter incoming track and produce
    /// secondaries if wanted.
    virtual void Compute(TTrack* track, TMCSimpleStack<TTrack*>& secondaries) = 0;
};

#endif /* ROOT_TVirtualMCFastSim */
