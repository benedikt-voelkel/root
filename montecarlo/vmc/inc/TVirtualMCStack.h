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

#ifndef ROOT_TVirtualMCStack
#define ROOT_TVirtualMCStack

// Class TVirtualMCStack
// ---------------------
// Interface to a user defined particles stack.
//

#include "TObject.h"
#include "TMCProcess.h"

class TParticle;

/// The transport status of a track.
enum class ETrackTransportStatus : Int_t {kNew, kProcessing, kFinished};

class TVirtualMCStack : public TObject {

public:
   // Constructor
   TVirtualMCStack();

   // Destructor
   virtual ~TVirtualMCStack();

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
   virtual void  PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                           Double_t px, Double_t py, Double_t pz, Double_t e,
                           Double_t vx, Double_t vy, Double_t vz, Double_t tof,
                           Double_t polx, Double_t poly, Double_t polz,
                           TMCProcess mech, Int_t& ntr, Double_t weight,
                           Int_t is) = 0;

    /// With geoStateIndex and transportStatus
    /// Should be overriden if user stack manages these
   virtual void  PushTrack(Int_t toBeDone, Int_t parent, Int_t pdg,
                           Double_t px, Double_t py, Double_t pz, Double_t e,
                           Double_t vx, Double_t vy, Double_t vz, Double_t tof,
                           Double_t polx, Double_t poly, Double_t polz,
                           Int_t geoStateIndex,
                           ETrackTransportStatus transportStatus,
                           TMCProcess mech, Int_t& ntr, Double_t weight,
                           Int_t is);


   /// The stack has to provide two pop mechanisms:
   /// The first pop mechanism required.
   /// Pop all particles with toBeDone = 1, both primaries and seconadies
   virtual TParticle* PopNextTrack(Int_t& itrack) = 0;

   /// The second pop mechanism required.
   /// Pop only primary particles with toBeDone = 1, stacking of secondaries
   /// is done by MC
   /// \note That is a misleading interface since one might assume that the
   /// particle popped with index i has also the ID i. This is not necessarily
   /// true because the internal stacking mechanism might be unknown to users as
   /// well as to the TVirtualMC which might use this interface for popping.
   /// It was used in TGeant4's TG4PrimaryGeneratorAction::TransformPrimaries as
   /// if all particles on the stack were primaries because the indices were
   /// derived from TVirtualMCStack::GetNtrack() which just returns the number
   /// of all tracks.
   /// Hence, this interface is inconsistent and should not be used.
   virtual TParticle* PopPrimaryForTracking(Int_t i) = 0;

   /// Don't assume that the primary at index i has ID i, secondaries might sit
   /// in between
   virtual TParticle* PopPrimaryForTracking(Int_t i, Int_t& itrack);
   //
   // Set methods
   //

   /// Set the current track number
   virtual void       SetCurrentTrack(Int_t trackNumber) = 0;

   //
   // Get methods
   //

   /// Total number of tracks
   virtual Int_t      GetNtrack()    const = 0;

   /// Total number of primary tracks
   virtual Int_t      GetNprimary()  const = 0;

   /// Current track particle
   virtual TParticle* GetCurrentTrack() const= 0;

   /// Current track number
   virtual Int_t      GetCurrentTrackNumber() const = 0;

   /// Number of the parent of the current track
   virtual Int_t      GetCurrentParentTrackNumber() const = 0;

   /// Get the geometry state as TGeoBranchArray
   virtual Int_t GetCurrentTrackGeoStateIndex() const;

   /// Get the geo state index by track ID
   virtual Int_t GetTrackGeoStateIndex(Int_t trackId) const;

   ClassDef(TVirtualMCStack,1) //Interface to a particles stack
};

#endif //ROOT_TVirtualMCStack
