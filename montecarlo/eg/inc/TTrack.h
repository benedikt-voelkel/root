// @(#)root/eg:$Id$
// Author: Rene Brun , Federico Carminati  26/04/99

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TParticle: defines  equivalent of HEPEVT particle                    //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TTrack
#define ROOT_TTrack

#include "TParticle.h"
#include <TObjArray.h>

class TTrack : public TParticle {


  //----------------------------------------------------------------------------
  //  functions
  //----------------------------------------------------------------------------
public:
                                // ****** constructors and destructor

   /// Default constructor
   TTrack();

   TTrack(Int_t id, Int_t pdg, Int_t status,
          TTrack* parent,
          Double_t px, Double_t py, Double_t pz, Double_t etot,
          Double_t vx, Double_t vy, Double_t vz, Double_t time,
          Int_t geoStateIndex);

   TTrack(Int_t id, Int_t pdg, Int_t status,
          TTrack* parent, const TLorentzVector &p,
          const TLorentzVector &v,
          Int_t geoStateIndex);

   TTrack(const TTrack& track);

   virtual ~TTrack();

   TTrack& operator=(const TTrack&);

   /// A track has a unique id. The objects using TTrack objects have to ensure
   /// that track IDs are unique.
   /// Set track ID
   void Id(Int_t id);
   /// Get track ID
   Int_t Id() const;

   /// A track is associated to a navigation state on the stack of TGeoNavigator
   /// Set the state index
   void GeoStateIndex(Int_t index);
   /// Get the state index
   Int_t GeoStateIndex() const;

   /// Add a child to this track
   void AddChild(TTrack* child);
   /// Number of children
   Int_t GetNChildren() const;
   /// Get a child
   const TTrack* GetChild(Int_t index) const;

   /// Get the parent track
   const TTrack* GetParent() const;


   private:
     /// Track id
     Int_t fId;
     /// Index of the state on the TGeoNavigator stack
     Int_t fGeoStateIndex;
     /// Pointer to parent track
     TTrack* fParent;
     /// Child tracks
     TObjArray* fChildren;

   ClassDef(TTrack,1)
};



/// Deriving a user class from this interface can be used to define some
/// management interception to e.g. directly assign track ids during
/// construction of a TTrack
/// It also ownes all TTracks
/*
class TTrackManagementInterface
{
  public:
    TTrackManagementInterface();
};
*/
#endif
