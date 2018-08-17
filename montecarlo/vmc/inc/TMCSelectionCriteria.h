#ifndef ROOT_TMCSelectionCriteria
#define ROOT_TMCSelectionCriteria

#include <vector>

#include "Rtypes.h"

class TGeoNode;
class TGeoVolume;
class TGeoManager;

class TMCSelectionCriteria
{
	public:
		TMCSelectionCriteria()
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
		/// Initialize the criteria with information provided by a TGeoManager
		void Initialize(TGeoManager* geoManager);
		/// Check whether given parameters fit the given criteria exclusively
		Bool_t FitsExclusively(Int_t volId) const;
		/// Check whether given parameters fit the given criteria inclusively
		Bool_t FitsInclusively() const;


	private:
		/// names of TGeoVolumes
		std::vector<const char*> fVolumeNames;
		/// Store IDs of constructed volumes for later lookup
		std::vector<Int_t> fVolumeIDs;
		/// all nodes corresponding to the volumes including the descendants
		/// these pointers will be used for lookup later

	ClassDef(TMCSelectionCriteria,1)
};
#endif
