// @(#)root/vmc:$Id$
// Authors: Benedikt Volkel 22/01/2019

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TMCParticleStatus
#define ROOT_TMCParticleStatus

// Class TVirtualMCStack
// ---------------------
// Interface to a user defined particles stack.
//

#include <iostream>

#include "TVector3.h"
#include "TLorentzVector.h"
#include "TParticle.h"
#include "TError.h"

struct TMCParticleStatus
{
  public:
    TMCParticleStatus()
      : fCurrentStepNumber(0), fCurrentTrackLength(0.),fCurrentPosition(),
        fCurrentMomentum(), fCurrentPolarization(), fCurrentWeight(1.),
        fGeoStateIndex(-1), fId(-1), fParentId(-1)
      {}

    void InitFromParticle(const TParticle* particle) {
      particle->ProductionVertex(fCurrentPosition);
      particle->Momentum(fCurrentMomentum);
      particle->GetPolarisation(fCurrentPolarization);
      fCurrentWeight = particle->GetWeight();
    }

    virtual ~TMCParticleStatus() = default;

    //
    // verbosity
    //

    /// Print all info at once
    void Print() const
    {
      Info("Print", "Initial status of track");
      std::cout << "\t" << "ID: " << fId << "\n"
                << "\t" << "parentID: " << fParentId << "\n"
                << "\t" << "weight: " << fCurrentWeight << "\n"
                << "\t" << "geo state index: " << fGeoStateIndex << "\n"
                << "\t" << "step number: " << fCurrentStepNumber << "\n"
                << "\t" << "track length: " << fCurrentTrackLength << "\n"
                << "\t" << "position" << std::endl;
      fCurrentPosition.Print();
      std::cout << "\t" << "momentum" << std::endl;
      fCurrentMomentum.Print();
      std::cout << "\t" << "polarization" << std::endl;
      fCurrentPolarization.Print();
    }

  public:
    /// Number of steps
    Int_t fCurrentStepNumber;
    /// Track length
    Double_t fCurrentTrackLength;
    /// Current position
    TLorentzVector fCurrentPosition;
    /// Current momentum
    TLorentzVector fCurrentMomentum;
    /// Current polarization
    TVector3 fCurrentPolarization;
    /// Current weight
    Double_t fCurrentWeight;
    /// Current geo state cache
    Int_t fGeoStateIndex;
    /// Unique ID assigned by the user
    Int_t fId;
    /// Unique ID assigned by the user
    Int_t fParentId;

    ClassDef(TMCParticleStatus,1)
};

#endif /* ROOT_TMCParticleStatus */
