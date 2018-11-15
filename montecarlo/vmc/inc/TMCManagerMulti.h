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

#ifndef ROOT_TMCManagerMulti
#define ROOT_TMCManagerMulti
//
// Class TVirtualMCApplication
// ---------------------------
// Interface to a user Monte Carlo application.

#include "TVrtualMCManager.h"

#include "TMCtls.h"

class TVirtualMC;
class TParticle;

template <class A=TVirtualMCApplication>
class TMCManagerMulti : public TVirtualMCManager {

public:
   /// Standard constructor
   TMCManagerMulti(A* application)
    : TVirtualMCManager(application), fApplication(application),
      fTargetMCCached(nullptr),
      fStack(nullptr), fCurrentPosition(TLorentzVector()),
      fCurrentMomentum(TLorentzVector()),
      fBranchArrayContainer(),
      fCachedGeoState(nullptr), fCurrentNavigator(nullptr),
      fIsGeometryConstructed(kFALSE), fIsInitialized(kFALSE)
  {
    fMCEngines.clear();
    fSubStackIds.clear();
  }

   /// Default constructor
   TMCManagerMulti() : TVirtualMCManager()
   {
   }

   /// Destructor
   virtual ~TMCManagerMulti()
   {
   }

   /// Static access method
   static TMCManagerMulti<A>* Instance()
   {
     return
     static_cast<TMCManagerMulti<A>*>(TVirtualMCManager::Instance());
   }

   //
   // methods
   //

   /// Register the transport engine and return ID.
   void RegisterMC(TVirtualMC* mc) override final
   {

     // First set the current engine
     fMC = newMC;
     // If in concurrent mode, add engine to the list of engines
     // make sure, at least engine names are unique
     for(auto& mc : fMCEngines) {
       if(strcmp(newMC->GetName(), mc->GetName()) == 0) {
         Fatal("RegisterMC", "There is already an engine with name %s.",
                             mc->GetName());
       }
     }
     fMC->SetBranchArrayContainer(&fBranchArrayContainer);
     fMC->SetId(fMCEngines.size());
     // Insert the new TVirtualMC
     fMCEngines.push_back(newMC);
   }

   /// Set the TVirtualMCMultiStack
   void SetStack(TVirtualMCMultiStack* stack)
   {
     if(fStack && fStack != stack) {
       Warning("SetStack", "There is already a TVirtualMCMultiStack " \
                           "which will be overriden now.");
     }
     fStack = stack;
   }

   /// Set the function called in stepping to check whether a track needs to
   /// be moved
   void RegisterSuggestTrackForMoving(
                        std::function<void(const TVirtualMC*, TVirtualMC*&)> f)
   {
     fSuggestTrackForMoving = f;
   }

   /// Set the function called to decide to which stack a primary is pushed
   void RegisterSpecifyEngineForTrack(
                        std::function<void(const TParticle*, TVirtualMC*&)> f)
   {
     fSpecifyEngineForTrack = f;
   }

   /// Construct geometry gloablly to be used by all engines
   void ConstructGlobalGeometry()
   {
     if(fIsGeometryConstructed) {
       return;
     }

     fApplication->ConstructGeometry();
     // Set top volume and close Root geometry if not yet done. Again check whether
     // the geometry was actually done using TGeo(Manager)
     if ( !gGeoManager->IsClosed() ) {
       TGeoVolume* top = dynamic_cast<TGeoVolume*>
                                     (gGeoManager->GetListOfVolumes()->First());
       if(!top) {
         Fatal("ConstructRootGeometry", "No top volume found: Apparently the " \
                                        "geometry was not constructed via TGeo");
       }
       gGeoManager->SetTopVolume(top);
       gGeoManager->CloseGeometry();
     }
     // TODO Do these methods also need their "Multi" counter part?
     fApplication->MisalignGeometry();
     fApplication->ConstructOpGeometry();

     // Initialize the branch array container from the TGeoManager to have
     // matching number of maxlevels
     fBranchArrayContainer.InitializeFromGeoManager(gGeoManager);

     fIsGeometryConstructed = kTRUE;
   }

   /// Do all initialisation steps at once including a custom user init as
   /// lambda.
   void InitTransport(std::function<void(TVirtualMC*)> customInit =
                                                       [](TVirtualMC*){})
   {
     // 1. Construct geometry
     // 2. Initialize VMCs to pick that up
     // 3. Set the stack

     if(!fIsGeometryConstructed) {
       Fatal("InitTransport","Geometry needs to be constructed first");
     }
     // Check whether there is a TVirtualMCMultiStack and fail if not.
     if(!fStack) {
       Fatal("InitTransport","A TVirtualMCMultiStack is required");
     }

     fStack->RegisterSpecifyStackForTrack([this](const TParticle* particle,
                                                 Int_t& stackID)
                                     {
                                       stackID = -1;
                                       TVirtualMC* mc = nullptr;
                                       this->fSpecifyEngineForTrack(particle, mc);
                                       if(mc) {
                                         stackID = this->fSubStackIds[mc->GetId()];
                                       }
                                     });

     fSubStackIds.reserve(fMCEngines.size());
     // Initialize engines
     for(auto& mc : fMCEngines) {
       Info("InitMCs", "Initialize engine %s", mc->GetName());
       if(!mc->IsRootGeometrySupported()) {
         Fatal("InitMCs", "Engine %s does not support ROOT geometry",
                          mc->GetName());
       }
       fMC = mc;
       // Create a sub-stack and set this stack to the engine
       Int_t stackID = fStack->AddSubStack();
       fSubStackIds.push_back(stackID);
       mc->SetStack(fStack);

       // Notify to use geometry built using TGeo
       mc->SetRootGeometry();
       // Call the user custom init
       customInit(mc);
       // Further init steps for the MCs
       mc->Init();
       mc->BuildPhysics();
     }
     fIsInitialized = kTRUE;
   }

