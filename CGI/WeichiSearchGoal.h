#ifndef WEICHI_SEARCH_GOAL
#define WEICHI_SEARCH_GOAL

enum WeichiSearchGoal {
	GOAL_UNKNOWN,
	GOAL_KILL,
	GOAL_LIVE,
	GOAL_LIVE_ANY,
	GOAL_KILL_ALL,
	GOAL_TERRITORY,

	GOAL_SIZE // total size, add new element before this one
};

inline string getWeichiSearchGoalString( WeichiSearchGoal goal )
{
	switch( goal ) {	
		case GOAL_UNKNOWN:		return "GOAL_UNKNOWN" ;
		case GOAL_KILL:			return "GOAL_KILL" ;
		case GOAL_LIVE:			return "GOAL_LIVE" ;		
		case GOAL_LIVE_ANY:		return "GOAL_LIVE_ANY" ;		
		case GOAL_KILL_ALL:		return "GOAL_KILL_ALL" ;
		case GOAL_TERRITORY:	return "GOAL_TERRITORY" ; 

		default:
			//should not happen
			assert( false );
			return "error goal type!";
	}
}


#endif