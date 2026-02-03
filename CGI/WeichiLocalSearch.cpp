#include "WeichiLocalSearch.h"
#include "WeichiLowLibReader.h"

WeichiBlockSearchResult WeichiLocalSearch::canSave( const WeichiBlock* block, WeichiLocalSearchNode* parentNode )
{
	assertToFile( block, const_cast<WeichiBoard*>(&m_board) );

	if( block->getLiberty()==1 ) { return canSave1Lib(block,parentNode); }
	else if( block->getLiberty()==2 ) { return canSave2Lib(block,parentNode); }

	return RESULT_SUCCESS;
}

WeichiBlockSearchResult WeichiLocalSearch::canSaveByMove( const WeichiBlock* block, const WeichiMove& move, WeichiLocalSearchNode* parentNode )
{
	assertToFile( move.getColor()==block->getColor(), const_cast<WeichiBoard*>(&m_board) );

	if( m_board.isIllegalMove(move,m_ht) ) { return RESULT_FAILED; }
	const uint blockPos = block->getiFirstGrid();

	WeichiLocalSearchNode* node = m_searchTree.newNode();
	if( node==NULL ) { return RESULT_UNKNOWN; }
	node->setMove(move);
	node->setNextSibling(parentNode->getChild());
	parentNode->setChild(node);

	play(move);

	// we find block again because after combine block by mini-play, block pointer maybe disappear
	const WeichiBlock* newBlock = m_board.getGrid(blockPos).getBlock();
	WeichiBlockSearchResult result = inverseWeichiBlockSearchResult(canKill(newBlock,node));
	node->setResult(result);

	undo();

	return result;
}

WeichiBlockSearchResult WeichiLocalSearch::canKill( const WeichiBlock* block, WeichiLocalSearchNode* parentNode )
{
	assertToFile( block, const_cast<WeichiBoard*>(&m_board) );

	if( block->getLiberty()==1 ) { return canKill1Lib(block,parentNode); }
	else if( block->getLiberty()==2 ) { return canKill2Lib(block,parentNode); }

	return RESULT_FAILED;
}

WeichiBlockSearchResult WeichiLocalSearch::canKillByMove( const WeichiBlock* block, const WeichiMove& move, WeichiLocalSearchNode* parentNode )
{
	assertToFile( move.getColor()==AgainstColor(block->getColor()), const_cast<WeichiBoard*>(&m_board) );

	if( m_board.isIllegalMove(move,m_ht) ) { return RESULT_FAILED; }
	const uint blockPos = block->getiFirstGrid();

	WeichiLocalSearchNode* node = m_searchTree.newNode();
	if( node==NULL ) { return RESULT_UNKNOWN; }
	node->setMove(move);
	node->setNextSibling(parentNode->getChild());
	parentNode->setChild(node);

	play(move);

	// we find block again because after combine block by mini-play, block pointer maybe disappear
	const WeichiBlock* newBlock = m_board.getGrid(blockPos).getBlock();
	WeichiBlockSearchResult result = inverseWeichiBlockSearchResult(canSave(newBlock,node));
	node->setResult(result);

	undo();

	return result;
}

WeichiBlockSearchResult WeichiLocalSearch::canSave1Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode )
{
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&m_board) );

	WeichiBlockSearchResult result;
	WeichiBlockSearchResult finalResult = RESULT_FAILED;

	result = canSaveByCounterattack1Lib(block,parentNode);
	if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
	else if( result==RESULT_SUCCESS ) {
		if( m_bRootFindAllSolution && parentNode==getSearchTreeRootNode() ) { finalResult = RESULT_SUCCESS; }
		else { return RESULT_SUCCESS; }
	}

	result = canSave1LibDirectly(block,parentNode);
	if( result==RESULT_SUCCESS ) { return RESULT_SUCCESS; }
	else if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }

	return finalResult;
}

