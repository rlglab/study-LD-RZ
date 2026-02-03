#ifndef WEICHILOCALHANDLER_H
#define WEICHILOCALHANDLER_H

#include "BasicType.h"
#include "WeichiLocalSearch.h"
#include "WeichiLocalSequence.h"

class WeichiLocalSearchHandler
{
private:
	WeichiBoard& m_board;
	OpenAddrHashTable& m_ht;
	WeichiLocalSearch m_localSearch;

	FeatureList<WeichiLocalSequence,MAX_NUM_BLOCKS*2> m_localSequence;	// since one block may have 2 sequence (save & kill)

public:
	WeichiLocalSearchHandler( WeichiBoard& board, OpenAddrHashTable& ht )
		: m_board(board), m_ht(ht), m_localSearch(board,ht) {}

	void reset();
	void findFullBoardBlockLocalSequence();
	WeichiLocalSearch& getLocalSearch() { return m_localSearch; }

private:
	void findSemeaiBlockLocalSequence( WeichiBlock* block );
	void findLowLibertyBlockLocalSequence( WeichiBlock* block );
};

#endif