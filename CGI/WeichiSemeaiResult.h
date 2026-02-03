#ifndef WEICHISEMEAITYPE_H
#define WEICHISEMEAITYPE_H

enum WeichiSemeaiResult {
	SEMEAI_UNKNOWN,
	SEMEAI_BASIC,
	SEMEAI_CONNECT,
	SEMEAI_NBR_1LIB,

	SEMEAI_SIZE	// total size, add new element before this one
};

inline string getWeichiSemeaiTypeString( WeichiSemeaiResult semeaiResult )
{
	switch( semeaiResult ) {
	case SEMEAI_UNKNOWN:	return "Unknown";
	case SEMEAI_BASIC:		return "semeai_basic";
	default:
		//should not happen
		assert( false );
		return "error semeai type!";
	}
}

#endif