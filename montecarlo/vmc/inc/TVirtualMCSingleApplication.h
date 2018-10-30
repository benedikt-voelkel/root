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

#ifndef ROOT_TVirtualMCSingleApplication
#define ROOT_TVirtualMCSingleApplication
//
// Class TVirtualMCSingleApplication
// ---------------------------
// Interface to a user Monte Carlo application using a single transport engine
// for the entire simulation setup.
//

#include "TVirtualMCApplication.h"

class TVirtualMCSingleApplication : public TVirtualMCApplication {

public:
   /// Standard constructor
   TVirtualMCSingleApplication(const char *name, const char *title);

   /// Default constructor
   TVirtualMCSingleApplication();

   /// Destructor
   virtual ~TVirtualMCSingleApplication();

   /// Static access method
   static TVirtualMCSingleApplication* Instance();

   /// Register the transport engine.
   void RegisterMC(TVirtualMC* mc) final;

   ClassDefOverride(TVirtualMCSingleApplication,1)
};

// inline functions

inline TVirtualMCSingleApplication* TVirtualMCSingleApplication::Instance()
{
  return
  static_cast<TVirtualMCSingleApplication*>(TVirtualMCApplication::Instance());
}

#endif //ROOT_TVirtualMCSingleApplication
