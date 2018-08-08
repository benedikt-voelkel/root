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

#ifndef ROOT_TVirtualMCApplication
#define ROOT_TVirtualMCApplication
//
// Class TVirtualMCApplication
// ---------------------------
// Interface to a user Monte Carlo application.
//

#include <memory>

#include "TNamed.h"
#include "TMath.h"
//#include "TVirtualMC.h"

#include "TMCtls.h"

class TVirtualMC;
class TMCSelectionCriteria;

class TVirtualMCApplication : public TNamed {

public:
   /// Standard constructor
   TVirtualMCApplication(const char *name, const char *title);

   /// Default constructor
   TVirtualMCApplication();

   /// Destructor
   virtual ~TVirtualMCApplication();

   /// Static access method
   static TVirtualMCApplication* Instance();

   //
   // methods
   //

   /// Register a transport engine
   void RegisterMC(TVirtualMC* mc);
   /// Get the current transport engine
   static TVirtualMC* GetMC();

   /// Construct the entire user geometry already
   /// 1) ConstructGeometry
   /// 2) MisalignGeometry
   /// 3) ConstructOpGeometry
   void ConstructUserGeometry();

   /// Construct user geometry
   virtual void ConstructGeometry() = 0;

   /// Misalign user geometry (optional)
   virtual Bool_t MisalignGeometry() {return kFALSE;}

   /// Define parameters for optical processes (optional)
   virtual void ConstructOpGeometry() {}

   /// Initialize geometry
   /// (Usually used to define sensitive volumes IDs)
   virtual void InitGeometry() = 0;

   /// Setting transport properties of media, namely enable/disable processes and set production cuts
   virtual void SetTransportMediaProperties() {};

   /// Add user defined particles (optional)
   virtual void AddParticles() {}

   /// Add user defined ions (optional)
   virtual void AddIons() {}

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
   virtual TVirtualMCApplication* CloneForWorker() const { return 0;}

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
   virtual void Merge(TVirtualMCApplication* /*localMCApplication*/) {}

   /// Get the selection criteria by engine name
   TMCSelectionCriteria* GetSelectionCriteria(const char* name);

protected:
  /// get the next responsible engine in the chain. Assuming the engines are run consecutively
  /// if all stacks are empty, return nullptr and set flag for new event
  // \note right now that is very brute-force, but for test-purposes
  TVirtualMC* GetNextEngine();
  /// update stack, shift/remove particles and stop tracks. Must be called in user implementation of Stepping()
  void UpdateStacks();

protected:
  /// registered engines, express TVirtualMCApplication's ownership of TVirtualMCs
  std::vector<TVirtualMC*> fMCEngines;
  /// corresponding selection criteria, only used by the TVirtualMCApplication ==> no pointers needed
  std::vector<TMCSelectionCriteria*> fTMCSelectionCriteria;
  /// for convenience and immediate usage, forwarding to the outside world
  static TVirtualMC* fCurrentMCEngine;
  //TMCSelectionCriteria* fCurrentMCSelectionCriteria;

private:
   // static data members
  #if !defined(__CINT__)
     static TMCThreadLocal TVirtualMCApplication* fgInstance; ///< Singleton instance
  #else
     static                TVirtualMCApplication* fgInstance; ///< Singleton instance
  #endif


   ClassDef(TVirtualMCApplication,1)  //Interface to MonteCarlo application
};

inline void TVirtualMCApplication::Field(const Double_t* /*x*/, Double_t* b) const {
   // No magnetic field
   b[0] = 0; b[1] = 0; b[2] = 0;
}

#endif //ROOT_TVirtualMCApplication
