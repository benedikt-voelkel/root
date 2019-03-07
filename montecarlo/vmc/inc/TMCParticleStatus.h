// @(#)root/vmc:$Id$
// Authors: Benedikt Volkel 07/031/2019

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

// Class TMCParticleStatus
// ---------------------
// additional information on the current status of a TParticle
//

#include <iostream>

#include "TVector3.h"
#include "TLorentzVector.h"
#include "TParticle.h"
#include "TError.h"

struct TMCParticleStatus {

   /// Default constructor
   TMCParticleStatus()
      : fStepNumber(0), fTrackLength(0.), fPosition(), fMomentum(), fPolarization(), fWeight(1.), fGeoStateIndex(-1),
        fId(-1), fParentId(-1)
   {
   }

   /// Use TParticle information as a starting point
   void InitFromParticle(const TParticle *particle)
   {
      particle->ProductionVertex(fPosition);
      particle->Momentum(fMomentum);
      particle->GetPolarisation(fPolarization);
      fWeight = particle->GetWeight();
   }

   virtual ~TMCParticleStatus() = default;

   //
   // verbosity
   //

   /// Print all info at once
   void Print() const
   {
      Info("Print", "Initial status of track");
      std::cout << "\t"
                << "ID: " << fId << "\n"
                << "\t"
                << "parentID: " << fParentId << "\n"
                << "\t"
                << "weight: " << fWeight << "\n"
                << "\t"
                << "geo state index: " << fGeoStateIndex << "\n"
                << "\t"
                << "step number: " << fStepNumber << "\n"
                << "\t"
                << "track length: " << fTrackLength << "\n"
                << "\t"
                << "position" << std::endl;
      fPosition.Print();
      std::cout << "\t"
                << "momentum" << std::endl;
      fMomentum.Print();
      std::cout << "\t"
                << "polarization" << std::endl;
      fPolarization.Print();
   }

   /// Number of steps
   Int_t fStepNumber;
   /// Track length
   Double_t fTrackLength;
   ///  position
   TLorentzVector fPosition;
   ///  momentum
   TLorentzVector fMomentum;
   ///  polarization
   TVector3 fPolarization;
   ///  weight
   Double_t fWeight;
   ///  geo state cache
   Int_t fGeoStateIndex;
   /// Unique ID assigned by the user
   Int_t fId;
   /// Unique ID assigned by the user
   Int_t fParentId;

private:
   /// Copying kept private
   TMCParticleStatus(const TMCParticleStatus &);
   /// Assignement kept private
   TMCParticleStatus &operator=(const TMCParticleStatus &);

   ClassDef(TMCParticleStatus, 1)
};

#endif /* ROOT_TMCParticleStatus */
