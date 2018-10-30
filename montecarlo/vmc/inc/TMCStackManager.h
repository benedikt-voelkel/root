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

#ifndef ROOT_TMCStackManager
#define ROOT_TMCStackManager

// Class TMCStackManager
// ---------------------
// Interface to a user defined particles stack as well as to TMCQueues of
// different TVirtualMCs.
//

#include <functional>
#include <map>

#include "TObject.h"

#include "TMCtls.h"
#include "TMCProcess.h"
#include "TLorentzVector.h"
#include "TGeoNavigator.h"
#include "TGeoBranchArray.h"

class TTrack;
class TMCQueue;
class TVirtualMCStack;
class TVirtualMC;
class TGeoCacheManual;


/// The transport status of a track.
enum class ETrackTransportStatus : Int_t {kNew, kProcessing, kFinished};


class TMCStackManager {

public:

   // Destructor
   ~TMCStackManager();

   /// Static access method
   static TMCStackManager* Instance();

   //
   // Methods for stacking
   //

   /// Wrapper around TVirtualMCStack::PushTrack
   ///
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

   //
   // Get methods
   //
   // Most of them wrapping get methods of TVirtualMCStack

   /// Total number of tracks
   Int_t      GetNtrack()    const;

   /// Total number of primary tracks
   Int_t      GetNprimary()  const;

   /// Current track
   TTrack* GetCurrentTrack() const;

   /// Current track number
   Int_t      GetCurrentTrackNumber() const;

   /// Number of the parent of the current track
   Int_t      GetCurrentParentTrackNumber() const;

   /// Return the transport status of the current track
   ETrackTransportStatus GetTrackTransportStatus() const;

   //
   // Set methods
   //

   /// Set the current track id from the outside and forward this to the
   /// user's TVirtualMCStack
   void SetCurrentTrack(Int_t trackID);

   /// Set the function called in stepping to check whether a track needs to
   /// be moved
   void RegisterSuggestTrackForMoving(std::function<void(TVirtualMC*, TVirtualMC*&)> f);
   /// Set the function called to decide to which queue a primary is pushed
   void RegisterSpecifyEngineForTrack(std::function<void(TTrack*, TVirtualMC*&)> f);

   /// Assign a queue for a given engine
   void SetQueue(TVirtualMC* mc);

   /// Register the user TVirtualMCStack
   void RegisterStack(TVirtualMCStack* stack);

   /// Set the current navigator
   void SetCurrentNavigator(TGeoNavigator* navigator);

   //
   // Action methods
   //

   /// Check whether the user's criteria call for moving the track from the
   /// current engine
   void SuggestTrackForMoving(TVirtualMC* currentMC);

   /// Clear the TVirtualMCStack
   void ResetStack();

   //
   // Verbosity
   //

   /// Print status of stack and queues
   void Print() const;

 private:
   /// Constructor private for save singleton behaviour
   TMCStackManager();
   /// Find the right queue to push this track to and push it
   void PushToQueue(TTrack* track);
   /// Move track from a given engine to a target queue of another engine
   void MoveTrack(TVirtualMC* currentMC, TMCQueue* targetQueue);

  private:
   // static data members
  #if !defined(__CINT__)
     static TMCThreadLocal TMCStackManager* fgInstance; ///< Singleton instance
  #else
     static                TMCStackManager* fgInstance; ///< Singleton instance
  #endif
    /// The global stack managed by the TMCStackManager to ensure
    /// a coherent history
    TVirtualMCStack* fGlobalStack;
    /// Pointer to current track
    TTrack* fCurrentTrack;
    /// Only the track id
    Int_t fCurrentTrackID;
    // Following are used for internal temorary caching of track information
    /// Cache for current position
    TLorentzVector fCurrentPosition;
    /// Cache for current momentum
    TLorentzVector fCurrentMomentum;
    /// Cache target engine when checking for moving track
    TVirtualMC* fTargetMCCached;
    /// Cache geo state when moving track
    TGeoBranchArray* fGeoStateCached;
    /// Flag the transport status to decide e.g. whether a track has been
    /// transported before.
    std::vector<ETrackTransportStatus> fTrackTransportStatus;
    /// Pointer to cache with geometry states
    TGeoCacheManual* fGeoStateCache;
    /// Pointer to navigator used by current engine
    TGeoNavigator* fCurrentNavigator;
    /// Decide where to transfer a track to given user conditions
    std::function<void(TVirtualMC*, TVirtualMC*&)> fSuggestTrackForMoving;
    /// Decide where to push a track to which has not been transported
    std::function<void(TTrack*, TVirtualMC*&)>  fSpecifyEngineForTrack;

   ClassDefNV(TMCStackManager,1)
};

inline void TMCStackManager::SetCurrentNavigator(TGeoNavigator* navigator)
{
  fCurrentNavigator = navigator;
}

#endif // ROOT_TMCStackManager
