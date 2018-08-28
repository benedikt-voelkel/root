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

#ifndef ROOT_TMCManager
#define ROOT_TMCManager
//
// Class TVirtualMCApplication
// ---------------------------
// Interface to a user Monte Carlo application.
//

#include <memory>
#include <stack>

#include "TObject.h"
#include "TMath.h"

#include "TMCtls.h"

class TMCStackManager;
class TVirtualMC;
class TVirtualMCApplication;
class TMCStateManager;

class TMCManager {

public:

   /// Destructor
   ~TMCManager();

   //
   // static methods
   //

   /// Static access method to TMCManager singleton
   static TMCManager* Instance();
   /// Static access method to current TVirtualMC
   static TVirtualMC* GetMC();

   //
   // action methods
   //

   /// Register a transport engine
   void RegisterMC(TVirtualMC* mc);
   /// Initialize MCs
   /// Only available in concurrent run mode
   void InitMCs();
   /// Running the MCs
   /// Only available in concurrent run mode
   void RunMCs(Int_t nofEvents);
   /// At each step decide whether a track has to be moved
   /// \todo Find a more suitable name for thins method
   void Stepping();
   /// Pass a pointer from the outside which will be updated whenever the
   /// engine used changes
   /// \note experimental
   void ConnectToCurrentMC(TVirtualMC*& mc);

   //
   // getting and setting
   //

   /// Check whther primaries must be pushed to the VMC stack
   Bool_t NeedPrimaries() const;

   //
   // Verbosity
   //

   /// Print status of TMCManager and registered engines
   void Print() const;

private:
  /// Check for next engine and set for event processing
  Bool_t GetNextEngine();
  /// Terminate the run for all registered engines
  void TerminateRun();
  /// Set all pointers registered to current engine
  void UpdateConnectedEnginePointers();

private:
  /// Default constructor
  TMCManager();
  /// Flag to tell whether to run in concurrent mode or single engine mode
  Bool_t fIsConcurrentMode;
  /// Flag to tell whether primaries should be pushed to the VMC stack
  Bool_t fNeedPrimaries;
  /// The running TVirtualMCApplication
  TVirtualMCApplication* fMCApplication;
  /// Wrapping engine, stack and criteria
  std::vector<TVirtualMC*> fMCEngines;
  /// So far also for convenience
  static TVirtualMC* fCurrentMCEngine;
  /// storing pointer to global TMCStackManager
  TMCStackManager* fMCStackManager;
  /// Count the number of processed events
  Int_t fNEventsProcessed;
  /// Pointer to pointer of outside engine
  std::vector<TVirtualMC**> fOutsideMCPointerAddresses;
  /// Pointer to global state manager
  TMCStateManager* fMCStateManager;


   // static data members
  #if !defined(__CINT__)
     static TMCThreadLocal TMCManager* fgInstance; ///< Singleton instance
  #else
     static                TMCManager* fgInstance; ///< Singleton instance
  #endif


   ClassDef(TMCManager,1)  //Interface to MonteCarlo application
};

#endif //ROOT_TMCManager
