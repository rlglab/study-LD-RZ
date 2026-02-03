#include "WeichiLowLibReader.h"
#include "WeichiBadMoveReader.h"
#include "WeichiLadderReader.h"

WeichiLowLibReader::WeichiLowLibReader()
{
}

void WeichiLowLibReader::save1Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block, WeichiBitBoard& bmSave, WeichiBitBoard& bmUnknown, WeichiBitBoard& bmSkip )
{
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&board) );

	const Color myColor = block->getColor();
	const uint runPos = block->getLastLiberty(board.getBitBoard());
	const WeichiMove runMove(myColor,runPos);
	const WeichiGrid& runGrid = board.getGrid(runMove);

	WeichiBitBoard bmEatNbr1Lib;
	bool bKillNbr1LibToSave = hasNbr1LibCanKill(board,ht,block,bmSave,bmEatNbr1Lib,bmSkip);

	// if can kill neighbor which can run away, don't run away
	if( bKillNbr1LibToSave ) { return; }
	
	if( !bmEatNbr1Lib.empty() && runGrid.getPattern().getEye(myColor) ) {
		// do not fill own eye
		bmSave |= bmEatNbr1Lib;
		bmSkip.SetBitOn(runPos);
	} else {
		Color oppColor = AgainstColor(myColor);
		if( runGrid.getLiberty()>=3 ) {
			// after run liberty>3
			bmSave.SetBitOn(runPos);
		} else {
			PredictPlayInfo runInfo;
			board.getPredictPlayInfoByPlay(runMove,runInfo);

			if( runInfo.m_liberty>=2 && !(runGrid.getStaticGrid().getStoneNbrsMap()&board.getTwoLibBlocksBitBoard(oppColor)).empty() ) {
				// after run can atari opp neighbor
				bmSave.SetBitOn(runPos);
			} else {
				bool bCanEatNbr = !bmEatNbr1Lib.empty();
				WeichiLadderType type = WeichiLadderReader::getLadderType(board,block,runPos);

				if( WeichiLadderReader::isInSimpleFastLadder(board,myColor,runGrid,type) ) {
					// in ladder, if no neighbor can eat don't run
					if( bCanEatNbr ) { bmSave |= bmEatNbr1Lib; bmSave.SetBitOn(runPos); }
					else { bmSkip.SetBitOn(runPos); }
				} else {
					// not in ladder, try to run away
					if( runInfo.m_liberty>2 ) {
						bmSave.SetBitOn(runPos);
					} else if( runInfo.m_liberty==2 ) {
						if( bCanEatNbr ) { bmSave |= bmEatNbr1Lib; bmSave.SetBitOn(runPos); }
						else if( runInfo.m_bHasDeadBlocks ) { bmSave.SetBitOn(runPos); }
						else { check1LibRunWillBeKillAgain(board,runPos,myColor,runInfo,bmSave,bmUnknown,bmSkip); }
					} else if( runInfo.m_liberty==1 ) {
						// TODO: replace suicide move?
						if( bCanEatNbr ) { bmSave |= bmEatNbr1Lib; bmSkip.SetBitOn(runPos); }
						else {  }
					} else {
						if( bCanEatNbr ) { bmSave |= bmEatNbr1Lib; }
						bmSkip.SetBitOn(runPos);
					}
				}
			}
		}
	}
}

void WeichiLowLibReader::kill2Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block, WeichiBitBoard& bmKill, WeichiBitBoard& bmUnknwon, WeichiBitBoard& bmSkip )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&board) );

	if( block->getStatus()==LAD_LIFE ) { return; }

	WeichiBitBoard bmNbr1LibBlock;
	board.findNeighborOneLibBlocksBitBoard(block,bmNbr1LibBlock);

	WeichiBitBoard bmLiberty = block->getLibertyBitBoard(board.getBitBoard());
	if( bmNbr1LibBlock.empty() ) {
		// just kill it
		if( isBitBoardInBlockCA(board,block,bmLiberty) ) {
			bmKill |= bmLiberty;
			return;
		} else {
			Vector<uint,2> vLibPos;
			bmLiberty.bitScanAll(vLibPos);

			if( kill2LibByCloseDoor(board,block,vLibPos[0],vLibPos[1],bmKill) ) { return; }
			kill2LibDirectly(board,block,vLibPos[0],vLibPos[1],bmKill,bmUnknwon);
			kill2LibDirectly(board,block,vLibPos[1],vLibPos[0],bmKill,bmUnknwon);
		}
	} else {
		// if neighbor has own 1 liberty, try to save
		uint pos;
		while( (pos=bmNbr1LibBlock.bitScanForward())!=-1 ) {
			const WeichiBlock* oneLibBlock = board.getGrid(pos).getBlock();
			bmNbr1LibBlock -= oneLibBlock->getStoneMap();

			save1Lib(board,ht,oneLibBlock,bmKill,bmUnknwon,bmSkip);
		}
	}
}

