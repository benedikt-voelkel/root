
#include <algorithm>

#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TGeoManager.h"
#include "TError.h"

#include "TMCSelectionCriteria.h"

ClassImp(TMCSelectionCriteria);

void TMCSelectionCriteria::AddVolume( const char* volName )
{
	fVolumeNames.push_back( volName );
}

Bool_t TMCSelectionCriteria::HasConflict( const TMCSelectionCriteria& anotherConstraint ) const
{
	/// \todo Give this method an actual purpose
	return kFALSE;
}

void TMCSelectionCriteria::Initialize(TGeoManager* geoManager)
{
	for(const char* volName : fVolumeNames) {
		Int_t id = geoManager->GetUID(volName);
		if(id < 0) {
			Warning("Initialize", "Could not find volume %s. Ignore...", volName);
			continue;
		}
		fVolumeIDs.push_back(id);
	}
}

Bool_t TMCSelectionCriteria::FitsInclusively() const
{
	if(fVolumeIDs.empty()) {
		return kTRUE;
	}
	return kFALSE;
}

Bool_t TMCSelectionCriteria::FitsExclusively(Int_t volId) const
{
	if(std::find(fVolumeIDs.begin(), fVolumeIDs.end(), volId) != fVolumeIDs.end()) {
		return kTRUE;
	}
	return kFALSE;
}
