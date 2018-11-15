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

#ifndef ROOT_TVirtualMCMultiStack
#define ROOT_TVirtualMCMultiStack

// Class TVirtualMCMultiStack
// ---------------------
// Interface to a user defined particles stack managing sub-stacks of
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

#include "TVirtualMCStack.h"
#include "TMCSimpleStack.h"

class TTrack;
class TParticle;
class TVirtualMC;
class TGeoStateCache;

/// The transport status of a track.
enum class ETrackTransportStatus : Int_t {kNew, kProcessing, kFinished};

class TVirtualMCMultiStack : public TVirtualMCStack {

public:

   /// Default constructor
   TVirtualMCMultiStack();
   /// Destructor
   ~TVirtualMCMultiStack();

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
   /// Without geoStateIndex
   void PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                  Double_t px, Double_t py, Double_t pz, Double_t e,
                  Double_t vx, Double_t vy, Double_t vz, Double_t tof,
                  Double_t polx, Double_t poly, Double_t polz,
                  TMCProcess mech, Int_t& ntr, Double_t weight,
                  Int_t is) override final;

   /// With geoStateIndex
   void PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                  Double_t px, Double_t py, Double_t pz, Double_t e,
                  Double_t vx, Double_t vy, Double_t vz, Double_t tof,
                  Double_t polx, Double_t poly, Double_t polz,
                  Int_t geoStateIndex, TMCProcess mech, Int_t& ntr,
                  Double_t weight, Int_t is) override final;

   /// Push a track including its geo state and return the pointer to it.
   /// Taht is the only user interface required.
   virtual TTrack* PushUserTrack(Int_t parent, Int_t pdg,
                                 Double_t px, Double_t py, Double_t pz,
                                 Double_t e, Double_t vx, Double_t vy,
                                 Double_t vz, Double_t tof, Double_t polx,
                                 Double_t poly, Double_t polz,
                                 Int_t geoStateIndex, TMCProcess mech,
                                 Double_t weight, Int_t is) = 0;

   //
   // Get methods
   //

   /// Pop the next track
   TParticle* PopNextTrack(Int_t&  itrack) override final;

   /// Pop next track with geoStateIndex
   TParticle* PopNextTrack(Int_t&  itrack, Int_t& geoStateIndex) override final;

   /// Pop i'th primar, that does not mean that this primariy also has ID==i
   TParticle* PopPrimaryForTracking(Int_t i) override final;

   /// Pop i'th primar, that does not mean that this primariy also has ID==i.
   /// including actual index and geoStateIndex
   TParticle* PopPrimaryForTracking(Int_t i, Int_t& itrack,
                                    Int_t& geoStateIndex) override final;

   /// Get number of tracks which are only those to be tracked.
   Int_t GetNtrack() const override final;

   /// Get the number of primaries in current stack
   Int_t GetNprimary() const override final;

   /// Current track
   TParticle* GetCurrentTrack() const override final;

   /// Current track number
   Int_t GetCurrentTrackNumber() const override final;

   /// Number of the parent of the current track
   Int_t GetCurrentParentTrackNumber() const override final;

   /// Return the transport status of the current track
   ETrackTransportStatus GetTrackTransportStatus() const;

   /// Check if there are tracks for specific TVirtualMC
   Bool_t HasTracks(TVirtualMC* mc) const;

   //
   // Set methods
   //

   /// Set the current track id from the outside and forward this to the
   /// user's TVirtualMCStack
   void SetCurrentTrack(Int_t trackID) override final;

   /// Set the function called in stepping to check whether a track needs to
   /// be moved
   void RegisterSuggestTrackForMoving(
                          std::function<void(TVirtualMC*, TVirtualMC*&)> f);

   /// Set the function called to decide to which stack a primary is pushed
   void RegisterSpecifyEngineForTrack(
                          std::function<void(TTrack*, TVirtualMC*&)> f);

   /// Add a internal stack for an engine
   void AddEngineStack(TVirtualMC* mc);

   /// Add a stack for a given engine and set engine stack
   void SetEngineStack(TVirtualMC* mc);

   /// Set the current navigator
   void SetCurrentNavigator(TGeoNavigator* navigator);

   //
   // Action methods
   //

   /// Check whether the user's criteria call for moving the track from the
   /// current engine
   void SuggestTrackForMoving(TVirtualMC* currentMC);

   /// Reset internals, clear engine stack and fTracks and reset buffered values
   void ResetInternals();

 private:
   /// Find the right engine stack to push this track to and push it
   void PushToInternalStack(TTrack* track);
   /// Insert a new transport status for a given track ID
   void InsertTrackTransportStatus(Int_t trackID, ETrackTransportStatus status);

  private:
    /// All tracks
    TObjArray* fTracks;
    /// Count the primaries
    Int_t fPrimaries;
    /// Pointer to current track
    TTrack* fCurrentTrack;
    /// Only the track id
    Int_t fCurrentTrackID;
    /// The parent track ID
    Int_t fCurrentParentTrackID;
    // Following are used for internal temorary caching of track information
    /// Cache for current position
    TLorentzVector fCurrentPosition;
    /// Cache for current momentum
    TLorentzVector fCurrentMomentum;
    /// Cache target engine when checking for moving track
    TVirtualMC* fTargetMCCached;
    /// Cache geo state when moving track
    TGeoBranchArray* fCachedGeoState;
    /// Flag the transport status to decide e.g. whether a track has been
    /// transported before.
    std::vector<ETrackTransportStatus> fTrackTransportStatus;
    /// Pointer to cache with geometry states
    TGeoStateCache* fGeoStateCache;
    /// Pointer to navigator used by current engine
    TGeoNavigator* fCurrentNavigator;
    /// Decide where to transfer a track to given user conditions
    std::function<void(TVirtualMC*, TVirtualMC*&)> fSuggestTrackForMoving;
    /// Decide where to push a track to which has not been transported
    std::function<void(TTrack*, TVirtualMC*&)>  fSpecifyEngineForTrack;
    /// Mapping the pointers of TVirtualMC to their stacks
    std::map<TVirtualMC*, TMCSimpleStack<TTrack*>> fVMCToStackMap;
    /// The stack tracks are currently popped from
    TMCSimpleStack<TTrack*>* fCurrentStack;
    /// Mapping the pointers of TVirtualMC to number of primaries in their stack
    std::map<TVirtualMC*, Int_t> fVMCToPrimariesMap;
    /// Number of primaries on current stack
    Int_t* fCurrentNPrimaries;

   ClassDefOverride(TVirtualMCMultiStack,1)
};

inline void TVirtualMCMultiStack::SetCurrentNavigator(TGeoNavigator* navigator)
{
  fCurrentNavigator = navigator;
}

#endif // ROOT_TVirtualMCMultiStack
