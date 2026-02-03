#include "WeichiMiniSearch.h"

// 1 liberty
WeichiBlockSearchResult WeichiMiniSearch::canEscape1Lib( uint blockPos, int deep, int& nodes )
{
	const WeichiBlock* block = m_board.getGrid(blockPos).getBlock();
	Color turnColor = block->getColor();
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&m_board) );

	WeichiBlockSearchResult result;
	WeichiBlockSearchResult finalResult = RESULT_FAILED;

	// escape by counterattack
	uint pos;
	WeichiBitBoard bmNbrOneLib;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbrOneLib);
	while( (pos=bmNbrOneLib.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmNbrOneLib -= oneLibBlock->getStoneMap();

		const WeichiMove escapeMove(turnColor,oneLibBlock->getLastLiberty(m_board.getBitBoard()));
		result = canEscapeByMove(blockPos,escapeMove,deep,nodes);
		if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
		else if( result==RESULT_SUCCESS ) { return RESULT_SUCCESS; }
	}

	// escape by run
	uint lastLibertyPos = block->getLastLiberty(m_board.getBitBoard());
	const WeichiMove escapeMove(turnColor,lastLibertyPos);
	result = canEscapeByMove(blockPos,escapeMove,deep,nodes);
	if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
	else if( result==RESULT_SUCCESS ) { return RESULT_SUCCESS; }

	return finalResult;
}

void WeichiMiniSearch::findEscape1LibPossibleMoves( const WeichiBlock* block, Color turnColor, Vector<WeichiMove,MAX_NUM_GRIDS>& vPossibleMove )
{
	uint pos;
	WeichiBitBoard bmNbrOneLib;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbrOneLib);
	vPossibleMove.push_back(WeichiMove(turnColor,block->getLastLiberty(m_board.getBitBoard())));

	while( (pos=bmNbrOneLib.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmNbrOneLib -= oneLibBlock->getStoneMap();
		vPossibleMove.push_back(WeichiMove(turnColor,oneLibBlock->getLastLiberty(m_board.getBitBoard())));
	}
}

// 2 liberty
WeichiBlockSearchResult WeichiMiniSearch::canEscape2Lib( uint blockPos, int deep, int& nodes )
{
	const WeichiBlock* block = m_board.getGrid(blockPos).getBlock();
	Color turnColor = block->getColor();
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	WeichiBlockSearchResult result;
	WeichiBlockSearchResult finalResult = RESULT_FAILED;

	// escape by run
	Vector<uint,2> vEscape;
	block->getLibertyPositions(m_board.getBitBoard(),vEscape);
	for( uint i=0; i<vEscape.size(); i++ ) {
		const WeichiMove escapeMove(turnColor,vEscape[i]);
		result = canEscapeByMove(blockPos,escapeMove,deep,nodes);
		if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
		else if( result==RESULT_SUCCESS ) { return RESULT_SUCCESS; }
	}

	// escape by counterattack
	uint pos;
	WeichiBitBoard bmNbrOneLib;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbrOneLib);
	while( (pos=bmNbrOneLib.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmNbrOneLib -= oneLibBlock->getStoneMap();

		const WeichiMove escapeMove(turnColor,oneLibBlock->getLastLiberty(m_board.getBitBoard()));
		result = canEscapeByMove(blockPos,escapeMove,deep,nodes);
		if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
		else if( result==RESULT_SUCCESS ) { return RESULT_SUCCESS; }
	}

	WeichiBitBoard bmNbrTwoLib;
	m_board.findNeighborTwoLibBlocksBitBoard(block,bmNbrTwoLib);
	while( (pos=bmNbrTwoLib.bitScanForward())!=-1 ) {
		const WeichiBlock* twoLibBlock = m_board.getGrid(pos).getBlock();
		bmNbrTwoLib -= twoLibBlock->getStoneMap();

		Vector<uint,2> vLibPos;
		twoLibBlock->getLibertyPositions(m_board.getBitBoard(),vLibPos);
		for( uint i=0; i<vLibPos.size(); i++ ) {
			const WeichiMove escapeMove(turnColor,vLibPos[i]);
			result = canEscapeByMove(blockPos,escapeMove,deep,nodes);
			if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
			else if( result==RESULT_SUCCESS ) { return RESULT_SUCCESS; }
		}
	}

	return finalResult;
}

