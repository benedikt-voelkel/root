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

#include "TVirtualMCApplication.h"

class TVirtualMCConcurrentApplication : public TVirtualMCApplication {

public:
   /// Standard constructor
   TVirtualMCConcurrentApplication(const char *name, const char *title);

   /// Default constructor
   TVirtualMCConcurrentApplication();

   /// Destructor
   virtual ~TVirtualMCConcurrentApplication() {}


   static TVirtualMCConcurrentApplication* Instance();
   //
   // methods
   //

   /// Initialize geometry
   /// (Usually used to define sensitive volumes IDs)
   virtual void InitGeometryConcurrent() = 0;
   virtual void InitGeometry() override;

   /// Generate primary particles
   /// Either push particles to the TVirtualMCStack directly or do it via the
   /// TMCStackManager::PushTrack(...)
   virtual void GeneratePrimariesConcurrent() = 0;
   virtual void GeneratePrimaries() override;

   /// Define actions at the beginning of the event
   virtual void BeginEventConcurrent() = 0;
   virtual void BeginEvent() override;

   /// Define actions at the beginning of the primary track
   virtual void BeginPrimaryConcurrent() = 0;
   virtual void BeginPrimary() override;

   /// Define actions at the beginning of each track
   virtual void PreTrackConcurrent() = 0;
   virtual void PreTrack() override;

   /// Define action at each step
   /// The stepping action also calling UserStepping
   /// Do not override this method
   virtual void SteppingConcurrent() = 0;
   virtual void Stepping() override;

   /// Define actions at the end of each track
   virtual void PostTrackConcurrent() = 0;
   virtual void PostTrack() override;

   /// Define actions at the end of the primary track
   virtual void FinishPrimaryConcurrent() = 0;
   virtual void FinishPrimary() override;

   /// Define actions at the end of the event
   virtual void FinishEventConcurrent() = 0;
   virtual void FinishEvent() override;

   ClassDef(TVirtualMCConcurrentApplication,1)  //Interface to MonteCarlo application
};

// inline functions

inline TVirtualMCConcurrentApplication* TVirtualMCConcurrentApplication::Instance()
{
  return static_cast<TVirtualMCConcurrentApplication*>(TVirtualMCApplication::Instance());
}

#endif //ROOT_TVirtualMCConcurrentApplication
