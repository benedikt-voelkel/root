// @(#)root/vmc:$Id$
// Authors: Ivana Hrivnacova 13/04/2002

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

// Class TVirtualMCStack
// ---------------------
// Interface to a user defined particles stack.
//

#include <queue>

#include "TObject.h"
#include "TMCProcess.h"

class TTrack;

class TMCQueue {

public:
   // Constructor
   TMCQueue();

   // Destructor
   ~TMCQueue();

   //
   // Methods for stacking
   //
   /// Used to push tracks by the TMCStackManager only
   // \todo Make it private and make TMCStackManager a friend class?
   void PushTrack(TTrack* track);

   /// Only pop tracks, since that's a queue all tracks are supposed to be processed
   const TTrack* PopNextTrack();

   //
   // Get methods
   //

   /// Total number of tracks
   Int_t      GetNtrack()    const;

 private:
   std::queue<TTrack*> fTracks;

   ClassDef(TMCQueue,1) //Interface to a particles stack
};

#endif //ROOT_TMCQueue
