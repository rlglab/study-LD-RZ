#ifndef WEICHIKNOWLEDGEBASE_H
#define WEICHIKNOWLEDGEBASE_H

#include "WeichiBoard.h"

class WeichiThreadState ;
class WeichiMiniSearch ;

class WeichiKnowledgeBase
{
    friend class WeichiThreadState ;
	friend class WeichiMoveFeatureHandler ;
	friend class WeichiEyeHandler ;
	friend class WeichiLifeDeathKnowledge ;
	friend class WeichiConnectorHandler ;
private:

    WeichiBoard& m_board; 
    OpenAddrHashTable& m_ht ;

    WeichiKnowledgeBase( WeichiBoard& board, OpenAddrHashTable& ht )
        : m_board(board), m_ht(ht) {}

public:
	static bool isKo( const WeichiBoard& board, const WeichiMove& move, Color koColor );
	static bool isMakeKo( const WeichiBoard& board, const WeichiMove& move );

	static bool isSemeaiBlock( const WeichiBoard& board, const WeichiBlock* block );
	static bool isNakadeShape ( const WeichiBoard& board, const WeichiBitBoard& bmShape, bool bCheckTwoLine = true );

	bool isBadMoveForKillSuicide( const WeichiMove& move, const WeichiBlock* suicideBlock ) const;
	void save1Lib( const WeichiBlock* block, WeichiBitBoard& bmSave, WeichiBitBoard& bmUnknown ) const;
	void save2Lib( const WeichiBlock* block, WeichiBitBoard& bmSave, WeichiBitBoard& bmUnknown ) const;
	
	bool kill1Lib( const WeichiBlock* block, WeichiBitBoard& bmKill ) const;
	bool kill2Lib( const WeichiBlock* block, WeichiBitBoard& bmKill, WeichiBitBoard& bmUnknown ) const;
	bool kill2LibDirectly( const WeichiBlock* block, uint killPos, uint runPos, WeichiBitBoard& bmKill, WeichiBitBoard& bmUnknown ) const;
	bool kill2LibByCloseDoor( const WeichiBlock* block, uint pos1, uint pos2, WeichiBitBoard& bmKill ) const;

	bool killNeighbor1Lib( const WeichiBlock* block, WeichiBitBoard& bmKill ) const;
	bool killNeighbor2Lib( const WeichiBlock* block, WeichiBitBoard& bmKill, WeichiBitBoard& bmUnknown ) const;

	/*!
		@brief  check if move is snapback or not
		@author 5D
		@return true if it is snapback
	*/
	bool isSnapback( const WeichiMove& move ) const;

private:
	static bool isTwoLineNakadeShape ( const WeichiBoard& board, const WeichiGrid& centerGrid, const uint line2NakadePatternDir );
	bool isBitBoardInOwnCA( const WeichiBoard& board, const WeichiBlock* block, const WeichiBitBoard& bmBoard ) const;
	bool invariance_snapback( WeichiMove move ) const;
};

#endif 