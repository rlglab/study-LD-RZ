#ifndef WEICHILADDERTYPE_H
#define WEICHILADDERTYPE_H

const int NUM_LADDER_TYPE = 8;
enum WeichiLadderType {
	LADDER_UP_RIGHT,
	LADDER_UP_LEFT,
	LADDER_DOWN_RIGHT,
	LADDER_DOWN_LEFT,
	LADDER_RIGHT_UP,
	LADDER_RIGHT_DOWN,
	LADDER_LEFT_UP,
	LADDER_LEFT_DOWN,

	LADDER_UNKNOWN,

	LADDER_SIZE	// total size, add new element before this one
};

inline string getWeichiLadderTypetring( WeichiLadderType type )
{
	switch( type ) {
	case LADDER_UP_RIGHT:	return "ladder_up_right";
	case LADDER_UP_LEFT:	return "ladder_up_left";
	case LADDER_DOWN_RIGHT:	return "ladder_down_right";
	case LADDER_DOWN_LEFT:	return "ladder_down_left";
	case LADDER_RIGHT_UP:	return "ladder_right_up";
	case LADDER_RIGHT_DOWN:	return "ladder_right_down";
	case LADDER_LEFT_UP:	return "ladder_left_up";
	case LADDER_LEFT_DOWN:	return "ladder_left_down";
	case LADDER_UNKNOWN:	return "ladder_unknown";
	default:
		//should not happen
		assert( false );
		return "error type!";
	}
}

#endif