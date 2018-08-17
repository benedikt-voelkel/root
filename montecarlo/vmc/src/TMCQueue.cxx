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

#include "TMCQueue.h"
#include "TParticle.h"
#include "TVector3.h"

/** \class TMCQueue
    \ingroup vmc

Queue an engine can pop particles from for tracking.
*/

ClassImp(TMCQueue);

////////////////////////////////////////////////////////////////////////////////
/// Default constructor

TMCQueue::TMCQueue()
  : TObject()
{}

////////////////////////////////////////////////////////////////////////////////
/// Destructor

TMCQueue::~TMCQueue()
{
  if(fTracks.size() > 0) {
    Warning("~TMCQueue", "There were still %i tracks on the queue", Int_t(fTracks.size()));
  }
}

//_____________________________________________________________________________
void TMCQueue::PushTrack(TParticle* particle)
{
  fTracks.push(particle);
}

//_____________________________________________________________________________
Int_t TMCQueue::GetNtrack() const
{
  return fTracks.size();
}

//_____________________________________________________________________________
TParticle* TMCQueue::PopNextTrack()
{
/// Get next particle for tracking from the stack.
/// \return        The popped particle object

  if (fTracks.empty()) {
    return nullptr;
  }

  TParticle* particle = fTracks.front();
  fTracks.pop();

  if (!particle) {
    return nullptr;
  }
  return particle;
}
