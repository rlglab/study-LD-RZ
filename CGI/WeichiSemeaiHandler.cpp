#include "WeichiSemeaiHandler.h"

WeichiSemeaiResult WeichiSemeaiHandler::getSemeaiType( const WeichiBoard& board, const WeichiBlock* block, const WeichiBlock* semeaiBlock, Vector<uint,MAX_NUM_GRIDS>& vSemeaiPoints )
{
	vSemeaiPoints.clear();

	// 1 stone never be semeai block
	if( block->getNumStone()==1 || block->getStatus()==LAD_LIFE ) { return SEMEAI_UNKNOWN; }
	if( block->getLiberty()<MIN_SEMEAI_LIBERTY || block->getLiberty()>MAX_SEMEAI_LIBERTY ) { return SEMEAI_UNKNOWN; }

	const Color ownColor = block->getColor();
	const Color oppColor = AgainstColor(ownColor);

	WeichiBitBoard bmNbr1Lib;
	board.findNeighborOneLibBlocksBitBoard(block,bmNbr1Lib);

	// kill neighbor
	if( !bmNbr1Lib.empty() ) {
		return SEMEAI_NBR_1LIB;
	} else {
		WeichiBitBoard bmLiberty = block->getLibertyBitBoard(board.getBitBoard());
		WeichiBitBoard bmLibertyDilate = (bmLiberty.dilate()-block->getStoneMap()-bmLiberty) & StaticBoard::getMaskBorder();
		
		// connect
		if( (bmLibertyDilate&board.getStoneBitBoard(ownColor)).empty() ) {
			// new liberty point
			if( (bmLibertyDilate-board.getStoneBitBoard(oppColor)).empty() || isBasicSemeai(board,block,bmLiberty) ) {
				if( semeaiBlock!=NULL ) { findBasicSemeaiPoint(board,block,bmLiberty,semeaiBlock,vSemeaiPoints); }
				return SEMEAI_BASIC;
			}
		} else {
			if( isBasicSemeai(board,block,bmLiberty) ) {
				if( semeaiBlock!=NULL ) { findBasicSemeaiPoint(board,block,bmLiberty,semeaiBlock,vSemeaiPoints); }
				return SEMEAI_CONNECT;
			}
			//return SEMEAI_CONNECT;
		}
	}

	return SEMEAI_UNKNOWN;
}

void WeichiSemeaiHandler::findBasicSemeaiPoint( const WeichiBoard& board, const WeichiBlock* block, WeichiBitBoard bmLiberty, const WeichiBlock* semeaiBlock, Vector<uint,MAX_NUM_GRIDS>& vSemeaiPoints )
{
	/*check is semeai point:
		1. if in semeai block closed area and block liberty != block closed area -> ok
		2. if not in block closed area -> should not be 1 liberty (replace move)
	*/

	uint pos;
	Color blockColor = block->getColor();
	Color semeaiColor = AgainstColor(blockColor);
	Vector<uint,MAX_NUM_GRIDS> vReduceLibPoint;

	while( (pos=bmLiberty.bitScanForward())!=-1 ) {
		uint semeaiPos = -1;
		const WeichiClosedArea* semeaiCA = board.getGrid(pos).getClosedArea(semeaiColor);
		const WeichiClosedArea* blockCA = board.getGrid(pos).getClosedArea(blockColor);
		if( blockCA && blockCA->getNumBlock()==1 ) { semeaiPos = pos; }
		else if( semeaiCA && semeaiCA->getNumBlock()==1 ) {
			if( semeaiCA->getNumStone()<=6 && (semeaiCA->getStoneMap()-semeaiBlock->getStonenNbrMap()-board.getBitBoard()).empty() ) {
				bmLiberty -= semeaiCA->getStoneMap();
			} else { semeaiPos = pos; }
		} else {
			WeichiMove move(semeaiColor,pos);
			PredictPlayInfo moveInfo;
			board.getPredictPlayInfoByPlay(move,moveInfo);
			if( moveInfo.m_liberty>1 ) { semeaiPos = pos; }
			else if( moveInfo.m_liberty==1 ) {
				semeaiPos = adjustSemeaiPoint(board,move,moveInfo);
				if( semeaiPos==pos ) { semeaiPos = -1; }
			}
		}

		if( semeaiPos==-1 ) { continue; }
		if( semeaiBlock->getStonenNbrMap().BitIsOn(semeaiPos)
			&& (board.getGrid(semeaiPos).getStaticGrid().getStoneNbrsMap()-semeaiBlock->getStonenNbrMap()-board.getBitBoard()).empty()
			&& (board.getGrid(semeaiPos).getStaticGrid().getStoneNbrsMap()&board.getTwoLibBlocksBitBoard(blockColor)).empty() )
		{ vReduceLibPoint.push_back(semeaiPos); }
		else { vSemeaiPoints.push_back(semeaiPos); }
	}

	if( vSemeaiPoints.empty() ) {
		for( uint i=0; i<vReduceLibPoint.size(); i++ ) { vSemeaiPoints.push_back(vReduceLibPoint[i]); }
	}
}

uint WeichiSemeaiHandler::adjustSemeaiPoint( const WeichiBoard& board, const WeichiMove move, const PredictPlayInfo& moveInfo )
{
	/* Adjust Semeai Point:
		@XX!OOOX			 XOOX @
		XOOOO OX			OX O!XOX
		XXXXOOOX			OXOOOOX
		XXXXXXXX			OOXXXX
		
		PS: change ! to @
	*/

	Color moveColor = move.getColor();
	Color oppColor = AgainstColor(moveColor);
	WeichiBitBoard bmNbr1Lib = moveInfo.m_bmNewNbrStoneMap&board.getOneLibBlocksBitBoard(oppColor);

	if( bmNbr1Lib.empty() ) {
		WeichiBitBoard bmNewLib = moveInfo.m_bmNewLiberty;
		WeichiMove adjustMove(moveColor,bmNewLib.bitScanForward());
		if( board.getLibertyAfterPlay(adjustMove)>1 ) { return adjustMove.getPosition(); }
	} else {
		// random pick one to kill
		const WeichiBlock* opp1LibBlock = board.getGrid(bmNbr1Lib.bitScanForward()).getBlock();
		WeichiMove adjustMove(moveColor,opp1LibBlock->getLastLibertyPos());
		if( board.getLibertyAfterPlay(adjustMove)>1 ) { return adjustMove.getPosition(); }
	}

	return move.getPosition();
}

bool WeichiSemeaiHandler::isBasicSemeai( const WeichiBoard& board, const WeichiBlock* block, WeichiBitBoard bmLiberty )
{
	uint pos;
	Color ownColor = block->getColor();

	while( (pos=bmLiberty.bitScanForward())!=-1 ) {
		WeichiMove move(ownColor,pos);
		if( board.getLibertyAfterPlay(move)>block->getLiberty() ) { return false; }
	}

	return true;
}