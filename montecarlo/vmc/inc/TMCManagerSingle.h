// @(#)root/vmc:$Id$
// Author: Ivana Hrivnacova, 23/03/2002

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TMCManagerSingle
#define ROOT_TMCManagerSingle
//
// Class TVirtualMCApplication
// ---------------------------
// Interface to a user Monte Carlo application.

#include "TVrtualMCManager.h"

#include "TMCtls.h"

class TVirtualMC;
class TParticle;

template <class A=TVirtualMCApplication>
class TMCManagerSingle : public TVirtualMCManager {

public:
   /// Standard constructor
   TMCManagerSingle(A* application)
    : TVirtualMCManager(application), fApplication(application)
   {
   }

   /// Destructor
   virtual ~TMCManagerSingle()
   {
   }

   /// Static access method
   static TMCManagerSingle<A>* Instance()
   {
     return
     static_cast<TMCManagerSingle<A>*>(TVirtualMCManager::Instance());
   }

   //
   // methods
   //

   /// Register the transport engine and return ID.
   void RegisterMC(TVirtualMC* mc) override final
   {
     // If there is already a transport engine, fail since only one is allowed.
     if(fMC) {
       Fatal("RegisterMC", "Attempt to register a second TVirtualMC which " \
                           "is not allowed");
     }
     fMC = mc;
     // There is only 1 VMC in this case so ID is known to be 0.
     fMC->SetId(0);
   }

   /// Construct user geometry
   void ConstructGeometry() override final
   {
     fApplication->ConstructGeometry();
   }

   /// Misalign user geometry (optional)
   Bool_t MisalignGeometry() override final
   {
     fApplication->MisalignGeometry();
   }

   /// Define parameters for optical processes (optional)
   void ConstructOpGeometry() override final
   {
     fApplication->ConstructOpGeometry();
   }

   /// Define sensitive detectors (optional)
   void ConstructSensitiveDetectors() override final
   {
     fApplication->ConstructSensitiveDetectors();
   }

   /// Initialize geometry
   /// (Usually used to define sensitive volumes IDs)
   void InitGeometry() override final
   {
     fApplication->InitGeometry();
   }

   /// Add user defined particles (optional)
   void AddParticles() override final
   {
     fApplication->AddParticles();
   }

   /// Add user defined ions (optional)
   void AddIons() override final
   {
     fApplication->AddIons();
   }

   /// Generate primary particles
   void GeneratePrimaries() override final
   {
     fApplication->GeneratePrimaries();
   }

   /// Define actions at the beginning of the event
   void BeginEvent() override final
   {
     fApplication->BeginEvent();
   }

   /// Define actions at the beginning of the primary track
   void BeginPrimary() override final
   {
     fApplication->BeginPrimary();
   }

   /// Define actions at the beginning of each track
   void PreTrack() override final
   {
     fApplication->PreTrack();
   }

   /// Define action at each step
   void Stepping() override final
   {
     fApplication->Stepping();
   }

   /// Define actions at the end of each track
   void PostTrack() override final
   {
     fApplication->PostTrack();
   }

   /// Define actions at the end of the primary track
   void FinishPrimary() override final
   {
     fApplication->FinishPrimary();
   }

   /// Define actions at the end of the event
   void FinishEvent() override final
   {
     fApplication->FinishEvent();
   }

   /// Define action at each step for Geane
   void GeaneStepping() override final
   {
     fApplication->GeaneStepping();
   }

 private:
   A* fApplication;


   ClassDefOverride(TMCManagerSingle,1)

 };

#endif /* ROOT_TMCManagerSingle */
