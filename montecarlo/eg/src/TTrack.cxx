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

#include "TTrack.h"

ClassImp(TTrack);

TTrack::TTrack()
  : TParticle(), fId(-1), fGeoStateIndex(-1), fParent(nullptr),
    fChildren(nullptr), fPoints(nullptr), fNpoints(0), fNmaxPoints(0)
{}

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

TTrack::TTrack(Int_t id, Int_t pdg, Int_t status, TTrack* parent,
               const TLorentzVector &p,
               const TLorentzVector &v, Int_t geoStateIndex)
  : TParticle(pdg, status, -1, -1, -1, -1, p, v),
    fId(id), fGeoStateIndex(geoStateIndex), fParent(parent), fChildren(new TObjArray(10))
{
  if(parent) {
    SetFirstMother(parent->Id());
  }
}

// This may have a valid geometry state but no ID yet
// \todo Queck whether a geometry state can be popped more than once from the
// TGeoNavigator
TTrack::TTrack(const TTrack& track)
  : TParticle(track), fId(-1), fGeoStateIndex(track.fGeoStateIndex),
    fParent(track.fParent), fChildren(track.fChildren)
{}

// This may have a valid geometry state but no ID yet
// \todo Queck whether a geometry state can be popped more than once from the
// TGeoNavigator
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

TTrack::~TTrack()
{}

void TTrack::Id(Int_t id)
{
 fId = id;
}

Int_t TTrack::Id() const
{
 return fId;
}

void TTrack::GeoStateIndex(Int_t index)
{
 fGeoStateIndex = index;
}

Int_t TTrack::GeoStateIndex() const
{
 return fGeoStateIndex;
}

void TTrack::AddChild(TTrack* child)
{
  // \todo Maybe first check whether this is already a child
  fChildren->Add(child);
}

Int_t TTrack::GetNChildren() const
{
  return fChildren->GetEntriesFast();
}

const TTrack* TTrack::GetChild(Int_t index) const
{
  if(index < 0 || index >= fChildren->GetEntriesFast()) {
    Fatal("GetChild", "Index out of range");
  }
  return (TTrack*)fChildren->At(index);
}

const TTrack* TTrack::GetParent() const
{
  return fParent;
}

void TTrack::AddPointMomentum(Double_t x, Double_t y, Double_t z, Double_t t,
                              Double_t px, Double_t py, Double_t pz, Double_t e)
{
  // Logic according to TGeoTrack
  if(!fPoints) {
    fNmaxPoints = 4;
    fNpoints = 0;
    // Have 8 parameters fNparameters per point
    fPoints = new Double_t[fNmaxPoints*fNparameters];
  } else if(fNpoints == fNmaxPoints) {
    // Allocate twice the memory and copy what is already there.
    Double_t* tmpPoints = new Double_t[2*fNmaxPoints*fNparameters];
    memcpy(tmpPoints, fPoints, fNmaxPoints*fNparameters*sizeof(Double_t));
    fNmaxPoints *= 2;
    delete [] fPoints;
    fPoints = tmpPoints;
  }
  // Spatial components
  fPoints[fNpoints] = x;
  fPoints[fNpoints+1] = y;
  fPoints[fNpoints+2] = z;
  fPoints[fNpoints+3] = t;
  // Momentum components
  fPoints[fNpoints+4] = px;
  fPoints[fNpoints+5] = py;
  fPoints[fNpoints+6] = pz;
  fPoints[fNpoints+7] = e;
  fNpoints++;
  // Now update the TParticle property
  SetProductionVertex(x, y, z, t);
  SetMomentum(px, py, pz, e);

}

void TTrack::AddPointMomentum(const TLorentzVector& v, const TLorentzVector& p)
{
  AddPointMomentum(v.X(), v.Y(), v.Z(), v.T(),
                   p.Px(), p.Py(), p.Pz(), p.Energy());
}

void TTrack::ClearPointsMomenta()
{
  delete[] fPoints;
  fNpoints = 0;
  fNmaxPoints = 0;
}
