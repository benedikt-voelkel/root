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

#include <functional>

#include "TVirtualMCApplication.h"
#include "TLorentzVector.h"
#include "TGeoMCBranchArrayContainer.h"

class TVirtualMCMultiStack;
class TParticle;
class TGeoNavigator;
class TGeoBranchArray;

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

   /// Set the function called in stepping to check whether a track needs to
   /// be moved
   void RegisterSuggestTrackForMoving(
                        std::function<void(const TVirtualMC*, TVirtualMC*&)> f);

   /// Set the function called to decide to which stack a primary is pushed
   void RegisterSpecifyEngineForTrack(
                        std::function<void(const TParticle*, TVirtualMC*&)> f);

   /// Construct geometry gloablly to be used by all engines
   void ConstructGlobalGeometry();

   /// Do all initialisation steps at once including a custom user init as
   /// lambda.
   void InitTransport(std::function<void(TVirtualMC*)> customInit = 
                                                          [](TVirtualMC*){});

   /// Run the transport by steering engines
   void RunTransport(Int_t nofEvents);


   //
   // Virtual methods
   //

   /// Construct user geometry
   void ConstructGeometry() override final;
   virtual void ConstructGeometryMulti() = 0;

   /// Construct user geometry
   Bool_t MisalignGeometry() override final;
   virtual Bool_t MisalignGeometryMulti() {return kFALSE;}

   /// Construct user geometry
   void ConstructOpGeometry() override final;
   virtual void ConstructOpGeometryMulti() {}

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

     /// Check whether the user's criteria call for moving the track from the
     /// current engine
     Bool_t SuggestTrackForMoving();


   private:
     /// A vector holding all registered engine pointers.
     std::vector<TVirtualMC*> fMCEngines;
     /// Cache target engine when checking for moving track
     TVirtualMC* fTargetMCCached;
     /// The TVirtualMCMultiStack
     TVirtualMCMultiStack* fStack;
     /// Collect the IDs of the sub-stacks
     std::vector<Int_t> fSubStackIds;
     // Following are used for internal temorary caching of track information
     /// Cache for current position
     TLorentzVector fCurrentPosition;
     /// Cache for current momentum
     TLorentzVector fCurrentMomentum;
     /// Pointer to cache with geometry states
     TGeoMCBranchArrayContainer fBranchArrayContainer;
     /// Cache geo state when moving track
     TGeoBranchArray* fCachedGeoState;
     /// Pointer to navigator used by current engine
     TGeoNavigator* fCurrentNavigator;
     /// Decide where to push a track to which has not been transported and pass
     /// ID of target engine
     std::function<void(const TParticle*, TVirtualMC*&)> fSpecifyEngineForTrack;
     /// Decide where to transfer a track to given user conditions and pass ID
     /// of target engine
     std::function<void(const TVirtualMC*, TVirtualMC*&)> fSuggestTrackForMoving;
     /// Flag if InitTransport was called
     Bool_t fIsGeometryConstructed;
     /// Flag if InitTransport was called
     Bool_t fIsInitialized;

   ClassDefOverride(TVirtualMCMultiApplication,1)
};

// inline functions

inline TVirtualMCMultiApplication* TVirtualMCMultiApplication::Instance()
{
  return
  static_cast<TVirtualMCMultiApplication*>(TVirtualMCApplication::Instance());
}

#endif //ROOT_TVirtualMCMultiApplication
