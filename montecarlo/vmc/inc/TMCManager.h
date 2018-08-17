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
#include "TMCSelectionCriteria.h"

class TMCStackManager;
class TVirtualMC;
class TMCContainer;

class TMCManager : public TObject {

public:

   /// Destructor
   virtual ~TMCManager();

   /// Static access method
   static TMCManager* Instance();

   //
   // methods
   //

   /// Register a transport engine
   void RegisterMC(TVirtualMC* mc);
   /// Initialize MCs
   void InitMCs();
   /// Initialize the queues using the TMCStackManager
   void InitializeQueues();
   /// Running the MCs
   void RunMCs(Int_t nofEvents);
   /// Terminate the run for all registered engines
   void TerminateRun();
   /// At each step decide whether a track has to be moved
   /// \todo Find a more suitable name for thins method
   void Stepping();
   /// Get the current transport engine
   static TVirtualMC* GetMC();
   /// Get the selection criteria of the currently running engine
   static TMCSelectionCriteria* GetSelectionCriteria();
   /// Get the selection criteria by engine name
   TMCSelectionCriteria* GetSelectionCriteria(const char* name);
   /// Get the selection criteria by engine pointer
   TMCSelectionCriteria* GetSelectionCriteria(TVirtualMC* mc);
   /// Pass a pointer from the outside which will be updated whenever the MC changes
   /// \note experimental
   void ConnectToCurrentMC(TVirtualMC*& mc);


protected:
  /// update stack, shift/remove particles and stop tracks. Must be called in user implementation of Stepping()
  void UpdateStacks();
  /// Get an engine by its name
  TVirtualMC* GetMC(const char* name);
  /// Choose and set the current container by name and pointer
  void SetCurrentMCContainer(const char* name);
  void SetCurrentMCContainer(TMCContainer* con);
  /// Check for next engine and set for event processing
  Bool_t GetNextEngine();

protected:
  /// Wrapping engine, stack and criteria
  std::vector<TMCContainer*> fMCContainers;
  /// for convenience and immediate usage, forwarding to the outside world
  static TMCContainer* fCurrentMCContainer;
  /// So far also for convenience
  static TVirtualMC* fCurrentMCEngine;
  /// storing pointer to global TMCStackManager
  TMCStackManager* fMCStackManager;
  /// Count the number of processed events
  Int_t fNEventsProcessed;
  /// Pointer to pointer of outside engine
  std::vector<TVirtualMC**> fOutsideMCPointerAddresses;

private:

  /// Default constructor
  TMCManager();

   // static data members
  #if !defined(__CINT__)
     static TMCThreadLocal TMCManager* fgInstance; ///< Singleton instance
  #else
     static                TMCManager* fgInstance; ///< Singleton instance
  #endif


   ClassDef(TMCManager,1)  //Interface to MonteCarlo application
};

#endif //ROOT_TMCManager
