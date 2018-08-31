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

#include <functional>

#include "TObject.h"

#include "TMCtls.h"
#include "TMCProcess.h"

class TTrack;
class TMCQueue;
class TVirtualMCStack;
class TVirtualMC;
class TVirtualMCApplication;
class TMCStateManager;


/// Specify the mode of stack export, either per event or per run
/// In the latter case all tracks are dumped into one event. Note, that a TTrack
/// itself does not know to which event it belongs so that information is lost
/// in that case.
//enum class EStackExportMode : Int_t {kPerEvent, kPerRun};


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
   void PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                  Double_t px, Double_t py, Double_t pz, Double_t e,
                  Double_t vx, Double_t vy, Double_t vz, Double_t tof,
                  Double_t polx, Double_t poly, Double_t polz,
                  Int_t geoStateIndex, TMCProcess mech, Int_t& ntr,
                  Double_t weight, Int_t is);

    /// The outside world might suggest that the track handled by the engine
    /// of the current engine needs to be moved
    void SuggestTrackForMoving(TVirtualMC* currentMC);

   /// The stack has to provide two pop mechanisms:
   /// The first pop mechanism required.
   /// Pop all particles with toBeDone = 1, both primaries and seconadies
   //virtual TTrack* PopNextTrack(Int_t& itrack) = 0;

   /// The second pop mechanism required.
   /// Pop only primary particles with toBeDone = 1, stacking of secondaries
   /// is done by MC
   //virtual TTrack* PopPrimaryForTracking(Int_t i) = 0;

   //
   // Set methods
   //


   /// Get a notification from a queue or an engine about the tracks which is processed
   //void               NotifyOnProcessedTrack(TTrack* track);
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
   TTrack* GetCurrentTrack() const { return fCurrentTrack; }

   /// Current track number
   Int_t      GetCurrentTrackNumber() const;

   /// Number of the parent of the current track
   Int_t      GetCurrentParentTrackNumber() const;

   /// Set the current track id from the outside
   void SetCurrentTrack(Int_t trackID);

   //
   // Set methods
   //

   /// Set the function called in stepping to check whether a track needs to be moved
   void RegisterSuggestTrackForMoving(std::function<Bool_t(TVirtualMC*, TVirtualMC*&)> f);
   /// Set the function called to decide to qhich queue a primary is pushed
   void RegisterSpecifyEngineForTrack(std::function<Bool_t(TTrack*, TVirtualMC*&)> f);


   /// Set a  new queue for an engine and register this engine
   /// Nobody but the TMCStackManager should ever need to manage a queue and engines
   /// are inly allowed to pop from these
   void SetQueue(TVirtualMC* mc);

   /// Register the master/global stack to the TMCStackManager
   void RegisterStack(TVirtualMCStack* stack);

   /// Check whether there are primaries on the global TVirtualMCStack
   Bool_t HasPrimaries();

   /// Forward primaries from global VMC stack to engine queues. By default it
   /// is suggested that this means a new event is started.
   void InitializeQueuesWithPrimaries(Bool_t isNewEvent = kTRUE);

   /// Notify the TMCStackManager that an event has finished.
   void NotifyOnFinishedEvent();

   /// Export the stack to a ROOT file
   //void ExportStack();
   /// Import a stack from a ROOT file
   //void ImportStack();
   /// Set the ROOT file to which/from where a stack should be exported/imported
   //void SetStackFileName(const char* stackFileName);
   //
   // Verbosity
   //

   /// Print status of stacks/queues
   void Print() const;
 private:

   /// Constructor prvate for save singleton behaviour
   TMCStackManager();
   /// Push a new track to a queue of the stored TVirtualMC
   void PushToQueue(TTrack* track);
   /// Buffer current parameters the selection is based on
   void BufferSelectionParameters(TVirtualMC* currentMC);
   /// Move a track handled byb an engine to a new target queue
   void MoveTrack(TVirtualMC* currentMC, TMCQueue* targetQueue);
   /// Push a track to pseudo stack
   void PushPseudoTrack(TTrack* pseudoTrack);
   /// Clear the psuedo stack
   void ClearPseudoStack();

  private:
   // static data members
  #if !defined(__CINT__)
     static TMCThreadLocal TMCStackManager* fgInstance; ///< Singleton instance
  #else
     static                TMCStackManager* fgInstance; ///< Singleton instance
  #endif
    TVirtualMCApplication*       fMCApplication;    ///< Pointer to global TVirtualMCApplication
    TVirtualMCStack*             fMasterStack;      ///< The master stack with coherent history, this is what the outside world sees
    TTrack*                      fCurrentTrack;     ///< Pointer to current track
    Int_t                        fCurrentTrackID;   ///< Only the track id
    std::vector<TVirtualMC*>     fMCEngines;        ///< Store MCs for stack/queue management
    // Information for the seletion of engines when moving tracks (or attempt to)
    Int_t                        fCurrentVolId;     ///< Buffer for the current volume id
    Int_t                        fCurrentVolCopyNo; ///< Buffer the copy number of the current volume
    Bool_t                       fLastTrackSuggestedForMoving; ///< Flag the current track since it might need to be moved in the next step
    TMCStateManager*             fMCStateManager;              ///< Pointer to global state manager
    std::vector<TTrack*>         fPseudoTracks;                ///< Pseudo tracks cached e.g. when tracks are moved between engines
    /// Decide where to transfer a track to given user conditions
    std::function<Bool_t(TVirtualMC*, TVirtualMC*&)> fSuggestTrackForMoving;
    /// Decide where to push a track to which has not been transported
    std::function<Bool_t(TTrack*, TVirtualMC*&)>  fSpecifyEngineForTrack;

   ClassDef(TMCStackManager,1) //Interface to a particles stack
};

#endif //ROOT_TMCStackManager