WeichiBlockSearchResult WeichiLocalSearch::canSave1LibDirectly( const WeichiBlock* block, WeichiLocalSearchNode* parentNode )
{
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&m_board) );

	const Color saveColor = block->getColor();
	const uint savePos = block->getLastLiberty(m_board.getBitBoard());
	const WeichiMove saveMove(saveColor,savePos);

	return canSaveByMove(block,saveMove,parentNode);
}

WeichiBlockSearchResult WeichiLocalSearch::canSave2Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	WeichiBlockSearchResult result;
	WeichiBlockSearchResult finalResult = RESULT_FAILED;

	result = canSaveByCounterattack1Lib(block,parentNode);
	if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
	else if( result==RESULT_SUCCESS ) {
		if( m_bRootFindAllSolution && parentNode==getSearchTreeRootNode() ) { finalResult = RESULT_SUCCESS; }
		else { return RESULT_SUCCESS; }
	}

	result = canSave2LibDirectly(block,parentNode);
	if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
	else if( result==RESULT_SUCCESS ) {
		if( m_bRootFindAllSolution && parentNode==getSearchTreeRootNode() ) { finalResult = RESULT_SUCCESS; }
		else { return RESULT_SUCCESS; }
	}

	// if can counterattack 1 liberty or save directly, we won't try counterattack 2 liberty
	if( finalResult==RESULT_SUCCESS ) { return finalResult; }
	result = canSaveByCounterattack2Lib(block,parentNode);
	if( result==RESULT_SUCCESS ) { return RESULT_SUCCESS; }
	else if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }

	return finalResult;
}

WeichiBlockSearchResult WeichiLocalSearch::canSave2LibDirectly( const WeichiBlock* block, WeichiLocalSearchNode* parentNode )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	WeichiBlockSearchResult result;
	WeichiBlockSearchResult finalResult = RESULT_FAILED;
	const Color saveColor = block->getColor();

	Vector<uint,2> vSavePos;
	block->getLibertyPositions(m_board.getBitBoard(),vSavePos);
	for( uint i=0; i<vSavePos.size(); i++ ) {
		WeichiMove saveMove(saveColor,vSavePos[i]);
		const WeichiGrid& saveGrid = m_board.getGrid(saveMove);
		if( saveGrid.getPattern().getPotentialTrueEye(saveColor)
			&& (saveGrid.getStaticGrid().getStoneNbrsMap()&m_board.getOneLibBlocksBitBoard(saveColor)).empty() ) { continue; }

		result = canSaveByMove(block,saveMove,parentNode);
		if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
		else if( result==RESULT_SUCCESS ) {
			if( m_bRootFindAllSolution && parentNode==getSearchTreeRootNode() ) { finalResult = RESULT_SUCCESS; }
			else { return RESULT_SUCCESS; }
		}
	}

	return finalResult;
}

WeichiBlockSearchResult WeichiLocalSearch::canSaveByCounterattack1Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode )
{
	WeichiBlockSearchResult result;
	WeichiBlockSearchResult finalResult = RESULT_FAILED;
	const Color saveColor = block->getColor();

	uint pos;
	WeichiBitBoard bmNbrOneLib;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmNbrOneLib);
	while( (pos=bmNbrOneLib.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmNbrOneLib -= oneLibBlock->getStoneMap();

		const uint savePos = oneLibBlock->getLastLiberty(m_board.getBitBoard());
		const WeichiMove saveMove(saveColor,savePos);

		result = canSaveByMove(block,saveMove,parentNode);
		if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
		else if( result==RESULT_SUCCESS ) {
			if( m_bRootFindAllSolution && parentNode==getSearchTreeRootNode() ) { finalResult = RESULT_SUCCESS; }
			else { return RESULT_SUCCESS; }
		}
	}

	return finalResult;
}

