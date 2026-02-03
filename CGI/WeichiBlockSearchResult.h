#ifndef WEICHIBLOCKSEARCHRESULT_H
#define WEICHIBLOCKSEARCHRESULT_H

enum WeichiBlockSearchResult {
	RESULT_UNKNOWN = 0,
	RESULT_SUCCESS = 1,
	RESULT_FAILED = 2,

	RESULT_SIZE	// total size, add new element before this one
};

inline string getWeichiBlockSearchResultString( WeichiBlockSearchResult result )
{
	switch( result ) {
	case RESULT_UNKNOWN:	return "unknown  ";
	case RESULT_SUCCESS:	return "success  ";
	case RESULT_FAILED:		return "failed   ";
	default:
		//should not happen
		assert( false );
		return "error result!";
	}
}

inline WeichiBlockSearchResult inverseWeichiBlockSearchResult( WeichiBlockSearchResult result )
{
	assert( result<RESULT_SIZE );
	return (result==RESULT_UNKNOWN) ? RESULT_UNKNOWN : (WeichiBlockSearchResult)(result^3);
}

#endif