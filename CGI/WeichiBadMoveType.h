#ifndef WEICHIBADMOVETYPE_H
#define WEICHIBADMOVETYPE_H

enum WeichiBadMoveType {
	BADMOVE_NOT_BADMOVE,
	BADMOVE_LIFE_TERRITORY,
	BADMOVE_POTENTIAL_TRUE_EYE,
	BADMOVE_TWO_LIB_FALSE_EYE,
	BADMOVE_SELF_ATARI,
	BADMOVE_MUTUAL_ATARI,
	BADMOVE_FILLING_OWN_EYE_SHAPE,
	BADMOVE_NOT_MAKING_NAKADE_IN_OPP_CA,

	BADMOVE_SIZE	// total size, add new element before this one
};

inline string getWeichiBadMoveTypeString( WeichiBadMoveType type )
{
	switch( type ) {
	case BADMOVE_NOT_BADMOVE:					return "not bad move                ";
	case BADMOVE_LIFE_TERRITORY:				return "life territory              ";
	case BADMOVE_POTENTIAL_TRUE_EYE:			return "potential true eye          ";
	case BADMOVE_TWO_LIB_FALSE_EYE:				return "two liberty false eye       ";
	case BADMOVE_SELF_ATARI:					return "self atari                  ";
	case BADMOVE_MUTUAL_ATARI:					return "mutual atari                ";
	case BADMOVE_FILLING_OWN_EYE_SHAPE:			return "filling own eye shape       ";
	case BADMOVE_NOT_MAKING_NAKADE_IN_OPP_CA:	return "not making nakade in opp CA ";
	default:
		//should not happen
		assert( false );
		return "error move!";
	}
}

#endif