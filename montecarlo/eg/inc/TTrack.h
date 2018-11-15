// @(#)root/eg:$Id$
// Author: Benedikt Volkel, 30/10/2018

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TTrack: extends TParticle providing some typical track functionality //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TTrack
#define ROOT_TTrack

#include "TParticle.h"
#include <TObjArray.h>


class TTrack : public TParticle {

   public:

     /// Default constructor
     TTrack();

     /// Standard constructors
     TTrack(Int_t id, Int_t pdg, Int_t status,
            TTrack* parent,
            Double_t px, Double_t py, Double_t pz, Double_t etot,
            Double_t vx, Double_t vy, Double_t vz, Double_t time,
            Int_t geoStateIndex);

     TTrack(Int_t id, Int_t pdg, Int_t status,
            TTrack* parent, const TLorentzVector &p,
            const TLorentzVector &v,
            Int_t geoStateIndex);

     /// Copy constructor
     TTrack(const TTrack& track);

     /// Assignment operator
     TTrack& operator=(const TTrack&);

     /// Desctructor
     virtual ~TTrack();

     /// Set track ID, uniqueness needs to be ensured by the user
     void Id(Int_t id);

     /// Get track ID
     Int_t Id() const;

     /// Set the geometry state index which can be associated to this track
     void GeoStateIndex(Int_t index);

     /// Get the geometry state index which can be associated to this track
     Int_t GeoStateIndex() const;

     /// Add a child to this track
     void AddChild(TTrack* child);

     /// Number of children
     Int_t GetNChildren() const;

     /// Get a child by index
     const TTrack* GetChild(Int_t index) const;

     /// Get parent track
     const TTrack* GetParent() const;

     /// Add a new point (spatial and momentum) to this track which is then the
     /// current point
     void AddPositionMomentum(Double_t t, Double_t x, Double_t y, Double_t z,
                           Double_t e, Double_t px, Double_t py, Double_t pz);

     /// Add a new point (spatial and momentum) to this track which is then the
     /// current point
     void AddPositionMomentum(const TLorentzVector& v, const TLorentzVector& p);

     /// Clear all positions and momenta
     void ClearPositionsMomenta();


   private:
     /// Track id
     Int_t fId;
     /// Index of the state on the TGeoNavigator stack
     Int_t fGeoStateIndex;
     /// Pointer to parent track
     TTrack* fParent;
     /// Child tracks
     TObjArray* fChildren;
     /// Points this track has passed, contains position (4 parameters) and
     /// momenta (following 4 parameters)
     Double_t* fPoints;
     /// Number of points
     Int_t fNpoints;
     /// Maximum nnumber of points that can currently be stored
     Int_t fNmaxPoints;
     /// Number of parameters of a point, 4 (position) + 4 (momenta)
     static constexpr Int_t fNparameters = 8;

   ClassDefOverride(TTrack,1)
};

#endif