bool WeichiLowLibReader::canKillNbr1Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block )
{
	WeichiBitBoard bmNbr1LibBlock;
	board.findNeighborOneLibBlocksBitBoard(block,bmNbr1LibBlock);

	if( bmNbr1LibBlock.empty() ) { return false; }

	uint pos;
	while( (pos=bmNbr1LibBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = board.getGrid(pos).getBlock();
		bmNbr1LibBlock -= oneLibBlock->getStoneMap();

		// check can eat 1 liberty block
		uint kill_pos = oneLibBlock->getLastLiberty(board.getBitBoard());
		if( !kill1Lib(board,ht,oneLibBlock,kill_pos) ) { continue; }

		return true;
	}

	return false;
}

bool WeichiLowLibReader::hasNbr1LibCanKill( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block, WeichiBitBoard& bmSave, WeichiBitBoard& bmEatNbr1Lib, WeichiBitBoard& bmSkip )
{
	WeichiBitBoard bmNbr1LibBlock;
	board.findNeighborOneLibBlocksBitBoard(block,bmNbr1LibBlock);

	if( bmNbr1LibBlock.empty() ) { return false; }

	uint pos;
	bool bEatCanRun = false;
	while( (pos=bmNbr1LibBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = board.getGrid(pos).getBlock();
		bmNbr1LibBlock -= oneLibBlock->getStoneMap();

		// check can eat 1 liberty block
		uint kill_pos = oneLibBlock->getLastLiberty(board.getBitBoard());
		if( !kill1Lib(board,ht,oneLibBlock,kill_pos) ) { bmSkip.SetBitOn(pos); continue; }

		bmEatNbr1Lib.SetBitOn(kill_pos);

		// 1 liberty block have another block can eat
		WeichiBitBoard bmOneLibNbr;
		board.findNeighborOneLibBlocksBitBoard(oneLibBlock,bmOneLibNbr);
		bmOneLibNbr -= block->getStoneMap();
		if( !bmOneLibNbr.empty() ) { bEatCanRun = true; bmSave.SetBitOn(kill_pos); continue; }

		// check 1 liberty can run away
		const WeichiMove runMove(oneLibBlock->getColor(),kill_pos);
		if( board.getLibertyAfterPlay(runMove)>=2 ) { bEatCanRun = true; bmSave.SetBitOn(kill_pos); }
	}

	return bEatCanRun;
}

bool WeichiLowLibReader::isSemeaiBlock( const WeichiBoard& board, const WeichiBlock* block )
{
	// 1 stone never be semeai block
	if( block->getNumStone()==1 ) { return false; }

	const Color ownColor = block->getColor();
	const Color oppColor = AgainstColor(ownColor);

	uint pos;
	WeichiBitBoard bmNbr1Lib;
	WeichiBitBoard bmLiberty = block->getLibertyBitBoard(board.getBitBoard());
	board.findNeighborOneLibBlocksBitBoard(block,bmNbr1Lib);
	while( (pos=bmNbr1Lib.bitScanForward())!=-1 ) {
		const WeichiBlock* nbr1LibBlock = board.getGrid(pos).getBlock();
		bmNbr1Lib -= nbr1LibBlock->getStoneMap();

		if( nbr1LibBlock->getNumStone()>1 ) { return false; }
	}

	WeichiBitBoard bmLibertyDilate = (bmLiberty.dilate()-block->getStoneMap()-bmLiberty) & StaticBoard::getMaskBorder();
	WeichiBitBoard bmOppStoneWithout1Lib = board.getStoneBitBoard(oppColor) - board.getOneLibBlocksBitBoard(oppColor);
	if( (bmLibertyDilate-bmOppStoneWithout1Lib).empty() ) { return true; }

	while( (pos=bmLiberty.bitScanForward())!=-1 ) {
		WeichiMove move(ownColor,pos);
		if( board.getLibertyAfterPlay(move)>block->getLiberty() ) { return false; }
	}

	return true;
}

void WeichiLowLibReader::check1LibRunWillBeKillAgain( const WeichiBoard& board, const uint runPos, const Color myColor, const PredictPlayInfo& moveInfo, WeichiBitBoard& bmSave, WeichiBitBoard& bmUnknown, WeichiBitBoard& bmSkip )
{
	assertToFile( moveInfo.m_liberty==2, const_cast<WeichiBoard*>(&board) );

	Vector<uint,2> vLibPos;
	moveInfo.m_bmNewLiberty.bitScanAll(vLibPos);

	for( uint i=0; i<vLibPos.size(); i++ ) {
		// run again
		const WeichiMove runAgainMove(myColor,vLibPos[i]);
		assertToFile( board.getGrid(runAgainMove).isEmpty(), const_cast<WeichiBoard*>(&board) );

		WeichiBitBoard bmRunAgain;
		board.getLibertyBitBoardAfterPlay(runAgainMove,bmRunAgain);
		bmRunAgain.SetBitOff(runPos);
		bmRunAgain.SetBitOff(vLibPos[1-i]);
		uint runAgainLib = bmRunAgain.bitCount();
		if( runAgainLib>2 ) { continue; }

		// opp kill again
		const WeichiMove killAgainMove(AgainstColor(myColor),vLibPos[1-i]);
		assertToFile( board.getGrid(killAgainMove).isEmpty(), const_cast<WeichiBoard*>(&board) );

		WeichiBitBoard bmKillAgain;
		board.getLibertyBitBoardAfterPlay(killAgainMove,bmKillAgain);
		bmKillAgain.SetBitOff(runPos);
		if( bmKillAgain.bitCount()<=1 ) { continue; }
		else {
			if( runAgainLib<=1 ) { bmSkip.SetBitOn(runPos); return; }
			else if( runAgainLib==2 ) { bmUnknown.SetBitOn(runPos); return; }
		}
	}

	bmSave.SetBitOn(runPos);
}

bool WeichiLowLibReader::kill1Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block )
{
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&board) );
	return kill1Lib(board,ht,block,block->getLastLiberty(board.getBitBoard()));
}