WeichiBlockSearchResult WeichiMiniSearch::canCapture2Lib( uint blockPos, int deep, int& nodes )
{
	const WeichiBlock* block = m_board.getGrid(blockPos).getBlock();
	Color turnColor = AgainstColor(block->getColor());
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	WeichiBlockSearchResult result;
	WeichiBlockSearchResult finalResult = RESULT_FAILED;

	// capture by eat directly
	Vector<uint,2> vCapture;
	block->getLibertyPositions(m_board.getBitBoard(),vCapture);

	uint mustKillPos = -1;
	for( uint i=0; i<vCapture.size(); i++ ) {
		// find best way to kill
		const WeichiGrid& captureGrid = m_board.getGrid(vCapture[i]);
		if( captureGrid.getLiberty()<=2 ) { continue; }

		if( mustKillPos!=-1 ) { return RESULT_FAILED; }
		mustKillPos = vCapture[i];
	}

	if( mustKillPos!=-1 ) {
		const WeichiMove captureMove(turnColor,mustKillPos);
		return canCaptureByMove(blockPos,captureMove,deep,nodes);
	}

	// no must kill move, try everything
	bool bIsIllegal[2] = { true, true };
	for( uint i=0; i<vCapture.size(); i++ ) {
		const WeichiMove captureMove(turnColor,vCapture[i]);
		if( m_board.isIllegalMove(captureMove,m_ht) ) { continue; }

		bIsIllegal[i] = false;
		result = canCaptureByMove(blockPos,captureMove,deep,nodes);
		if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
		else if( result==RESULT_SUCCESS ) { return RESULT_SUCCESS; }
	}

	// if both capture move is illegal move, don't protect own
	if( bIsIllegal[0] && bIsIllegal[1] ) { return finalResult; }

	// capture by protect own
	uint pos;
	WeichiBitBoard bmNbrOwnOneLib;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbrOwnOneLib);
	while( (pos=bmNbrOwnOneLib.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmNbrOwnOneLib -= oneLibBlock->getStoneMap();

		const WeichiMove captureMove(turnColor,oneLibBlock->getLastLiberty(m_board.getBitBoard()));
		result = canCaptureByMove(blockPos,captureMove,deep,nodes);
		if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
		else if( result==RESULT_SUCCESS ) { return RESULT_SUCCESS; }
	}

	return finalResult;
}

void WeichiMiniSearch::findEscape2LibDirectlyPossibleMoves( const WeichiBlock* block, Color turnColor, Vector<WeichiMove,MAX_NUM_GRIDS>& vPossibleMove )
{
	Vector<uint,2> vEscape;
	block->getLibertyPositions(m_board.getBitBoard(),vEscape);
	for( uint i=0; i<vEscape.size(); i++ ) {
		vPossibleMove.push_back(WeichiMove(turnColor,vEscape[i]));
	}

	uint pos;
	WeichiBitBoard bmNbrOneLib;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbrOneLib);
	while( (pos=bmNbrOneLib.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmNbrOneLib -= oneLibBlock->getStoneMap();
		vPossibleMove.push_back(WeichiMove(turnColor,oneLibBlock->getLastLiberty(m_board.getBitBoard())));
	}
}

void WeichiMiniSearch::findEscape2LibIndirectlyPossibleMoves( const WeichiBlock* block, Color turnColor, Vector<WeichiMove,MAX_NUM_GRIDS>& vPossibleMove )
{
	uint pos;
	WeichiBitBoard bmNbrTwoLib;
	m_board.findNeighborTwoLibBlocksBitBoard(block,bmNbrTwoLib);
	while( (pos=bmNbrTwoLib.bitScanForward())!=-1 ) {
		const WeichiBlock* twoLibBlock = m_board.getGrid(pos).getBlock();
		bmNbrTwoLib -= twoLibBlock->getStoneMap();
		Vector<uint,2> vLibPos;
		twoLibBlock->getLibertyPositions(m_board.getBitBoard(),vLibPos);
		for( uint i=0; i<vLibPos.size(); i++ ) {
			vPossibleMove.push_back(WeichiMove(turnColor,vLibPos[i]));
		}
	}
}

void WeichiMiniSearch::findCapture2LibDirectlyPossibleMoves( const WeichiBlock* block, Color turnColor, Vector<WeichiMove,MAX_NUM_GRIDS>& vPossibleMove )
{
	Vector<uint,2> vCapture;
	block->getLibertyPositions(m_board.getBitBoard(),vCapture);
	for( uint i=0; i<vCapture.size(); i++ ) {
		const WeichiMove move(turnColor,vCapture[i]);
		vPossibleMove.push_back(move);

		WeichiBitBoard bmNewLib;
		uint newLib = m_board.getLibertyBitBoardAndLibertyAfterPlay(move,bmNewLib);
		if( newLib==1 ) { vPossibleMove.push_back(WeichiMove(turnColor,bmNewLib.bitScanForward())); }
	}
}

void WeichiMiniSearch::findCapture2LibIndirectlyPossibleMoves( const WeichiBlock* block, Color turnColor, Vector<WeichiMove,MAX_NUM_GRIDS>& vPossibleMove )
{
	uint pos;
	WeichiBitBoard bmNbrOneLib;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbrOneLib);
	while( (pos=bmNbrOneLib.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmNbrOneLib -= oneLibBlock->getStoneMap();

		vPossibleMove.push_back(WeichiMove(turnColor,oneLibBlock->getLastLiberty(m_board.getBitBoard())));
	}
}

bool WeichiMiniSearch::invariance( bool bIsSearchStart )
{
	return true;
}

// board play & undo
bool WeichiMiniSearch::minPlay( WeichiBoard& board, OpenAddrHashTable& ht, const WeichiMove& move )
{
	assertToFile( board.isPlayLightStatus(), const_cast<WeichiBoard*>(&board) );
	assertToFile( !board.isIllegalMove(move,ht), const_cast<WeichiBoard*>(&board) );

	bool bResult = board.playLight(move);
	CERR() << board.toString() << endl;
	ht.store(board.getHash());

	assertToFile( bResult, const_cast<WeichiBoard*>(&board) );
	return bResult;
}

bool WeichiMiniSearch::minUndo( WeichiBoard& board, OpenAddrHashTable& ht )
{
	assertToFile( board.isPlayLightStatus(), const_cast<WeichiBoard*>(&board) );

	ht.erase(board.getHash());
	board.undoLight();

	return true;
}