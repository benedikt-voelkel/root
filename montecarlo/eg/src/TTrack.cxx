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

#include "TTrack.h"

ClassImp(TTrack);

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TTrack::TTrack()
  : TParticle(), fId(-1), fGeoStateIndex(-1), fParent(nullptr),
    fChildren(nullptr), fPoints(nullptr), fNpoints(0), fNmaxPoints(0)
{}

////////////////////////////////////////////////////////////////////////////////
///
/// Standard constructor
///

TTrack::TTrack(Int_t id, Int_t pdg, Int_t status, TTrack* parent,
               Double_t px, Double_t py,
               Double_t pz, Double_t etot, Double_t vx, Double_t vy,
               Double_t vz, Double_t time, Int_t geoStateIndex)
  : TParticle(pdg, status, -1, -1, -1, -1, px, py, pz,
              etot, vx, vy, vz, time), fId(id), fGeoStateIndex(geoStateIndex),
              fParent(parent), fChildren(new TObjArray(10)), fPoints(nullptr),
              fNpoints(0), fNmaxPoints(0)
{
  if(parent) {
    SetFirstMother(parent->Id());
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Standard constructor
///

TTrack::TTrack(Int_t id, Int_t pdg, Int_t status, TTrack* parent,
               const TLorentzVector &p,
               const TLorentzVector &v, Int_t geoStateIndex)
  : TParticle(pdg, status, -1, -1, -1, -1, p, v),
    fId(id), fGeoStateIndex(geoStateIndex), fParent(parent),
    fChildren(new TObjArray(10)), fPoints(nullptr), fNpoints(0), fNmaxPoints(0)
{
  if(parent) {
    SetFirstMother(parent->Id());
  }
}

////////////////////////////////////////////////////////////////////////////////
///
/// Copy constructor
///

// This may have a valid geometry state but no ID yet
TTrack::TTrack(const TTrack& track)
  : TParticle(track), fId(-1), fGeoStateIndex(track.fGeoStateIndex),
    fParent(track.fParent), fChildren(track.fChildren)
{}

////////////////////////////////////////////////////////////////////////////////
///
/// Assignment operator
///

// This may have a valid geometry state but no ID yet
TTrack& TTrack::operator=(const TTrack& track)
{
  if(this!=&track) {
    TParticle::operator=(track);
    fId = -1;
    fGeoStateIndex = track.fGeoStateIndex;
    fParent = track.fParent;
    fChildren = track.fChildren;
  }
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Standard constructor
///

TTrack::~TTrack()
{
  delete[] fPoints;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set track ID, uniqueness needs to be ensured by the user
///

void TTrack::Id(Int_t id)
{
 fId = id;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get track ID
///

Int_t TTrack::Id() const
{
 return fId;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Set the geometry state index which can be associated to this track
///

void TTrack::GeoStateIndex(Int_t index)
{
 fGeoStateIndex = index;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get the geometry state index which can be associated to this track
///

Int_t TTrack::GeoStateIndex() const
{
 return fGeoStateIndex;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Add a child to this track
///

void TTrack::AddChild(TTrack* child)
{
  // \todo Maybe first check whether this is already a child
  fChildren->Add(child);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Number of children
///

Int_t TTrack::GetNChildren() const
{
  return fChildren->GetEntriesFast();
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get a child by index
///

const TTrack* TTrack::GetChild(Int_t index) const
{
  if(index < 0 || index >= fChildren->GetEntriesFast()) {
    Fatal("GetChild", "Index out of range");
  }
  return dynamic_cast<TTrack*>(fChildren->At(index));
}

////////////////////////////////////////////////////////////////////////////////
///
/// Get parent track
///

const TTrack* TTrack::GetParent() const
{
  return fParent;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Add a new point (spatial and momentum) to this track which is then the
/// current point
///

void TTrack::AddPositionMomentum(Double_t x, Double_t y, Double_t z, Double_t t,
                                 Double_t px, Double_t py, Double_t pz,
                                 Double_t e)
{
  // Logic according to TGeoTrack
  if(!fPoints) {
    fNmaxPoints = 4;
    fNpoints = 0;
    // Have fNparameters parameters per point
    fPoints = new Double_t[fNmaxPoints*fNparameters];
  } else if(fNpoints == fNmaxPoints) {
    // Allocate twice the memory and copy what is already there.
    Double_t* tmpPoints = new Double_t[2*fNmaxPoints*fNparameters];
    memcpy(tmpPoints, fPoints, fNmaxPoints*fNparameters*sizeof(Double_t));
    fNmaxPoints *= 2;
    // Reassign address
    delete [] fPoints;
    fPoints = tmpPoints;
  }
  // Set spatial components
  fPoints[fNpoints] = x;
  fPoints[fNpoints+1] = y;
  fPoints[fNpoints+2] = z;
  fPoints[fNpoints+3] = t;
  // Set momentum components
  fPoints[fNpoints+4] = px;
  fPoints[fNpoints+5] = py;
  fPoints[fNpoints+6] = pz;
  fPoints[fNpoints+7] = e;
  fNpoints++;
  // Update the TParticle properties
  SetProductionVertex(x, y, z, t);
  SetMomentum(px, py, pz, e);
}

////////////////////////////////////////////////////////////////////////////////
///
/// Add a new point (spatial and momentum) to this track which is then the
/// current point
///

void TTrack::AddPositionMomentum(const TLorentzVector& v, const TLorentzVector& p)
{
  AddPositionMomentum(v.X(), v.Y(), v.Z(), v.T(),
                      p.Px(), p.Py(), p.Pz(), p.Energy());
}

////////////////////////////////////////////////////////////////////////////////
///
/// Clear all positions and momenta
///

void TTrack::ClearPositionsMomenta()
{
  delete[] fPoints;
  fNpoints = 0;
  fNmaxPoints = 0;
}
