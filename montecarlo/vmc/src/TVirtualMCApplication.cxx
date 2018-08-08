// @(#)root/vmc:$Id$
// Author: Ivana Hrivnacova, 27/03/2002

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TVirtualMCApplication.h"
#include "TVirtualMC.h"
#include "TMCSelectionCriteria.h"
#include "TError.h"
#include "TGeoManager.h"

/** \class TVirtualMCApplication
    \ingroup vmc

Interface to a user Monte Carlo application.

*/

ClassImp(TVirtualMCApplication);

TMCThreadLocal TVirtualMCApplication* TVirtualMCApplication::fgInstance = 0;
TVirtualMC* TVirtualMCApplication::fCurrentMCEngine = nullptr;

////////////////////////////////////////////////////////////////////////////////
///
/// Standard constructor
///

TVirtualMCApplication::TVirtualMCApplication(const char *name,
                                             const char *title)
  : TNamed(name,title)
{
   if (fgInstance) {
      Fatal("TVirtualMCApplication",
            "Attempt to create two instances of singleton.");
   }
   fgInstance = this;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Default constructor
///

TVirtualMCApplication::TVirtualMCApplication()
  : TNamed()
{
   fgInstance = this;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Destructor
///

TVirtualMCApplication::~TVirtualMCApplication()
{
   fgInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Register a new VMC
///

void TVirtualMCApplication::RegisterMC(TVirtualMC* mc)
{
  // make sure, at least engine names are unique
  for(auto& en : fMCEngines) {
    if(strcmp(en->GetName(), mc->GetName()) == 0) {
      Fatal("TVirtualMCApplication::RegisterMC", "There is already an engine with name %s.", mc->GetName());
    }
  }
  // new engine, hence push back
  fMCEngines.push_back(mc);
  // Provide selection criteria
  fTMCSelectionCriteria.push_back(new TMCSelectionCriteria(mc->GetName()));
  fCurrentMCEngine = mc;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Add TMCSelectionCriteria
///

TMCSelectionCriteria* TVirtualMCApplication::GetSelectionCriteria(const char* name)
{
   for(auto sc : fTMCSelectionCriteria) {
     if(strcmp(sc->GetName(), name) == 0) {
       return sc;
     }
   }
   Fatal("TVirtualMCApplication::GetSelectionCriteria", "There is no engine with name %s.", name);
   return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
///
/// Return current pointer to current engine
///

TVirtualMC* TVirtualMCApplication::GetMC()
{
   return fCurrentMCEngine;
}

void TVirtualMCApplication::ConstructUserGeometry()
{
  ConstructGeometry();
  // Set top volume and close Root geometry if not yet done
  if ( ! gGeoManager->IsClosed() ) {
    TGeoVolume *top = (TGeoVolume*)gGeoManager->GetListOfVolumes()->First();
    gGeoManager->SetTopVolume(top);
    gGeoManager->CloseGeometry();
  }
  MisalignGeometry();
  ConstructOpGeometry();
  //InitGeometry();
}



////////////////////////////////////////////////////////////////////////////////
///
/// Return pointer to current engine
///
/*
TVirtualMC* TVirtualMCApplication::GetNextEngine()
{
  // if a new event starts, the initial engine following the primary event generation will be passed
  if(fNextEvent) {
    fCurrentMCEngine = fInitialMCEngine;
    fNextEvent = kFALSE;
    return fCurrentMCEngine;
  }
  // okay, so we are in the middle of an event...
  // 1. Check whether there are still tracks on current stack
  if(fCurrentMCEngine->GetStack()->GetNTrack() >  0) {
    return fCurrentMCEngine;
  }
  // if no tracks on current stack, find next engine
  // \todo finding the next engine should be as sophisticated as possible since in the case of multiple engines the entire run should be as efficient as possible
  for(auto& mc : fMCEngines) {
    if(mc->GetStack()->GetNTrack() >  0) {
      fCurrentMCEngine = mc;
      return mc;
    }
  }
  fNextEvent = kTRUE;
  return nullptr;
}
*/
////////////////////////////////////////////////////////////////////////////////
///
/// Update stacks
///
/*
void UpdateStacks()
{
  Int_t id = fCurrentMCEngine->CurrentVolID(copyNo);
  fVolIter = fVolMap.find(id);
  if (fVolIter!=fVolMap.end()) {
    // get the volume
    fDisVol=fVolIter->second;
    fCopyNo=fDisVol->getCopyNo();
    if(copyNo==fCopyNo) {
      // get corresponding detector
      fDisDet=fDisVol->GetDetector();
      if (fDisDet) {
        // get the name
        const std::string detName = fDisDet->GetName();
        // just return if the current criteria cover this detector
        if(fCurrentMCSelectionCriteria->HasDetector(detName)) {
          return;
        }
        // if not, find responsible engine and push to their stack
        for(Int_t i = 0; i < fMCEngines.size(); i++) {
          if(fTMCSelectionCriteria[i]->HasDetector(detName)) {
            //fMCEngines[i]->PushTrack(...)
            // stop track on the current engine
            fCurrentMCEngine->StopTrack();
            break;
          }
        }
      }
    }
  }
}
*/
////////////////////////////////////////////////////////////////////////////////
///
/// Static access method
///

TVirtualMCApplication* TVirtualMCApplication::Instance()
{
  return fgInstance;
}
