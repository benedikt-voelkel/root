// @(#)root/vmc:$Id$
// Authors: Benedikt Volkel, 30/10/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TMCQueue
#define ROOT_TMCQueue

// Class TMCQueue
// ---------------------
// Queue a TVirtualMC pops its tracks from
//

#include <queue>

#include "Rtypes.h"

class TTrack;

class TMCQueue {

/// TMCStackManager is friend so only this can push tracks.
friend class TMCStackManager;

public:
   // Constructor
   TMCQueue();

   // Destructor
   ~TMCQueue();

   /// Total number of tracks
   Int_t GetNtrack() const;

   /// Pop next track to be processed
   const TTrack* PopNextTrack();

 private:
   /// Used to push tracks by the TMCStackManager only
   void PushTrack(TTrack* track);

 private:
   std::queue<TTrack*> fTracks;

   ClassDefNV(TMCQueue,1) //Interface to a particles stack
};

#endif // ROOT_TMCQueue
