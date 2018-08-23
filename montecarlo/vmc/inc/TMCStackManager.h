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

#ifndef ROOT_TMCStackManager
#define ROOT_TMCStackManager

// Class TVirtualMCStack
// ---------------------
// Interface to a user defined particles stack.
//
#include "TObject.h"

#include "TMCtls.h"
#include "TMCProcess.h"

class TParticle;
class TMCQueue;
class TVirtualMCStack;
class TMCContainer;
class TVirtualMC;


class TMCStackManager {

public:


   // Destructor
   ~TMCStackManager();

   /// Static access method
   static TMCStackManager* Instance();
   //
   // Methods for stacking
   //

   /// Create a new particle and push into stack;
   /// - toBeDone   - 1 if particles should go to tracking, 0 otherwise
   /// - parent     - number of the parent track, -1 if track is primary
   /// - pdg        - PDG encoding
   /// - px, py, pz - particle momentum [GeV/c]
   /// - e          - total energy [GeV]
   /// - vx, vy, vz - position [cm]
   /// - tof        - time of flight [s]
   /// - polx, poly, polz - polarization
   /// - mech       - creator process VMC code
   /// - ntr        - track number (is filled by the stack
   /// - weight     - particle weight
   /// - is         - generation status code
   TParticle* PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                           Double_t px, Double_t py, Double_t pz, Double_t e,
                           Double_t vx, Double_t vy, Double_t vz, Double_t tof,
                           Double_t polx, Double_t poly, Double_t polz,
                           TMCProcess mech, Int_t& ntr, Double_t weight,
                           Int_t is);

   TParticle* PushTrack(Int_t toBeDone, TParticle* particle, TMCProcess mech, Int_t& ntr);

    /// The outside world might suggest that the track handled by the engine
    /// of the current container needs to be moved
    void SuggestTrackForMoving(TMCContainer* container);

   /// The stack has to provide two pop mechanisms:
   /// The first pop mechanism required.
   /// Pop all particles with toBeDone = 1, both primaries and seconadies
   //virtual TParticle* PopNextTrack(Int_t& itrack) = 0;

   /// The second pop mechanism required.
   /// Pop only primary particles with toBeDone = 1, stacking of secondaries
   /// is done by MC
   //virtual TParticle* PopPrimaryForTracking(Int_t i) = 0;

   //
   // Set methods
   //


   /// Get a notification from a queue or an engine about the tracks which is processed
   //void               NotifyOnProcessedTrack(TParticle* particle);
   //void               NotifyOnProcessedTrack(Int_t trackID);

   //
   // Get methods
   //

   /// Number of tracks still to be done
   Int_t      GetNtrackToDo() const;

   /// Total number of tracks
   Int_t      GetNtrack()    const;

   /// Total number of primary tracks
   Int_t      GetNprimary()  const;

   /// Current track particle
   TParticle* GetCurrentTrack() const { return fCurrentTrack; }

   /// Current track number
   Int_t      GetCurrentTrackNumber() const;

   /// Number of the parent of the current track
   Int_t      GetCurrentParentTrackNumber() const;

   /// Set a  new queue for an engine and register this container
   /// Nobody but the TMCStackManager should ever need to manage a queue and engines
   /// are inly allowed to pop from these
   void SetQueue(TMCContainer* container);

   /// Set the current track id from the outside
   void SetCurrentTrack(Int_t trackID);

   /// Register the master/global stack to the TMCStackManager
   void RegisterStack(TVirtualMCStack* stack);

   /// Forward primaries from the global stack to the engine queues wrt the respective TMCSelectionCriteria
   Bool_t HasPrimaries();

   //
   // Verbosity
   //

   /// Print status of stacks/queues
   void Print() const;
 private:

   /// Constructor prvate for save singleton behaviour
   TMCStackManager();
   /// Push a new track to a queue of the stored TMCContainers
   void PushToQueue(TParticle* track);
   /// Buffer current parameters the selection is based on
   void BufferSelectionParameters(TMCContainer* currentContainer);
   /// Check whether a track meets the selection criteria of a given TMCContainer
   Bool_t TrackFitsExclusively(TMCContainer* container) const;
   Bool_t TrackFitsInclusively(TMCContainer* container) const;
   /// Move a track handled byb an engine to a new target queue
   void MoveTrack(TVirtualMC* currentMC, TMCQueue* targetQueue);

  private:
   // static data members
  #if !defined(__CINT__)
     static TMCThreadLocal TMCStackManager* fgInstance; ///< Singleton instance
  #else
     static                TMCStackManager* fgInstance; ///< Singleton instance
  #endif
    TVirtualMCStack*             fMasterStack;      ///< The master stack with coherent history, this is what the outside world sees
    TParticle*                   fCurrentTrack;     ///< Pointer to current track
    Int_t                        fCurrentTrackID;   ///< Only the track id
    std::vector<TMCContainer*>   fMCContainers;     ///< Store MC containers for stack/queue management based on TMCSelectionCriteria
    // Information for the seletion of engines when moving tracks (or attempt to)
    Int_t                        fCurrentVolId;     ///< Buffer for the current volume id
    Int_t                        fCurrentVolCopyNo; ///< Buffer the copy number of the current volume
    Bool_t                       fLastTrackSuggestedForMoving; ///< Flag the current track since it might need to be moved in the next step

   ClassDef(TMCStackManager,1) //Interface to a particles stack
};

#endif //ROOT_TMCStackManager
