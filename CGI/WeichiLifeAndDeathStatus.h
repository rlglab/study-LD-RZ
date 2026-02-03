#ifndef WEICHILIFEANDDEATHSTATE_H
#define WEICHILIFEANDDEATHSTATE_H

enum WeichiLifeAndDeathStatus {
	LAD_NOT_EYE,
	LAD_TRUE_EYE,
	LAD_LIFE,

	LAD_SIZE	// total size, add new element before this one
};

inline string getWeichiLifeAndDeathString( WeichiLifeAndDeathStatus status )
{
	switch( status ) {
	case LAD_NOT_EYE:	return "LAD_NOT_EYE   ";
	case LAD_TRUE_EYE:	return "LAD_TRUE_EYE  ";
	case LAD_LIFE:		return "LAD_LIFE      ";
	default:
		//should not happen
		assert ( false ) ;
		return "LAD_ERROR STATUS ";
	}
}

#endif