WeichiBlockSearchResult WeichiLocalSearch::canSaveByCounterattack2Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode )
{
	WeichiBlockSearchResult result;
	WeichiBlockSearchResult finalResult = RESULT_FAILED;
	const Color saveColor = block->getColor();

	uint pos;
	WeichiBitBoard bmNbrTwoLib;
	m_board.findNeighborTwoLibBlocksBitBoard(block,bmNbrTwoLib);
	while( (pos=bmNbrTwoLib.bitScanForward())!=-1 ) {
		const WeichiBlock* twoLibBlock = m_board.getGrid(pos).getBlock();
		bmNbrTwoLib -= twoLibBlock->getStoneMap();

		Vector<uint,2> vSavePos;
		twoLibBlock->getLibertyPositions(m_board.getBitBoard(),vSavePos);
		for( uint i=0; i<vSavePos.size(); i++ ) {
			const WeichiMove saveMove(saveColor,vSavePos[i]);
			const WeichiGrid& saveGrid = m_board.getGrid(saveMove);
			if( saveGrid.getPattern().getTrueEye(saveColor) ) { continue; }

			result = canSaveByMove(block,saveMove,parentNode);
			if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
			else if( result==RESULT_SUCCESS ) {
				if( m_bRootFindAllSolution && parentNode==getSearchTreeRootNode() ) { finalResult = RESULT_SUCCESS; }
				else { return RESULT_SUCCESS; }
			}
		}
	}

	return finalResult;
}

WeichiBlockSearchResult WeichiLocalSearch::canKill1Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode )
{
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&m_board) );

	const Color killColor = AgainstColor(block->getColor());
	const uint lastLibertyPos = block->getLastLiberty(m_board.getBitBoard());
	WeichiBlockSearchResult result = RESULT_FAILED;
	if( WeichiLowLibReader::kill1Lib(m_board,m_ht,block,lastLibertyPos) ) { result = RESULT_SUCCESS; }

	WeichiLocalSearchNode* node = m_searchTree.newNode();
	if( node==NULL ) { return RESULT_UNKNOWN; }
	node->setMove(WeichiMove(killColor,lastLibertyPos));
	node->setNextSibling(parentNode->getChild());
	parentNode->setChild(node);
	node->setResult(result);

	return result;
}

WeichiBlockSearchResult WeichiLocalSearch::canKill2Lib( const WeichiBlock* block, WeichiLocalSearchNode* parentNode )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	WeichiBlockSearchResult result;
	WeichiBlockSearchResult finalResult = RESULT_FAILED;

	bool bIsDefinitelyFail = false;
	result = canKill2LibDirectly(block,bIsDefinitelyFail,parentNode);
	if( bIsDefinitelyFail ) { return result; }
	if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
	else if( result==RESULT_SUCCESS ) {
		if( m_bRootFindAllSolution && parentNode==getSearchTreeRootNode() ) { finalResult = RESULT_SUCCESS; }
		else { return RESULT_SUCCESS; }
	}

	result = canKill2LibByDefense(block,parentNode);
	if( result==RESULT_SUCCESS ) { return RESULT_SUCCESS; }
	else if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }

	return finalResult;
}

WeichiBlockSearchResult WeichiLocalSearch::canKill2LibDirectly( const WeichiBlock* block, bool& bIsDefinitelyFail, WeichiLocalSearchNode* parentNode )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	WeichiBlockSearchResult result;
	WeichiBlockSearchResult finalResult = RESULT_FAILED;
	const Color killColor = AgainstColor(block->getColor());

	Vector<uint,2> vKillPos;
	block->getLibertyPositions(m_board.getBitBoard(),vKillPos);

	// find best way to kill
	uint mustKillPos = -1;
	for( uint i=0; i<vKillPos.size(); i++ ) {
		const WeichiGrid& killGrid = m_board.getGrid(vKillPos[i]);
		if( killGrid.getLiberty()<=2 ) { continue; }

		if( mustKillPos!=-1 ) { return RESULT_FAILED; }
		mustKillPos = vKillPos[i];
	}

	if( mustKillPos!=-1 ) {
		WeichiMove killMove(killColor,mustKillPos);
		killMove = replaceKill2LibMove(killMove);
		return canKillByMove(block,killMove,parentNode);
	}

	// no must kill move, try everything
	uint numIllegal = 0;
	for( uint i=0; i<vKillPos.size(); i++ ) {
		WeichiMove killMove(killColor,vKillPos[i]);
		if( m_board.isIllegalMove(killMove,m_ht) ) { numIllegal++; continue; }

		killMove = replaceKill2LibMove(killMove);
		result = canKillByMove(block,killMove,parentNode);
		if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
		else if( result==RESULT_SUCCESS ) {
			if( m_bRootFindAllSolution && parentNode==getSearchTreeRootNode() ) { finalResult = RESULT_SUCCESS; }
			else { return RESULT_SUCCESS; }
		}
	}

	// if both kill move is illegal means this block is alive (or maybe ko?), we won't try defense move
	if( numIllegal==2 ) { bIsDefinitelyFail = true; }
	else { bIsDefinitelyFail = false; }

	return finalResult;
}

