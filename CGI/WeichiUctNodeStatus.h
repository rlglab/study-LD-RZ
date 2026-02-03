#ifndef WEICHI_UCTNODE_STATUS
#define WEICHI_UCTNODE_STATUS

enum WeichiUctNodeStatus {
	UCT_STATUS_UNKNOWN,	
	UCT_STATUS_WIN,
	UCT_STATUS_LOSS,
	UCT_STATUS_RZONE_PRUNED,	

	STATUS_SIZE	// total size, add new element before this one
};

inline string getWeichiUctNodeStatus( WeichiUctNodeStatus status )
{
	switch (status) {	
		case UCT_STATUS_UNKNOWN:		return "UCT_UNKNOWN";
		case UCT_STATUS_WIN:			return "UCT_WIN";
		case UCT_STATUS_LOSS:			return "UCT_LOSS";
		case UCT_STATUS_RZONE_PRUNED:	return "UCT_RZONE_PRUNED";

		default:
			//should not happen
			assert( false );
			return "error status type!";
	}
}

#endif