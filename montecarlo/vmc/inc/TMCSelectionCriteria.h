#ifndef ROOT_TMCSelectionCriteria
#define ROOT_TMCSelectionCriteria

#include <vector>

#include "TNamed.h"

class TMCSelectionCriteria : TNamed
{
	public:
		TMCSelectionCriteria( const char* name, const char* title = "" )
			: TNamed( name, title )
		{
			fModules.clear();
		}
		/* 1. Set selection criteria for
		 * 2. Can check for overlapping criteria hence looking for conflicts
		 */
		/// Add a module specified by its name which can be a sensitive detector
		/// as well as passive ones.
		void AddModule( const char* module );
		// and so on...
		/// Check for overlap conflicts with another TMCCondition
		Bool_t hasConflict( const TMCSelectionCriteria& anotherConstraint ) const;

	private:
		/// Don't copy or default construct this
		// \note do it like that temporarily but move to ...() = delete. Right now, ROOT complains when doing that since the default constructor seems to be needed
		TMCSelectionCriteria();
		TMCSelectionCriteria( const TMCSelectionCriteria& );

	private:
		/// modules which are simulated
		std::vector<const char*> fModules;

	ClassDef(TMCSelectionCriteria,1)
};
#endif