bool WeichiLowLibReader::kill1Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block, uint lastLiberty )
{
	// check isIllegal move or snapback
	// TODO: maybe can use pattern to fast kill?
	assertToFile( block->getLiberty()==1, const_cast<WeichiBoard*>(&board) );
	assertToFile( block->getLastLiberty(board.getBitBoard())==lastLiberty, const_cast<WeichiBoard*>(&board) );

	const WeichiMove killMove(AgainstColor(block->getColor()),lastLiberty);
	if( WeichiBadMoveReader::isSnapback(board,killMove) || board.isIllegalMove(killMove,ht) ) { return false; }

	return true;
}

bool WeichiLowLibReader::killNeighbor1Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block, WeichiBitBoard& bmKill )
{
	uint pos;
	bool bCanKill = false;
	WeichiBitBoard bmNbr1LibBlock;
	board.findNeighborOneLibBlocksBitBoard(block,bmNbr1LibBlock);

	while( (pos=bmNbr1LibBlock.bitScanForward())!=-1 ) {
		const WeichiBlock* oneLibBlock = board.getGrid(pos).getBlock();
		bmNbr1LibBlock -= oneLibBlock->getStoneMap();

		if( kill1Lib(board,ht,oneLibBlock) ) { bCanKill = true; }
	}

	return bCanKill;
}

bool WeichiLowLibReader::isBitBoardInBlockCA( const WeichiBoard& board, const WeichiBlock* block, const WeichiBitBoard& bmBoard )
{
	if( bmBoard.empty() ) { return false; }

	for( uint iNumCA=0; iNumCA<block->getNumClosedArea(); iNumCA++ ) {
		const WeichiClosedArea* closedArea = block->getClosedArea(iNumCA,board.getCloseArea());
		if( closedArea->getNumBlock()>1 ) { continue; }

		if( (bmBoard-closedArea->getStoneMap()).empty() ) { return true; }
	}

	return false;
}

