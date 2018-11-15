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

#ifndef ROOT_TVirtualMCManager
#define ROOT_TVirtualMCManager
//
// Class TVirtualMCApplication
// ---------------------------
// Interface to a user Monte Carlo application.

#include "TVrtualMCApplication.h"

#include "TMCtls.h"

class TVirtualMC;
class TParticle;

class TVirtualMCManager {

public:
   /// Standard constructor
   TVirtualMCManager(TVirtualMCApplication* application);

   /// Default constructor
   TVirtualMCManager();

   /// Destructor
   virtual ~TVirtualMCManager();

   /// Static access method
   static TVirtualMCManager* Instance();

   //
   // methods
   //

   /// Return the current transport engine in use
   /// \note This is static to ensure backwards compatibility with
   /// TVirtualMC::GetMC()
   static TVirtualMC* GetMCStatic();

   /// Return the transport engine registered to this application
   TVirtualMC* GetMC() const;

   /// Return the current transport engine in use
   /// \note This is static to ensure backwards compatibility with
   /// TVirtualMC::GetMC()
   static TVirtualMCApplication* GetApplicationStatic();

   /// Return the transport engine registered to this application
   TVirtualMCApplication* GetApplication() const;

   /// Register the transport engine and return ID.
   virtual void RegisterMC(TVirtualMC* mc) = 0;

   /// Construct user geometry
   virtual void ConstructGeometry() = 0;

   /// Misalign user geometry (optional)
   virtual Bool_t MisalignGeometry() = 0;

   /// Define parameters for optical processes (optional)
   virtual void ConstructOpGeometry() = 0;

   /// Define sensitive detectors (optional)
   virtual void ConstructSensitiveDetectors() = 0;

   /// Initialize geometry
   /// (Usually used to define sensitive volumes IDs)
   virtual void InitGeometry() = 0;

   /// Add user defined particles (optional)
   virtual void AddParticles() = 0;

   /// Add user defined ions (optional)
   virtual void AddIons() = 0;

   /// Generate primary particles
   virtual void GeneratePrimaries() = 0;

   /// Define actions at the beginning of the event
   virtual void BeginEvent() = 0;

   /// Define actions at the beginning of the primary track
   virtual void BeginPrimary() = 0;

   /// Define actions at the beginning of each track
   virtual void PreTrack() = 0;

   /// Define action at each step
   virtual void Stepping() = 0;

   /// Define actions at the end of each track
   virtual void PostTrack() = 0;

   /// Define actions at the end of the primary track
   virtual void FinishPrimary() = 0;

   /// Define actions at the end of the event
   virtual void FinishEvent() = 0;

   /// Define action at each step for Geane
   virtual void GeaneStepping() = 0;

   //
   // Other than hooks, non-virtual since no special treatment in single vs.
   // multi-run
   //

   /// Define maximum radius for tracking (optional)
   Double_t TrackingRmax() const
   {
     return fApplicationBase->TrackingRmax();
   }
   /// Define maximum z for tracking (optional)
   Double_t TrackingZmax() const
   {
     return fApplicationBase->TrackingZmax();
   }

   /// Calculate user field \a b at point \a x
   void Field(const Double_t* x, Double_t* b) const
   {
     fApplicationBase->Field();
   }

 private:
   // Private, no copying.
   TVirtualMCManager(const TVirtualMCManager &);
   TVirtualMCManager & operator=(const TVirtualMCManager &);


protected:
   /// The current transport engine in use. \note This is static to ensure
   /// backwards compatibility with TVirtualMC::GetMC() via
   /// TVirtualMCApplication::GetMCStatic()
   #if !defined(__CINT__)
      /// Global pointer to current TVirtualMC
      static TMCThreadLocal TVirtualMC* fMC;
   #else
      static                TVirtualMC* fMC;
   #endif

 private:
    #if !defined(__CINT__)
       /// Global pointer to current TVirtualMCManager
       static TMCThreadLocal TVirtualMCManager* fgInstance;
       /// Global pointer to registered TVirtualMCApplication
       static TMCThreadLocal TVirtualMCApplication* fApplicationBase;
    #else
       static                TVirtualMCManager* fgInstance; ///< Singleton instance
       static                TVirtualMCApplication* fApplicationBase;
    #endif

   ClassDef(TVirtualMCManager,1)
};

#endif /* ROOT_TVirtualMCManager */
