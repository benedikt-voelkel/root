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
#include <vector>
#include <stack>

#include "TObject.h"

#include "TMCtls.h"
#include "TMCProcess.h"

#include "TVirtualMCStack.h"

#include "TParticle.h"

class TLorentzVector;

/// Container to connect ID and geoState to TParticle
struct TMCParticleStatus
{
  public:
    TMCParticleStatus()
      : fUserParticle(nullptr), fTmpParticle(nullptr), fId(-1), fParentId(-1),
        fGeoStateIndex(-1), fTransportStatus(ETrackTransportStatus::kFinished)
      {}

  public:
    /// Pointer to particle created by the user
    TParticle* fUserParticle;
    /// Copy of user particle
    TParticle* fTmpParticle;
    /// Unique ID assigned by the user
    Int_t fId;
    /// Parent unique ID
    Int_t fParentId;
    /// Geometry state (>=0: valid state, no state cached otherwise)
    Int_t fGeoStateIndex;
    /// Current state of the particle transport
    ETrackTransportStatus fTransportStatus;

    ClassDefNV(TMCParticleStatus,1)
};

class TVirtualMCMultiStack : public TVirtualMCStack {

public:

   /// Default constructor
   TVirtualMCMultiStack();
   /// Constrcutor passing the number of slots/memory to be reserved for
   /// tracks
   TVirtualMCMultiStack(Int_t initialStackSize);
   /// Destructor
   virtual ~TVirtualMCMultiStack();

   //
   // Methods for stacking
   //

   /// Wrapper around TVirtualMCMultiStack::PushTrack
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
   /// Without geoState
   void PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                  Double_t px, Double_t py, Double_t pz, Double_t e,
                  Double_t vx, Double_t vy, Double_t vz, Double_t tof,
                  Double_t polx, Double_t poly, Double_t polz,
                  TMCProcess mech, Int_t& ntr, Double_t weight,
                  Int_t is) override final;

   /// With geoState and track transport status
   void PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                  Double_t px, Double_t py, Double_t pz, Double_t e,
                  Double_t vx, Double_t vy, Double_t vz, Double_t tof,
                  Double_t polx, Double_t poly, Double_t polz,
                  Int_t geoStateIndex,
                  ETrackTransportStatus transportStatus,
                  TMCProcess mech, Int_t& ntr, Double_t weight, Int_t is)
                  override final;

   /// That is the only user interface required to be implemented.
   /// Push a track including its geoState and transport status and return
   /// provide pointer to partice stacked on user stack.
   virtual TParticle*  PushUserTrack(Int_t parent, Int_t pdg,
                                     Double_t px,Double_t py, Double_t pz,
                                     Double_t e, Double_t vx, Double_t vy,
                                     Double_t vz, Double_t tof, Double_t polx,
                                     Double_t poly, Double_t polz,
                                     Int_t geoStateIndex,
                                     ETrackTransportStatus transportStatus,
                                     TMCProcess mech, Int_t& ntr,
                                     Double_t weight, Int_t is) = 0;

   //
   // Get methods
   //

   /// Pop the next track
   TParticle* PopNextTrack(Int_t& itrack) override final;

   /// Pop next track from specific sub-stack
   TParticle* PopNextTrackFromSubStack(Int_t stackId, Int_t& itrack);

   /// Pop i'th primar, that does not mean that this primariy also has ID==i
   TParticle* PopPrimaryForTracking(Int_t i) override final;

   /// Pop i'th primary, that does not mean that this primariy also has ID==i.
   /// including actual index
   TParticle* PopPrimaryForTracking(Int_t i, Int_t& itrack) override final;

   /// Pop i'th primary from sub-stack
   TParticle* PopPrimaryForTrackingFromSubStack(Int_t stackId, Int_t i,
                                                Int_t& itrack);

   /// Get number of tracks on current sub-stack
   Int_t GetNtrack() const override final;

   /// Get total number of tracks
   Int_t GetTotalNtrack() const;

   /// Get number of primaries on current sub-stack
   Int_t GetNprimary() const override final;

   /// Get total number of primaries
   Int_t GetTotalNprimary() const;

   /// Current track
   TParticle* GetCurrentTrack() const override final;

   /// Current track number
   Int_t GetCurrentTrackNumber() const override final;

   /// Number of the parent of the current track
   Int_t GetCurrentParentTrackNumber() const override final;

   /// Get the geometry state as TGeoBranchArray
   Int_t GetCurrentTrackGeoStateIndex() const override final;

   /// Get the geo state index by track ID
   Int_t GetTrackGeoStateIndex(Int_t trackID) const override final;

   /// Return the transport status of the current track
   ETrackTransportStatus GetTrackTransportStatus() const;

   /// Check if there are tracks for specific TVirtualMC
   Bool_t HasTracks(Int_t stackId) const;

   //
   // Set methods
   //

   /// Set the current track id from the outside and forward this to the
   /// user's TVirtualMCMultiStack
   void SetCurrentTrack(Int_t trackID) override final;

   /// Set the function called to decide to which stack a primary is pushed
   void RegisterSpecifyStackForTrack(std::function<void(const TParticle*, Int_t&)> f);

   /// Add a internal stack for an engine
   Int_t AddSubStack();

   /// Add a stack for a given engine and set engine stack
   void UseSubStack(Int_t id);


   //
   // Action methods
   //

   /// Move the current track to other stack with ID
   void MoveCurrentTrack(Int_t stackId, const TLorentzVector& newPosition,
                         const TLorentzVector& newMomentum,
                         Int_t geoStateIndex);

   /// Reset internals, clear engine stack and fTracks and reset buffered values
   void ResetInternals();

   void PrintTracks() const;

 private:
   /// Check whether track ID exists
   Bool_t HasTrack(Int_t id) const;
   /// Check whether stack with specified ID is present
   Bool_t HasSubStackId(Int_t id) const;
   /// Insert a new transport status for a given track ID
   void InsertTrack(Bool_t toBeDone, TParticle* particle, Int_t usertrackID,
                    Int_t parentTrackID, Int_t geoStateIndex,
                    ETrackTransportStatus transportStatus);

  private:
    /// Remember the initial stack size
    Int_t fInitialStackSize;
    /// Pointer to current track
    Int_t fCurrentTrackId;
    /// The stack tracks are currently popped from
    Int_t fCurrentStackId;
    /// All tracks
    std::vector<TMCParticleStatus> fTracks;
    /// Mapping the pointers of TVirtualMC to their stacks
    std::vector<std::stack<Int_t>> fStacks;
    /// Mapping the pointers of TVirtualMC to number of primaries in their stack
    std::vector<Int_t> fPrimariesStackCounts;
    /// Decide where to push a track to which has not been transported and pass
    /// ID of target engine
    std::function<void(const TParticle*, Int_t&)> fSpecifyStackForTrack;

   ClassDefOverride(TVirtualMCMultiStack,1)
};

#endif // ROOT_TVirtualMCMultiStack
