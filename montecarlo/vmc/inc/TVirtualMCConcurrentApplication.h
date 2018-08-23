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

#ifndef ROOT_TVirtualMCConcurrentApplication
#define ROOT_TVirtualMCConcurrentApplication
//
// Class TVirtualMCConcurrentApplication
// ---------------------------
// Interface to a user Monte Carlo application.
//

#include <memory>

#include "TNamed.h"
#include "TMath.h"

#include "TMCtls.h"

#include "TVirtualMCApplication.h"

class TVirtualMCConcurrentApplication : public TVirtualMCApplication {

public:
   /// Standard constructor
   TVirtualMCConcurrentApplication(const char *name, const char *title);

   /// Default constructor
   TVirtualMCConcurrentApplication();

   /// Destructor
   virtual ~TVirtualMCConcurrentApplication();

   /// Static access method
   static TVirtualMCConcurrentApplication* Instance();

   //
   // methods
   //


   // --------------- Current status ---------------------
   /// Construct user geometry
   virtual void ConstructGeometry() = 0;

   /// Misalign user geometry (optional)
   virtual Bool_t MisalignGeometry() {return kFALSE;}

   /// Define parameters for optical processes (optional)
   virtual void ConstructOpGeometry() {}

   /// Initialize geometry
   /// (Usually used to define sensitive volumes IDs)
   virtual void InitGeometryConcurrent() = 0;
   void InitGeometry();

   /// Add user defined particles (optional)
   virtual void AddParticles() {}

   /// Add user defined ions (optional)
   virtual void AddIons() {}

   /// Generate primary particles
   /// Either push particles to the TVirtualMCStack directly or do it via the
   /// TMCStackManager::PushTrack(...)
   virtual void GeneratePrimariesConcurrent() = 0;
   void GeneratePrimaries();

   /// Define actions at the beginning of the event
   virtual void BeginEventConcurrent() = 0;
   void BeginEvent();

   /// Define actions at the beginning of the primary track
   virtual void BeginPrimaryConcurrent() = 0;
   void BeginPrimary();

   /// Define actions at the beginning of each track
   virtual void PreTrackConcurrent() = 0;
   void PreTrack();

   /// Define action at each step
   /// The stepping action also calling UserStepping
   /// Do not override this method
   virtual void SteppingConcurrent() = 0;
   void Stepping();

   /// Define actions at the end of each track
   virtual void PostTrackConcurrent() = 0;
   void PostTrack();

   /// Define actions at the end of the primary track
   virtual void FinishPrimaryConcurrent() = 0;
   void FinishPrimary();

   /// Define actions at the end of the event
   virtual void FinishEventConcurrent() = 0;
   void FinishEvent();

   /// Define maximum radius for tracking (optional)
   virtual Double_t TrackingRmax() const { return DBL_MAX; }

   /// Define maximum z for tracking (optional)
   virtual Double_t TrackingZmax() const { return DBL_MAX; }

   /// Calculate user field \a b at point \a x
   virtual void Field(const Double_t* x, Double_t* b) const;

   /// Define action at each step for Geane
   virtual void GeaneStepping() {;}

   // Functions for multi-threading applications
   /// Clone MC application on worker
   virtual TVirtualMCConcurrentApplication* CloneForWorker() const { return 0;}

   /// Const Initialize MC application on worker  - now deprecated
   /// Use new non-const InitOnWorker()  instead
   virtual void InitForWorker() const {}
   /// Const Define actions at the beginning of the worker run if needed - now deprecated
   /// Use new non-const BeginRunOnWorker() instead
   virtual void BeginWorkerRun() const {}
   /// Const Define actions at the end of the worker run if needed - now deprecated
   /// Use new non-const FinishRunOnWorker() instead
   virtual void FinishWorkerRun() const {}

   /// Initialize MC application on worker
   virtual void InitOnWorker() {}
   /// Define actions at the beginning of the worker run if needed
   virtual void BeginRunOnWorker() {}
   /// Define actions at the end of the worker run if needed
   virtual void FinishRunOnWorker() {}
   /// Merge the data accumulated on workers to the master if needed
   virtual void Merge(TVirtualMCConcurrentApplication* /*localMCApplication*/) {}

private:
   // static data members
  #if !defined(__CINT__)
     static TMCThreadLocal TVirtualMCConcurrentApplication* fgInstance; ///< Singleton instance
  #else
     static                TVirtualMCConcurrentApplication* fgInstance; ///< Singleton instance
  #endif


   ClassDef(TVirtualMCConcurrentApplication,1)  //Interface to MonteCarlo application
};

inline void TVirtualMCConcurrentApplication::Field(const Double_t* /*x*/, Double_t* b) const {
   // No magnetic field
   b[0] = 0; b[1] = 0; b[2] = 0;
}

#endif //ROOT_TVirtualMCConcurrentApplication