   /// Run the transport by steering engines
   void RunTransport(Int_t nofEvents)
   {
     // Set initial navigator \todo Move this to a more consistent place.
     fCurrentNavigator = gGeoManager->GetCurrentNavigator();
     // Check dryrun, so far nothing is done.
     if(nofEvents < 1) {
       Info("RunMCs", "Starting dry run.");
       return;
     }
     // Run 1 event nofEvents times
     for(Int_t i = 0; i < nofEvents; i++) {
       // Generate primaries according to the user
       fStack->ResetInternals();
       fApplication->GeneratePrimaries();
       // Call user begin event action
       fApplication->BeginEvent();

       // Loop as long there are tracks in any engine stack
       while(GetNextEngine()) {
         fMC->ProcessEvent(i);
       }
       // Call user finish event action
       fApplication->FinishEvent();
     }
     // Terminate this run
     TerminateRun();
   }

   /// Construct user geometry
   void ConstructGeometry() override final
   {
     // Don't do anything since it needs to be done globally by this class
   }

   /// Misalign user geometry (optional)
   Bool_t MisalignGeometry() override final
   {
     // Don't do anything since it needs to be done globally by this class
   }

   /// Define parameters for optical processes (optional)
   void ConstructOpGeometry() override final
   {
     // Don't do anything since it needs to be done globally by this class
   }

   /// Define sensitive detectors (optional)
   void ConstructSensitiveDetectors() override final
   {
     // Don't do anything since it needs to be done globally by this class
   }

   /// Initialize geometry
   /// (Usually used to define sensitive volumes IDs)
   void InitGeometry() override final
   {
     // Don't do anything since it needs to be done globally by this class
   }

   /// Add user defined particles (optional)
   void AddParticles() override final
   {
     // Do something before...
     fApplication->AddParticles();
     // Do something afterwards...
   }

   /// Add user defined ions (optional)
   void AddIons() override final
   {
     // Do something before...
     fApplication->AddIons();
     // Do something afterwards...
   }

   /// Generate primary particles
   void GeneratePrimaries() override final
   {
     // Don't do anything since it needs to be done globally by this class
   }

   /// Define actions at the beginning of the event
   void BeginEvent() override final
   {
     // Don't do anything since it needs to be done globally by this class
   }

   /// Define actions at the beginning of the primary track
   void BeginPrimary() override final
   {
     // Do something before...
     fApplication->BeginPrimary();
     // Do something afterwards...
   }

   /// Define actions at the beginning of each track
   void PreTrack() override final
   {
     // Do something before...
     fApplication->PreTrack();
     // Do something afterwards...
   }

   /// Define action at each step
   void Stepping() override final
   {
     // Do something before...
     fApplication->Stepping();
     // Do something afterwards...
   }

   /// Define actions at the end of each track
   void PostTrack() override final
   {
     // Do something before...
     fApplication->PostTrack();
     // Do something afterwards...
   }

   /// Define actions at the end of the primary track
   void FinishPrimary() override final
   {
     // Do something before...
     fApplication->FinishPrimary();
     // Do something afterwards...
   }

   /// Define actions at the end of the event
   void FinishEvent() override final
   {
     // Don't do anything since it needs to be done globally by this class
   }

   /// Define action at each step for Geane
   void GeaneStepping() override final
   {
     // Do something before...
     fApplication->GeaneStepping();
     // Do something afterwards...
   }

 private:
   /// Choose next engines to be run in the loop
   Bool_t GetNextEngine()
   {
     // \note Kind of brute force selection by just checking the engine's stack
     for(UInt_t i = 0; i < fMCEngines.size(); i++) {
       Int_t stackId = fSubStackIds[i];
       if(fStack->HasTracks(stackId)) {
         fMC = fMCEngines[i];
         fStack->UseSubStack(stackId);
         return kTRUE;
       }
     }
     // No track to be processed.
     return kFALSE;
   }

   /// Terminate the run for all engines
   void TerminateRun()
   {
     for(auto& mc : fMCEngines) {
      mc->TerminateRun();
     }
   }

   /// Check whether the user's criteria call for moving the track from the
   /// current engine
   Bool_t SuggestTrackForMoving()
   {
     // Start off with nullptr.
     fTargetMCCached = nullptr;
     // Ask the user implemented lambda what the target engine for this track
     // should be.
     fSuggestTrackForMoving(fMC, fTargetMCCached);
     // If target engine is not specified or target engine is current engine
     // do nothing
     if(!fTargetMCCached || fMC == fTargetMCCached) {
       return kFALSE;
     }
     fMC->TrackPosition(fCurrentPosition);
     fMC->TrackMomentum(fCurrentMomentum);

     // Get an idle geometry state cache and set from current navigator.
     Int_t geoStateIndex = -1;
     fCachedGeoState = fBranchArrayContainer.GetNewGeoState(geoStateIndex);
     fCachedGeoState->InitFromNavigator(fCurrentNavigator);

     // Tell the stack to move the track to stack with id and...
     fStack->MoveCurrentTrack(fSubStackIds[fTargetMCCached->GetId()],
                              fCurrentPosition, fCurrentMomentum,
                              geoStateIndex);
     // ...stop this track
     fMC->StopTrack();
     return kTRUE;
   }

 private:
   /// The concrete application pointer
   A* fApplication;
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

   ClassDefOverride(TMCManagerMulti,1)
};

#endif /* ROOT_TMCManagerMulti */
