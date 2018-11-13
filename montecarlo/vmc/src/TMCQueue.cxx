// @(#)root/vmc:$Id$
// Author: Benedikt Volkel, 30/10/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TMCQueue.h"
#include "TTrack.h"
#include "TVector3.h"

/** \class TMCQueue
    \ingroup vmc

Queue a TVirtualMC pops its tracks from
*/

ClassImp(TMCQueue);

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TMCQueue::TMCQueue()
{}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TMCQueue::~TMCQueue()
{
  /// Warn if there are still tracks for potential transport
  if(fTracks.size() > 0) {
    Warning("~TMCQueue", "There were still %i tracks on the queue", Int_t(fTracks.size()));
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Push a track
///

void TMCQueue::PushTrack(TTrack* track)
{
  fTracks.push(track);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Total number of tracks
///

Int_t TMCQueue::GetNtrack() const
{
  return fTracks.size();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Pop next track to be processed
///

TTrack* TMCQueue::PopNextTrack()
{
  if (fTracks.empty()) {
    return nullptr;
  }

  TTrack* track = fTracks.front();
  fTracks.pop();

  return track;
}
