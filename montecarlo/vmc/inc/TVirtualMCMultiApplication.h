// @(#)root/vmc:$Id$
// Author: Benedikt Volkel, 30/10/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TVirtualMCMultiApplication
#define ROOT_TVirtualMCMultiApplication
//
// Class TVirtualMCMultiApplication
// ---------------------------
// Interface to a user Monte Carlo application using multiple transport engines
// for the simulation setup.
//

#include "TVirtualMCApplication.h"

class TVirtualMCMultiStack;

class TVirtualMCMultiApplication : public TVirtualMCApplication {

public:
   /// Standard constructor
   TVirtualMCMultiApplication(const char *name, const char *title);

   /// Default constructor
   TVirtualMCMultiApplication();

   /// Destructor
   virtual ~TVirtualMCMultiApplication();

   /// Static access method
   static TVirtualMCMultiApplication* Instance();

   /// Register a new transport engine
   void RegisterMC(TVirtualMC* mc) override final;

   /// Set the TVirtualMCMultiStack
   void SetStack(TVirtualMCMultiStack* stack);

   /// Do all initialisation steps at once
   void InitTransport() override final;

   /// Run the transport by steering engines
   void RunTransport(Int_t nofEvents) override final;


   //
   // Virtual methods
   //

   /// Construct user geometry
   void ConstructGeometry() override final;
   virtual void ConstructGeometryMulti() = 0;

   /// Initialize geometry
   /// (Usually used to define sensitive volumes IDs)
   virtual void InitGeometryMulti() = 0;
   void InitGeometry() override final;

   /// Generate primary particles
   virtual void GeneratePrimariesMulti() = 0;
   void GeneratePrimaries() override final;

   /// Define actions at the beginning of the event
   virtual void BeginEventMulti() = 0;
   void BeginEvent() override final;

   /// Define actions at the beginning of the primary track
   virtual void BeginPrimaryMulti() = 0;
   void BeginPrimary() override final;

   /// Define actions at the beginning of each track
   virtual void PreTrackMulti() = 0;
   void PreTrack() override final;

   /// Define action at each step
   virtual void SteppingMulti() = 0;
   void Stepping() override final;

   /// Define actions at the end of each track
   virtual void PostTrackMulti() = 0;
   void PostTrack() override final;

   /// Define actions at the end of the primary track
   virtual void FinishPrimaryMulti() = 0;
   void FinishPrimary() override final;

   /// Define actions at the end of the event
   virtual void FinishEventMulti() = 0;
   void FinishEvent() override final;

   private:
     /// Choose next engines to be run in the loop
     Bool_t GetNextEngine();

     /// Terminate the run for all engines
     void TerminateRun();


   private:
     /// A vector holding all registered engine pointers.
     std::vector<TVirtualMC*> fMCEngines;
     /// The TVirtualMCMultiStack
     TVirtualMCMultiStack* fStack;

   ClassDefOverride(TVirtualMCMultiApplication,1)
};

// inline functions

inline TVirtualMCMultiApplication* TVirtualMCMultiApplication::Instance()
{
  return
  static_cast<TVirtualMCMultiApplication*>(TVirtualMCApplication::Instance());
}

#endif //ROOT_TVirtualMCMultiApplication
