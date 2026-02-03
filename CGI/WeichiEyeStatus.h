#ifndef WEICHI_EYE_STATUS
#define WEICHI_EYE_STATUS

enum WeichiEyeStatus {
	POSSIBLE_TRUE_EYE,
	STATIC_TRUE_EYE,
	PROTECTED_TRUE_EYE,
	SLUGGISH_TRUE_EYE,
	ONE_REGION_ENOUGH_LIVE,
	BENSON_LIVE_JOINER,	
	SIZE3STONE_IN_PRISON_TRUE_EYE,
	MIAI_TO_BE_TRUE_EYE,
	PATTERN_TRUE_EYE,
	CONNECTOR_TRUE_EYE,
	EYE_UNKNOWN,

	EYE_SIZE	// total size, add new element before this one
};

inline string getEyeStatusString( WeichiEyeStatus eyeStatus )
{
	switch (eyeStatus) {	
		case POSSIBLE_TRUE_EYE:					return "POSSIBLE_TRUE_EYE";
		case PROTECTED_TRUE_EYE:				return "PROTECTED_TRUE_EYE";
		case STATIC_TRUE_EYE:					return "STATIC_TRUE_EYE";
		case SLUGGISH_TRUE_EYE:					return "SLUGGISH_TRUE_EYE";
		case ONE_REGION_ENOUGH_LIVE:			return "ONE_REGION_ENOUGH_LIVE";
		case BENSON_LIVE_JOINER:				return "BENSON_LIVE_JOINER";
		case SIZE3STONE_IN_PRISON_TRUE_EYE:		return "SIZE3STONE_IN_PRISON_TRUE_EYE";
		case MIAI_TO_BE_TRUE_EYE:				return "MIAI_TO_BE_TRUE_EYE";
		case PATTERN_TRUE_EYE:					return "PATTERN_TRUE_EYE";
		case CONNECTOR_TRUE_EYE:				return "CONNECTOR_TRUE_EYE";
		case EYE_UNKNOWN:						return "EYE_UNKNOWN";

		default:
			//should not happen
			assert( false );
			return "error status type!";
	}
}

#endif