bool WeichiLowLibReader::kill2LibDirectly( const WeichiBoard& board, const WeichiBlock* block, uint killPos, uint runPos, WeichiBitBoard& bmKill, WeichiBitBoard& bmUnknown )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&board) );
	assertToFile( block->getStatus()!=LAD_LIFE, const_cast<WeichiBoard*>(&board) );

	Color runColor = block->getColor();
	const WeichiMove runMove(runColor,runPos);
	const WeichiGrid& runGrid = board.getGrid(runMove);
	uint runLib = 0;

	if( runGrid.getLiberty()>=3 ) { return false; }
	if( !runGrid.getPattern().getTrueEye(runColor) ) {
		WeichiBitBoard bmRunLib;
		board.getLibertyBitBoardAfterPlay(runMove,bmRunLib);
		bmRunLib.SetBitOff(killPos);
		runLib = bmRunLib.bitCount();
	}
	if( runLib>2 ) { return false; }

	Color killColor = AgainstColor(block->getColor());
	const WeichiMove killMove(killColor,killPos);
	uint killLib = board.getLibertyAfterPlay(killMove);
	if( killLib<=1 ) { return false; }
	else if( runLib<2 ) { bmKill.SetBitOn(killPos); return true; }
	else if( runLib==2 ) { bmUnknown.SetBitOn(killPos); return true; }

	// should not happened
	assertToFile( false, const_cast<WeichiBoard*>(&board) );

	return false;
}

bool WeichiLowLibReader::kill2LibByCloseDoor( const WeichiBoard& board, const WeichiBlock* block, uint pos1, uint pos2, WeichiBitBoard& bmKill )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&board) );
	assertToFile( block->getStatus()!=LAD_LIFE, const_cast<WeichiBoard*>(&board) );

	uint kill_pos = kill2LibByCloseDoor(board,block,pos1,pos2);
	if( kill_pos==-1 ) { return false; }

	bmKill.SetBitOn(kill_pos);
	return true;
}

uint WeichiLowLibReader::kill2LibByCloseDoor( const WeichiBoard& board, const WeichiBlock* block, uint pos1, uint pos2 )
{
	assertToFile( block->getLiberty()==2, const_cast<WeichiBoard*>(&board) );
	assertToFile( block->getStatus()!=LAD_LIFE, const_cast<WeichiBoard*>(&board) );

	Color myColor = AgainstColor(block->getColor());
	const WeichiGrid& grid1 = board.getGrid(pos1);
	const WeichiGrid& grid2 = board.getGrid(pos2);

	if( !checkCloseDoor(board,grid1,myColor) || !checkCloseDoor(board,grid2,myColor) ) { return -1; }

	int dir = grid1.getPattern().getCloseDoorDir(myColor);
	uint kill_pos = grid1.getStaticGrid().getNeighbor(board.realDirectionDIR4of(dir));
	assertToFile( board.getGrid(kill_pos).isEmpty(), const_cast<WeichiBoard*>(&board) );

	return kill_pos;
}

bool WeichiLowLibReader::checkCloseDoor( const WeichiBoard& board, const WeichiGrid& grid, Color myColor )
{
	int dir;

	if( grid.getStaticGrid().getLineNo()==1 ) {
		if( !grid.getPattern().getLine1CloseDoor(myColor) ) { return false; }
	} else {
		dir = grid.getPattern().getCloseDoor(myColor);
		if( !isDirectionBlockLargeThan2Lib(board,grid,dir) ) { return false; }
	}
	
	return true;
}

bool WeichiLowLibReader::isDirectionBlockLargeThan2Lib( const WeichiBoard& board, const WeichiGrid& grid, const uint dir )
{
	if( dir==0 ) { return false; }

	const WeichiBlock* dirBlock = board.getGrid(grid,board.realDirectionDIR4of(dir)).getBlock();
	assertToFile( dirBlock, const_cast<WeichiBoard*>(&board) );
	
	return (dirBlock->getLiberty()>2);
}