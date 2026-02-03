#ifndef WEICHILOCALSEARCH_H
#define WEICHILOCALSEARCH_H

#include "BasicType.h"
#include "WeichiBoard.h"
#include "OpenAddrHashTable.h"
#include "WeichiLocalSearchTree.h"

class WeichiLocalSearch
{
	friend class WeichiMoveFeatureHandler;
private:
	bool m_bRootFindAllSolution;
	WeichiBoard& m_board;
	OpenAddrHashTable& m_ht;
	WeichiLocalSearchTree m_searchTree;

public:
	WeichiLocalSearch( WeichiBoard& board, OpenAddrHashTable& ht )
		: m_board(board), m_ht(ht)
	{
		reset();
	}

	// some setup for searching
	inline void reset()
	{
		m_searchTree.reset();
		m_bRootFindAllSolution = false;
	}
	inline void startLocalSearch() { m_board.setPlayLightSataus(true); }
	inline void endLocalSearch() { m_board.setPlayLightSataus(false); }
	inline void setRootFindAllSolution( bool bRootFindAllSolution ) { m_bRootFindAllSolution = bRootFindAllSolution; }
	inline WeichiLocalSearchNode* getSearchTreeRootNode() { return m_searchTree.getRootNode(); }

	// block status search
	WeichiBlockSearchResult canSave( const WeichiBlock* block, WeichiLocalSearchNode* parentNode );
	WeichiBlockSearchResult canSaveByMove( const WeichiBlock* block, const WeichiMove& move, WeichiLocalSearchNode* parentNode );

	WeichiBlockSearchResult canKill( const WeichiBlock* block, WeichiLocalSearchNode* parentNode );
	WeichiBlockSearchResult canKillByMove( const WeichiBlock* block, const WeichiMove& move, WeichiLocalSearchNode* parentNode );

private:
	WeichiBlockSearchResult canSave1Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode );
	WeichiBlockSearchResult canSave1LibDirectly( const WeichiBlock* block, WeichiLocalSearchNode* parentNode );
	WeichiBlockSearchResult canSave2Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode );
	WeichiBlockSearchResult canSave2LibDirectly( const WeichiBlock* block, WeichiLocalSearchNode* parentNode );
	WeichiBlockSearchResult canSaveByCounterattack1Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode );
	WeichiBlockSearchResult canSaveByCounterattack2Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode );

	WeichiBlockSearchResult canKill1Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode );
	WeichiBlockSearchResult canKill2Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode );
	WeichiBlockSearchResult canKill2LibDirectly( const WeichiBlock* block, bool& bIsDefinitelyFail, WeichiLocalSearchNode* parentNode );
	WeichiBlockSearchResult canKill2LibByDefense( const WeichiBlock* block, WeichiLocalSearchNode* parentNode );
	WeichiMove replaceKill2LibMove( const WeichiMove& killMove );

	void play( const WeichiMove& move );
	void undo();
};

#endif