WeichiBlockSearchResult WeichiLocalSearch::canKill2LibByDefense( const WeichiBlock* block, WeichiLocalSearchNode* parentNode )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&m_board) );

	WeichiBlockSearchResult result;
	WeichiBlockSearchResult finalResult = RESULT_FAILED;
	const Color killColor = AgainstColor(block->getColor());

	uint pos;
	WeichiBitBoard bmOwnOneLib;
	m_board.findNeighborOneLibBlocksBitBoard(block,bmOwnOneLib);
	while( (pos=bmOwnOneLib.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = m_board.getGrid(pos).getBlock();
		bmOwnOneLib -= oneLibBlock->getStoneMap();

		const uint defensePos = oneLibBlock->getLastLiberty(m_board.getBitBoard());
		const WeichiMove defenseMove(killColor,defensePos);

		result = canKillByMove(block,defenseMove,parentNode);
		if( result==RESULT_UNKNOWN ) { finalResult = RESULT_UNKNOWN; }
		else if( result==RESULT_SUCCESS ) {
			if( m_bRootFindAllSolution && parentNode==getSearchTreeRootNode() ) { finalResult = RESULT_SUCCESS; }
			else { return RESULT_SUCCESS; }
		}
	}

	return finalResult;
}

WeichiMove WeichiLocalSearch::replaceKill2LibMove( const WeichiMove& killMove )
{
	/*
		we will use approach move to replace self-atari move, like following example:
			?O.OX.	
			XOOOX.
			!XXXX.
			......
		? is kill move but it's also self-atari move, we will replace it by play at !

		TODO:
			case 2:			case 3:
				O.O?.!..		O.O?....
				OOOXXOX.		OOOXXO!.
				XXXOOX..		XXXOOX..
				........		........
	*/
	const Color killColor = killMove.getColor();
	const WeichiGrid& grid = m_board.getGrid(killMove);
	if( grid.getLiberty()>1 || grid.getPattern().getAdjGridCount(killColor)==0 ) { return killMove; }

	WeichiBitBoard bmNewLib;
	uint newLib = m_board.getLibertyBitBoardAndLibertyAfterPlay(killMove,bmNewLib);
	if( newLib>1 ) { return killMove; }

	uint replacePos = bmNewLib.bitScanForward();
	return WeichiMove(killColor,replacePos);
}

void WeichiLocalSearch::play( const WeichiMove& move )
{
	assertToFile( m_board.isPlayLightStatus(), const_cast<WeichiBoard*>(&m_board) );
	assertToFile( !m_board.isIllegalMove(move,m_ht), const_cast<WeichiBoard*>(&m_board) );

	m_board.playLight(move);
	m_ht.store(m_board.getHash());
	
	/*m_board.showColorBoard();
	int k;
	cin >> k;*/
}

void WeichiLocalSearch::undo()
{
	assertToFile( m_board.isPlayLightStatus(), const_cast<WeichiBoard*>(&m_board) );
	assertToFile( m_board.getMoveList().size()>0, const_cast<WeichiBoard*>(&m_board) );

	m_ht.erase(m_board.getHash());
	m_board.undoLight();
}