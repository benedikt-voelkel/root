
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

bool TMCSelectionCriteria::HasConflict( const TMCSelectionCriteria& anotherConstraint ) const
{
	// not clear, which engine is responsible if both vectors of nodes are empty
	if( fNodes.empty() && anotherConstraint.fNodes.empty() ) {
		return true;
	}
	// there must not be the same node in both constraints
	for( auto& n : fNodes ) {
		if( std::find( anotherConstraint.fNodes.begin(), anotherConstraint.fNodes.end(), n ) != anotherConstraint.fNodes.end() ) {
			return true;
		}
	}
	return false;
}

void TMCSelectionCriteria::CollectDaughters(TGeoVolume* vol)
{
	TObjArray* nodes = vol->GetNodes();
	TIter next(nodes);
	TGeoNode* tmpNode = nullptr;
	while((tmpNode = dynamic_cast<TGeoNode*>(next()))) {
		fNodes.push_back(tmpNode);
		CollectDaughters(tmpNode->GetVolume());
	}
}

void TMCSelectionCriteria::CollectNodes()
{
	for( auto& v : fVolumeNames ) {
		TGeoVolume* motherVol = gGeoManager->GetVolume(v);
		// Fail in case of Unknown volume name
		if(!motherVol) {
			Fatal("TMCSelectionCriteria::CollectNodes", "Unknown volume name %s", v);
		}
		CollectDaughters(motherVol);
	}
}
