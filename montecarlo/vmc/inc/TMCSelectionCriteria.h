#ifndef ROOT_TMCSelectionCriteria
#define ROOT_TMCSelectionCriteria

#include <vector>

#include "TNamed.h"

class TGeoNode;
class TGeoVolume;

class TMCSelectionCriteria : public TNamed
{
	public:
		TMCSelectionCriteria( const char* name, const char* title = "" )
			: TNamed( name, title )
		{
			fVolumeNames.clear();
		}
		/* 1. Set selection criteria for
		 * 2. Can check for overlapping criteria hence looking for conflicts
		 */
		/// Add a module specified by its name which can be a sensitive detector
		/// as well as passive ones.
		void AddVolume( const char* volName );
		// and so on...
		/// Check for overlap conflicts with another TMCCondition
		Bool_t HasConflict( const TMCSelectionCriteria& anotherConstraint ) const;

	private:
		/// Don't copy or default construct this
		// \note do it like that temporarily but move to ...() = delete. Right now, ROOT complains when doing that since the default constructor seems to be needed
		TMCSelectionCriteria();
		/// Get the pointers to all nodes and their descendants given the volume names
		void CollectNodes();
		/// The actual method deriving all nodes under a given mother volume
		void CollectDaughters(TGeoVolume* vol);

	private:
		/// names of TGeoVolumes
		std::vector<const char*> fVolumeNames;
		/// all nodes corresponding to the volumes including the descendants
		/// these pointers will be used for lookup later
		std::vector<TGeoNode*> fNodes;

	ClassDef(TMCSelectionCriteria,1)
};
#endif
