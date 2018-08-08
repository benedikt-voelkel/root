// @(#)root/vmc:$Name:  $:$Id$
// Authors: Benedikt Volkel 06/08/2018

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TMCStateManager
#define ROOT_TMCStateManager

#include <vector>

#include "TNamed.h"

/// States in processing order to check/set the current state the application is in
enum class : int EVMCApplicationState = { kPreInit,											// the very first state, nothing has been done so far
																					kConstructGeometry,						// construction of bare geometry
																					kConstructOpGeometry,					// construction of optical geometry
																					kConstructGeometryFinished,		// geometry construction finished
																					kGeometryFinished,						// geometry construction finished and TGeoManager geometry closed and ready to be used by TVirtualMCs
																					kMediaInitialization, 				// initialize media properties, processes and production cuts
																					kMediaFinished,								// media finished, meaning cuts and process setting for media are set
																					kInitializeEngines,						// forward geometry and media info to native engines
																					kEnginesInitialized,					// engines are ready to run
																					kRunSimulation,								// running simulation
																					kFinishSimulation,						// simulation is being finished
																					kSimulationFinished,					// simulation run is entirely finished
																					kPostProcessing,							// some post requried post processing steps
																					kDone													// simulation and all post processing steps done, nothing else to do
																					}

class TMCStateManager : public TNamed
{
	public:

		static TMCStateManager& Instance()
		{
			static TMCStateManager inst();
			return inst;
		}
		/// Set current state
		void EnterState(EVMCApplicationState state);
		/// Get the current state
		EVMCApplicationState GetCurrentState() const;
		/// Require specific state and exit if wrong
		RequireState(EVMCApplicationState state) const;

	private:
		TMCStateManager()
			: TNamed( "TMCStateManager", "" )
		{}

	private:
		/// The current state
		EVMCApplicationState fCurrentState = kPreInit;
		/// Collection of all previous states
		std::vector<EVMCApplicationState> fProcessedStates;

	ClassDef(TMCStateManager,1)
};
#endif /* ROOT_TMCStateManager */
