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
    fChildren(nullptr)
{}

TTrack::TTrack(Int_t id, Int_t pdg, Int_t status, TTrack* parent,
               Double_t px, Double_t py,
               Double_t pz, Double_t etot, Double_t vx, Double_t vy,
               Double_t vz, Double_t time, Int_t geoStateIndex)
  : TParticle(pdg, status, -1, -1, -1, -1, px, py, pz,
              etot, vx, vy, vz, time), fId(id), fGeoStateIndex(geoStateIndex),
              fParent(parent), fChildren(new TObjArray(10))
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
