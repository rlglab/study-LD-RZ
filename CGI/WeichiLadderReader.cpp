#include "WeichiLadderReader.h"
#include "WeichiMiniSearch.h"
#include "WeichiLowLibReader.h"

WeichiLadderReader::WeichiLadderReader()
{
}

bool WeichiLadderReader::isInSimpleFastLadder( const WeichiBoard& board, Color runColor, const WeichiGrid& runGrid, WeichiLadderType type )
{
	if( type<0 || type>=NUM_LADDER_TYPE ) { return false; }

	assertToFile( runGrid.isEmpty(), const_cast<WeichiBoard*>(&board) );
	assertToFile( runGrid.getPattern().getIsLadder(runColor), const_cast<WeichiBoard*>(&board) );

	const uint ladderDir = board.realDirectionDIR4of(runGrid.getPattern().getLadderDir(runColor));
	const WeichiGrid& ladderGrid = board.getGrid(runGrid,ladderDir);
	const WeichiBitBoard& bmLadderPath = ladderGrid.getStaticGrid().getLadderPath(type);
	return (board.getStoneBitBoard(runColor)&bmLadderPath).empty();
}

WeichiLadderType WeichiLadderReader::getLadderType( const WeichiBoard& board, const WeichiBlock* block )
{
	if( block->getLiberty()!=1 ) { return LADDER_UNKNOWN; }
	return getLadderType(board,block,block->getLastLiberty(board.getBitBoard()));
}

WeichiLadderType WeichiLadderReader::getLadderType( const WeichiBoard& board, const WeichiBlock* block, const uint lastLiberty )
{
	assertToFile( block->getLiberty()==1 && block->getLastLiberty(board.getBitBoard())==lastLiberty, const_cast<WeichiBoard*>(&board) );

	const WeichiGrid& runGrid = board.getGrid(lastLiberty);
	assertToFile( runGrid.getPatternIndex()<pattern33::TABLE_SIZE, const_cast<WeichiBoard*>(&board) );
	if( !runGrid.getPattern().getIsLadder(block->getColor()) ) { return LADDER_UNKNOWN; }
	
	return static_cast<WeichiLadderType>(runGrid.getPattern().getLadderType(block->getColor()));
}

bool WeichiLadderReader::isInComplicateLadder( WeichiBoard& board, OpenAddrHashTable& ht, const WeichiBlock* block )
{
	// block must be 1 liberty & cannot kill any neighbor block
	assertToFile( !WeichiLowLibReader::canKillNbr1Lib(board,ht,block), const_cast<WeichiBoard*>(&board) );
	
	if( block->getLiberty()!=1 ) { return false; }

	return true;
}

WeichiBlockSearchResult WeichiLadderReader::save_ladder( WeichiBoard& board, OpenAddrHashTable& ht, const uint blockPos, const uint savePos )
{
	const WeichiBlock* block = board.getGrid(blockPos).getBlock();
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&board) );
	assertToFile( block->getLastLiberty(board.getBitBoard())==savePos, const_cast<WeichiBoard*>(&board) );

	// check after run liberty is 2
	Vector<uint,2> vNextKillPos;
	const Color saveColor = block->getColor();
	const WeichiGrid& saveGrid = board.getGrid(savePos);
	const uint newAdjEmpty = saveGrid.getPattern().getEmptyAdjGridCount();
	if( newAdjEmpty>2 ) { return RESULT_SUCCESS; }
	else if( saveGrid.getPattern().getAdjGridCount(saveColor)==1 ) {
		if( newAdjEmpty<2 ) { return RESULT_FAILED; }
		// newAdjEmpty = 2
		const WeichiGrid& lastSaveGrid = board.getGrid(savePos);
		const uint direction = lastSaveGrid.getPattern().getEmptyPosition();
		const Vector<uint,4>& vEmptyDir = StaticBoard::getPatternAdjacentDirection(direction);
		assertToFile( vEmptyDir.size()==2, const_cast<WeichiBoard*>(&board) );
		for( uint i=0; i<vEmptyDir.size(); i++ ) {
			const uint pos = board.getGrid(lastSaveGrid,vEmptyDir[i]).getPosition();
			vNextKillPos.push_back(pos);
		}
	} else {
		WeichiBitBoard bmNewLib;
		const WeichiMove saveMove(saveColor,savePos);
		const uint newLib = board.getLibertyBitBoardAndLibertyAfterPlay(saveMove,bmNewLib);
		if( newLib>2 ) { return RESULT_SUCCESS; }
		else if( newLib<2 ) { return RESULT_FAILED; }
		bmNewLib.bitScanAll(vNextKillPos);
	}

	// after run is two liberty but will atari kill color
	const Color killColor = AgainstColor(saveColor);
	const bool bWillAtariKillColor = !(saveGrid.getStaticGrid().getStoneNbrsMap() & board.getTwoLibBlocksBitBoard(killColor)).empty();
	if( bWillAtariKillColor ) { return RESULT_SUCCESS; }

	// run
	const WeichiMove saveMove(saveColor,savePos);
	WeichiMiniSearch::minPlay(board,ht,saveMove);
	WeichiBlockSearchResult result = inverseWeichiBlockSearchResult(kill_ladder(board,ht,blockPos,vNextKillPos));
	WeichiMiniSearch::minUndo(board,ht);

	return result;
}

WeichiBlockSearchResult WeichiLadderReader::kill_ladder( WeichiBoard& board, OpenAddrHashTable& ht, const uint blockPos, const Vector<uint,2> vKillPos )
{
	const WeichiBlock* block = board.getGrid(blockPos).getBlock();
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&board) );
	assertToFile( vKillPos.size()==2, const_cast<WeichiBoard*>(&board) );

	const Color saveColor = block->getColor();
	const Color killColor = AgainstColor(saveColor);

	uint mustKillPos = -1, anotherPos = -1;
	for( uint i=0; i<vKillPos.size(); i++ ) {
		const WeichiGrid& killGrid = board.getGrid(vKillPos[i]);
		if( killGrid.getPattern().getEmptyAdjGridCount()<=2 ) { continue; }

		if( mustKillPos!=-1 ) { return RESULT_FAILED; }
		mustKillPos = vKillPos[i];
		anotherPos = vKillPos[1-i];
	}

	WeichiBlockSearchResult result = RESULT_UNKNOWN;
	if( mustKillPos!=-1 ) {
		const WeichiMove killMove(killColor,mustKillPos);
		WeichiMiniSearch::minPlay(board,ht,killMove);
		result = inverseWeichiBlockSearchResult(save_ladder(board,ht,blockPos,anotherPos));
		WeichiMiniSearch::minUndo(board,ht);
	} else {
		for( uint i=0; i<vKillPos.size(); i++ ) {
			const WeichiGrid& killGrid = board.getGrid(vKillPos[i]);

			const WeichiMove killMove(killColor,killGrid.getPosition());
			WeichiMiniSearch::minPlay(board,ht,killMove);
			result = save_ladder(board,ht,blockPos,vKillPos[1-i]);
			WeichiMiniSearch::minUndo(board,ht);

			if( result==RESULT_FAILED ) { return RESULT_SUCCESS; }
		}
	}

	return result;
}