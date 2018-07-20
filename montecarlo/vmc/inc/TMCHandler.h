#ifndef ROOT_TMCHandler
#define ROOT_TMCHandler

#include <vector>

class TMCHandler : TNamed
{
	public:
		static TMCHandler& Instance()
	  {
	    static TMCHandler inst("VirtualMCHandler", "VirtualMCHandler");
	    return inst;
	  }
		/// register another VMC along with the selection selection criteria
		void RegisterMC(TVirtualMC* mc, TMCSelectionCriteria* selectionCriteria)
		{
			fMCEngines.push_back(mc);
			fTMCSelectionCriteria.push_back(selectionCriteria);
		}
		/// get the next responsible engine in the chain. Assuming the engines are run consecutively
		/// if all stacks are empty, return nullptr and set flag for new event
		// \note right now that is very brute-force, but for test-purposes
		TVirtualMC* GetNextEngine()
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
		/// check if stack was modified by the TMCHandler already or whether primaries must be generated
		// \todo How to definitely decide whether a stack is ready?
		Bool_t IsStackReady() const
		{
			// so far just check whether there are already tracks on the stack
			if(fCurrentMCEngine->GetStack()->GetNTrack() > 0) {
				return kTRUE;
			}
			return kFALSE;
		}
		/* 1. Set the actual conditions for the case a certain MC transport engine
		 * 	  should be invoked. These can be phase space regions, specific particle
		 *    types, specific volumes/media and also modules/detectors.
		 *    Regarding the phase space this can be extended such that specific slices
		 *    of phase space are determined.
		 * 2. Before initialisation of MCs i.e. before materials, geometry etc. is
		 *    constructed the conditions are checked for overlap conflicts.
		 * 3. These conditions are also invoked for managing the stack to make simulation
		 *    efficient.
		 */
		/// update stack, shift/remove particles and stop tracks
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


	private:
		/// Don't copy or default construct this
		TMCHandler() = delete;
		TMCHandler( const char* name, const char* title = "" )
			: TNamed( name, title ), fCurrentMCEngine(nullptr)
		{
			fMCEngines.clear();
			fStacks.clear();
			fTMCSelectionCriteria.clear();
		}
		TMCHandler( const TMCHandler& );
		/// check for conflicting selection criteria whenever a new MC engine is registered and abort execution if conflict is found
		bool checkSelectionConflicts( const TMCSelectionCriteria* criteria ) const;

	private:
		/// registered engines
		std::vector<TVirtualMC*> fMCEngines;
		/// corresponding selection criteria
    std::vector<TMCSelectionCriteria*> fTMCSelectionCriteria;
    /// for convenience and immediate report to outside world
    TVirtualMC* fCurrentMCEngine;
		TMCSelectionCriteria* fCurrentMCSelectionCriteria;

#if !defined(__CINT__)
   static TMCThreadLocal TMCHandler* fTMCHandler; ///< Handler singleton instance
#else
   static                TMCHandler* fTMCHandler; ///< Handler singleton instance
#endif

	ClassDef(TMCHandler,1)
};
#endif
