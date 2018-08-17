// @(#)root/vmc:$Name:  $:$Id$
// Authors: Benedikt Volkel 14/08/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TMCContainer
#define ROOT_TMCContainer

#include "TVirtualMC.h"
#include "TMCQueue.h"
#include "TMCSelectionCriteria.h"

struct TMCContainer
{
  TMCContainer(TVirtualMC* mc)
    : fName(mc->GetName()), fMC(mc), fQueue(nullptr), fSelectionCriteria(new TMCSelectionCriteria())
  {}

  ~TMCContainer() {}
  
  const char* fName;
  TVirtualMC* fMC;
  TMCQueue* fQueue;
  TMCSelectionCriteria* fSelectionCriteria;

  void SetQueue(TMCQueue* queue)
  {
    fQueue = queue;
    fMC->SetQueue(queue);
  }


  ClassDef(TMCContainer,1)
};

#endif /* ROOT_TMCContainer */
