#ifndef WEICHIMINISEARCH_H
#define WEICHIMINISEARCH_H

#include "BasicType.h"
#include "WeichiBoard.h"
#include "OpenAddrHashTable.h"

/*
	This class is to find a block can be captured or escape by fast search
	Remind: we use blockPos instead of block pointer because after combine block by mini-play, block pointer maybe disappear
*/

class WeichiMiniSearch
{
private:
	WeichiBoard& m_board; 
	OpenAddrHashTable& m_ht;
	static const int MAX_SEARCH_DEEP = 50;
	static const int MAX_NODES = 300;

public:
	WeichiMiniSearch( WeichiBoard& board, OpenAddrHashTable& ht )
		: m_board (board), m_ht(ht) {}
	inline void startMiniSearch() { m_board.setPlayLightSataus(true); }
	inline void endMiniSearh() { m_board.setPlayLightSataus(false); }
	inline WeichiBlockSearchResult canEscape( const uint blockPos, int deep, int& nodes )
	{
		const WeichiBlock* block = m_board.getGrid(blockPos).getBlock();
		assertToFile( block, const_cast<WeichiBoard*>(&m_board) );
		
		if( block->getLiberty()==1 ) { return canEscape1Lib(blockPos,deep,nodes); }
		else if( block->getLiberty()==2 ) { return canEscape2Lib(blockPos,deep,nodes); }

		return RESULT_SUCCESS;
	}
	inline WeichiBlockSearchResult canCapture( const uint blockPos )
		{ int nodes = 0; return canCapture(blockPos,0,nodes); }
	inline WeichiBlockSearchResult canCapture( const uint blockPos, int deep, int& nodes )
	{
		const WeichiBlock* block = m_board.getGrid(blockPos).getBlock();
		assertToFile( block, const_cast<WeichiBoard*>(&m_board) );

		if( block->getLiberty()==1 ) { return RESULT_SUCCESS; }
		else if( block->getLiberty()==2 ) { return canCapture2Lib(blockPos,deep,nodes); }

		return RESULT_FAILED;
	}
	inline WeichiBlockSearchResult canEscapeByMove( const uint blockPos, const WeichiMove& escapeMove ) 
		{ int nodes = 0; return canEscapeByMove(blockPos,escapeMove,0,nodes); }
	inline WeichiBlockSearchResult canEscapeByMove( const uint blockPos, const WeichiMove& escapeMove, int deep, int& nodes )
	{
		if( m_board.isIllegalMove(escapeMove,m_ht) ) { return RESULT_FAILED; }
		if( deep>MAX_SEARCH_DEEP || nodes>MAX_NODES ) { return RESULT_UNKNOWN; }
		
		const WeichiBlock* block = m_board.getGrid(blockPos).getBlock();

		assertToFile( !escapeMove.isPass(), const_cast<WeichiBoard*>(&m_board) );
		assertToFile( escapeMove.getColor()==block->getColor(), const_cast<WeichiBoard*>(&m_board) );

		// now for capture move
		boardPlay(escapeMove);
		WeichiBlockSearchResult result = canCapture(blockPos,deep+1,++nodes);
		boardUndo();

		return inverseWeichiBlockSearchResult(result);
	}
	inline WeichiBlockSearchResult canCaptureByMove( const uint blockPos, const WeichiMove& captureMove, int deep, int& nodes )
	{
		if( m_board.isIllegalMove(captureMove,m_ht) ) { return RESULT_FAILED; }
		if( deep>MAX_SEARCH_DEEP || nodes>MAX_NODES ) { return RESULT_UNKNOWN; }

		const WeichiBlock* block = m_board.getGrid(blockPos).getBlock();

		assertToFile( !captureMove.isPass(), const_cast<WeichiBoard*>(&m_board) );
		assertToFile( captureMove.getColor()==AgainstColor(block->getColor()), const_cast<WeichiBoard*>(&m_board) );

		// now for escape move
		boardPlay(captureMove);
		WeichiBlockSearchResult result = canEscape(blockPos,deep+1,++nodes);
		boardUndo();

		return inverseWeichiBlockSearchResult(result);
	}

	// board play & undo
	static inline void startMiniPlay( WeichiBoard& board ) { board.setPlayLightSataus(true); }
	static inline void endMiniPlay( WeichiBoard& board ) { board.setPlayLightSataus(false); }
	static bool minPlay( WeichiBoard& board, OpenAddrHashTable& ht, const WeichiMove& move );
	static bool minUndo( WeichiBoard& board, OpenAddrHashTable& ht );

private:
	// for 1 liberty mini-search to use
	WeichiBlockSearchResult canEscape1Lib( uint blockPos, int deep, int& nodes );
	void findEscape1LibPossibleMoves( const WeichiBlock* block, Color turnColor, Vector<WeichiMove,MAX_NUM_GRIDS>& vPossibleMove );

	// for 2 liberty mini-search to use
	WeichiBlockSearchResult canEscape2Lib( uint blockPos, int deep, int& nodes );
	WeichiBlockSearchResult canCapture2Lib( uint blockPos, int deep, int& nodes );
	void findEscape2LibDirectlyPossibleMoves( const WeichiBlock* block, Color turnColor, Vector<WeichiMove,MAX_NUM_GRIDS>& vPossibleMove );
	void findEscape2LibIndirectlyPossibleMoves( const WeichiBlock* block, Color turnColor, Vector<WeichiMove,MAX_NUM_GRIDS>& vPossibleMove );
	void findCapture2LibDirectlyPossibleMoves( const WeichiBlock* block, Color turnColor, Vector<WeichiMove,MAX_NUM_GRIDS>& vPossibleMove );
	void findCapture2LibIndirectlyPossibleMoves( const WeichiBlock* block, Color turnColor, Vector<WeichiMove,MAX_NUM_GRIDS>& vPossibleMove );

	inline bool boardPlay( const WeichiMove& move )
	{
		assertToFile( m_board.isPlayLightStatus(), const_cast<WeichiBoard*>(&m_board) );
		assertToFile( !m_board.isIllegalMove(move,m_ht), const_cast<WeichiBoard*>(&m_board) );

		bool bResult = m_board.playLight(move);
		m_ht.store(m_board.getHash());
		
		assertToFile( bResult, const_cast<WeichiBoard*>(&m_board) );
		return bResult;
	}
	inline bool boardUndo()
	{
		assertToFile( m_board.isPlayLightStatus(), const_cast<WeichiBoard*>(&m_board) );

		m_ht.erase(m_board.getHash());
		m_board.undoLight();

		return true;
	}

	bool invariance( bool bIsSearchStart );
};

#endif