
#include "TMCSelectionCriteria.h"

ClassImp(TMCSelectionCriteria);

void TMCSelectionCriteria::AddModule( const char* name )
{
	fModules.push_back( name );
}

bool TMCSelectionCriteria::hasConflict( const TMCSelectionCriteria& anotherConstraint ) const
{
	// only need to check for same modules
	for( auto& am : anotherConstraint.fModules )
	{
		for( auto& tm : fModules )
		{
			if( strcmp( am, tm ) == 0 )
			{
				return true;
			}
		}
	}
	return false;
}
