#include "WeichiBoard.h"
#include "Configure.h"
#include "WeichiKnowledgeBase.h"
#include "WeichiThreadState.h"
#include "WeichiDynamicKomi.h"
#include "ColorMessage.h"
#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WINNT)
#include <Windows.h>
#endif

WeichiBoard::WeichiBoard()
{
	//set handler
	if( WeichiConfigure::use_closed_area ) { m_closedAreaHandler.initialize(this); }
	m_edgeHandler.initialize(this);
	m_probabilityHandler.initialize(this);	

	reset();
}

void WeichiBoard::reset()
{
	// initialize structure
	initializeState();
	initializeGrids();
	initializeMoveStack();

	// private variable
	m_hash = 0;
	m_bmBoard.Reset();
	m_bmDeadStones.Reset();
	m_bmStone.reset();
	m_boundingBox.reset();
	m_preset.clear();
	m_moves.clear();
	m_vbmStone.clear();
	m_bmOneLibBlocks.reset();
	m_bmTwoLibBlocks.reset();
	m_blockList.reset();
	m_territory.setAllAs(COLOR_NONE,MAX_NUM_GRIDS);
	m_candidates = StaticBoard::getInitCandidate();
	m_bmEverCaptured.Reset();
	m_LADKoAvailable.get(COLOR_BLACK) = true;
	m_LADKoAvailable.get(COLOR_WHITE) = true;
	m_bmLADToLifeStones.reset();
	m_bmLADToKillStones.reset();
	if (WeichiConfigure::use_closed_area) { m_closedAreaHandler.reset(); }
	if (WeichiConfigure::use_probability_playout) { m_probabilityHandler.initailizeTable(); }	
}

void WeichiBoard::initializeStatic()
{
	StaticBoard::initialize();
}

void WeichiBoard::initializeGrids()
{
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		m_grids[*it].initialize(*it);
	}
}

void WeichiBoard::initializeState()
{
	m_status.m_bIsEarlyEndGame = false;
	m_status.m_bLastIsPass = false;
	m_status.m_bIsTwoPass = false;
	m_status.m_bIsPlayoutStatus = false;
	m_status.m_bIsExpansionStatus = false;
	m_status.m_bIsPreSimulationStatus = false;
	m_status.m_bIsPlayLightStatus = false;
	m_status.m_bLastHasDeadBlocks = false;
	m_status.m_bLastHasCombineBlocks = false;
	m_status.m_bLastHasModifyClosedArea = false;
	m_status.m_ko = -1;
	m_status.m_lastKo = -1;
	m_status.m_koEater = -1;
	m_status.m_colorToPlay = COLOR_BLACK;
}

void WeichiBoard::initializeMoveStack()
{
	m_moveStacks.resize(MAX_GAME_LENGTH);
	for( uint i=0; i<MAX_GAME_LENGTH; i++ ) {
		BoardMoveStack& moveStack = m_moveStacks[i];

		BoardStatus& status = moveStack.m_status;
		status.m_bIsEarlyEndGame = false;
		status.m_bLastIsPass = false;
		status.m_bIsTwoPass = false;
		status.m_bIsPlayoutStatus = false;
		status.m_bIsExpansionStatus = false;
		status.m_bIsPreSimulationStatus = false;
		status.m_bIsPlayLightStatus = false;
		status.m_bLastHasDeadBlocks = false;
		status.m_bLastHasCombineBlocks = false;
		status.m_bLastHasModifyClosedArea = false;
		status.m_ko = -1;
		status.m_lastKo = -1;
		status.m_koEater = -1;
		status.m_colorToPlay = COLOR_NONE;

		moveStack.m_mainBlock = NULL;
		moveStack.m_bmMainBlockStoneNbrMap.Reset();
		moveStack.m_vCombineBlocks.clear();
		moveStack.m_vDeadBlocks.clear();
	}

	m_moveStacks.clear();
}

bool WeichiBoard::preset( const WeichiMove& move )
{
	/// only empty board can set stone
	if( !m_moves.empty() ) { return false; }
	if( move.getColor()==COLOR_NONE ) { return false; }
	if( move.isPass() ) { return false; }

	if( !checkSuicideRule(move) ) {
		/// no need to check ko rule
		return false;
	}
	m_preset.push_back(move);

	setStone(move,getNextMoveStack());
	m_vbmStone.push_back(m_bmStone);
	
	Color myColor = move.getColor();
	WeichiGrid& grid = getGrid(move);

	updateRadiusPatternIndex(grid,myColor);
	updateCandidateRange(move.getPosition());
	if( !isPlayLightStatus() ) { grid.setNbrLibIndex(0); }
	if( WeichiConfigure::use_probability_playout && !isPlayLightStatus() ) {
		m_probabilityHandler.updatePattern();
		m_probabilityHandler.putStone(move.getPosition());
	}

	assertToFile( invariance(), this );
	return true;
}

void WeichiBoard::recordPreviousMoveInfo( const WeichiMove& move )
{
	m_bmApproachLibBlockPos.Reset();

	const WeichiGrid& grid = getGrid(move);
	const uint splitDir = grid.getPattern().getApproachLib(move.getColor());

	const Vector<uint,8>& vSplitDir = StaticBoard::getPattern3x3Direction(splitDir);
	for( uint i=0; i<vSplitDir.size(); i++ ) {
		const uint pos = grid.getStaticGrid().getNeighbor(vSplitDir.at(i));
		const WeichiBlock* nbrBlock = getGrid(pos).getBlock();

		assertToFile( nbrBlock && nbrBlock->getColor()==move.getColor(), this );

		if( nbrBlock->getLiberty()!=2 ) { continue; }
		m_bmApproachLibBlockPos |= nbrBlock->getStonenNbrMap();
	}
}

bool WeichiBoard::play( const WeichiMove& move )
{
	assertToFile( move.getColor()==COLOR_BLACK || move.getColor()==COLOR_WHITE, this );
	assertToFile( move.isPass() || WeichiMove::isValidPosition(move.getPosition()), this );
	
	if( !isPlayLightStatus() && !move.isPass() ) { recordPreviousMoveInfo(move); }

	playLight(move);
	if( isPlayLightStatus() || move.isPass() ) { return true; }

	WeichiGrid& grid = getGrid(move);

	updateCandidateRange(move.getPosition());
	updateClosedArea(grid,getCurrentMoveStack());
	updateBlockModifyNumber(grid);
	
	assertToFile(invariance(), this);
	return true;
}

bool WeichiBoard::playLight( const WeichiMove& move )
{
	assertToFile( invariance(), this );
	assertToFile( move.getColor()==COLOR_BLACK || move.getColor()==COLOR_WHITE, this );
	assertToFile( move.isPass() || WeichiMove::isValidPosition(move.getPosition()), this );

	m_moves.push_back(move);
	setStone(move,getNextMoveStack());
	m_hash ^= StaticBoard::getHashGenerator().getTurnKey();
	m_vbmStone.push_back(m_bmStone);

	if( move.isPass() ) { return true; }
	Color myColor = move.getColor();
	WeichiGrid& grid = getGrid(move);	
	// No need to update pattern index because it is done in the setStone().
	updateRadiusPatternIndex(grid,myColor);
	if( !isPlayLightStatus() ) { grid.setNbrLibIndex(0); }
	if( WeichiConfigure::use_probability_playout && !isPlayLightStatus() ) {
		m_probabilityHandler.updatePattern();
		m_probabilityHandler.putStone(move.getPosition());
	}

	return true;
}

void WeichiBoard::undo()
{
	const WeichiMove& lastMove = m_moves.back();
	WeichiGrid& grid = getGrid(lastMove);

	undoBlockModifyNumber(grid);
	undoClosedArea(grid, getCurrentMoveStack());
	undoCandidateRange(lastMove.getPosition());

	undoLight();

	return;
}

void WeichiBoard::undoLight()
{
	const WeichiMove& lastMove = m_moves.back();
	BoardMoveStack& moveStack = m_moveStacks.back();
	m_moves.pop_back();
	m_vbmStone.pop_back();
	m_moveStacks.pop_back();

	undoStone(lastMove,moveStack);

	Color myColor = lastMove.getColor();
	WeichiGrid& grid = getGrid(lastMove);
	updateDeadGridPatternIndex(grid, myColor);

	assertToFile( invariance(), this );
}

void WeichiBoard::setStone( const WeichiMove& move, BoardMoveStack& moveStack )
{
	updateBoardStatus(moveStack);
	m_status.m_colorToPlay = AgainstColor(move.getColor());

	if( move.isPass() ) {
		m_status.m_bIsTwoPass = m_status.m_bLastIsPass;
		m_status.m_bLastIsPass = true;
		m_status.m_lastKo = m_status.m_ko;
		m_status.m_ko = -1;	// reset ko point, if ko point exists, rewrite it
		return;
	}

	m_status.m_bLastIsPass = false;
	m_status.m_bIsTwoPass = false;
	
	updateFullBoardData(move,moveStack);
	updateSiblings(move,moveStack);
}

void WeichiBoard::undoStone( const WeichiMove& lastMove, BoardMoveStack& moveStack )
{
	undoBoardStatus(moveStack);

	if( lastMove.isPass() ) {
		m_hash ^= StaticBoard::getHashGenerator().getTurnKey();
		return;
	}

	undoFullBoardData(lastMove,moveStack);
	undoSiblings(lastMove,moveStack);
}

void WeichiBoard::updateFullBoardData( const WeichiMove& move, BoardMoveStack& moveStack )
{
	uint pos = move.getPosition();
	Color myColor = move.getColor();
	WeichiGrid& grid = getGrid(move);

	grid.setColor(myColor);
	grid.setPlayedColor(myColor);
	m_hash ^= grid.getStaticGrid().getHash(myColor);
	//m_regionHandler.updateRegion(move);

	m_bmBoard.SetBitOn(pos);
	m_bmStone.get(myColor).SetBitOn(pos);

	//update boundingBox
	m_boundingBox.get(myColor).combine(grid.getPosition());
}

void WeichiBoard::updateSiblings( const WeichiMove& move, BoardMoveStack& moveStack )
{
	Color myColor = move.getColor();
	WeichiGrid& grid = getGrid(move);
	WeichiBitBoard bmCheckIndex;
	uint ko_point = -1;
	uint mainFlag = -1;
	uint numMaxCombine = 0;

	m_status.m_lastKo = m_status.m_ko;
	m_status.m_ko = -1;	// reset ko point, if ko point exists, rewrite it
	moveStack.m_mainBlock = NULL;
	m_bmUpdateGridLibPos.Reset();

	const int *iNeighbor = grid.getStaticGrid().getAdjacentNeighbors();
	for( ; *iNeighbor!=-1; iNeighbor++ ) {
		WeichiGrid& nbrGrid = getGrid(*iNeighbor);
		nbrGrid.decLiberty();

		if( nbrGrid.isEmpty() ) { continue; }
		
		WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if( bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) { continue; }

		bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
		if( nbrBlock->getColor()==myColor ) {
			moveStack.m_vCombineBlocks.push_back(nbrBlock);
			if( nbrBlock->getNumStone()>numMaxCombine ) {
				mainFlag = moveStack.m_vCombineBlocks.size()-1;
				numMaxCombine = nbrBlock->getNumStone();
				moveStack.m_mainBlock = nbrBlock;
			}
		} else {
			nbrBlock->decLiberty();
			uint liberty = nbrBlock->getLiberty();
			if( liberty==0 ) {
				assertToFile( nbrBlock->getColor()==AgainstColor(myColor), this );
				moveStack.m_vDeadBlocks.push_back(nbrBlock);
				m_bmEverCaptured |= nbrBlock->getStoneMap();
				if( nbrBlock->getNumStone()==1 ) { ko_point = nbrGrid.getPosition(); }
			} else if( liberty==1 ) {
				nbrBlock->setLastLibertyPos(nbrBlock->getLastLiberty(m_bmBoard));
			}
			addAndRemoveLibertyBitBoard(nbrBlock,liberty+1);
		}

		if( nbrBlock->getLiberty()<=2 ) { m_bmUpdateGridLibPos |= nbrBlock->getLibertyBitBoard(m_bmBoard); }
	}

	// remove main block & save main block's stone neighbor map
	if( moveStack.m_vCombineBlocks.size()>0 ) {
		moveStack.m_vCombineBlocks[mainFlag] = moveStack.m_vCombineBlocks.back();
		moveStack.m_vCombineBlocks.pop_back();
	}
	
	updateBlocks(grid,moveStack);
	updateDeadBlocks(moveStack.m_vDeadBlocks);

	updateNeighborPatternIndex( grid, myColor );

	// put stone is 1 liberty, 1 stone and has ko position
	WeichiBlock* gridBlock = grid.getBlock();
	if( gridBlock->getLiberty()==1 ) { gridBlock->setLastLibertyPos(gridBlock->getLastLiberty(m_bmBoard)); }
	if( gridBlock->getLiberty()==1 && gridBlock->getNumStone()==1 && moveStack.m_vDeadBlocks.size()==1 && ko_point!=-1 ) {
		m_status.m_ko = ko_point;
		m_status.m_koEater = grid.getPosition();
	}

	if( gridBlock->getLiberty()<=2 ) { m_bmUpdateGridLibPos |= gridBlock->getLibertyBitBoard(m_bmBoard); }
	if( !isPlayLightStatus() ) {
		uint pos;
		m_bmUpdateGridLibPos -= m_bmBoard;
		while( (pos=m_bmUpdateGridLibPos.bitScanForward())!=-1 ) {
			updatePatternLibIndex(getGrid(pos));
		}
	}
}

void WeichiBoard::updateBlocks( WeichiGrid& grid, BoardMoveStack& moveStack )
{
	m_status.m_bLastHasCombineBlocks = false;

	Vector<WeichiBlock*,4>& vCombineBlocks = moveStack.m_vCombineBlocks;

	if( vCombineBlocks.size()==0 ) {
		if( moveStack.m_mainBlock==NULL ) { createNewBlock(grid); }
		else { addGridToBlock(grid,moveStack); }
	} else {
		combineBlocks(grid,moveStack);
	}
}

void WeichiBoard::createNewBlock( WeichiGrid& grid )
{
	WeichiBlock* newBlock = m_blockList.NewOne();
	newBlock->init(grid);
	grid.setBlock(newBlock);
	addLibertyBlock(newBlock,newBlock->getLiberty());
}

void WeichiBoard::addGridToBlock( WeichiGrid& grid, BoardMoveStack& moveStack )
{
	WeichiBlock* mainBlock = moveStack.m_mainBlock;

	// backup stoneNbrMap
	moveStack.m_bmMainBlockStoneNbrMap = mainBlock->getStonenNbrMap();

	// add grid to block
	uint org_lib = mainBlock->getLiberty();
	mainBlock->addGrid(grid,getBitBoard());
	if( !isPlayLightStatus() ) { mainBlock->getBoundingBox().combine(grid.getPosition()); }
	grid.setBlock(mainBlock);
	addLibertyStone(mainBlock,grid.getPosition(),org_lib);
}

void WeichiBoard::combineBlocks( WeichiGrid& grid, BoardMoveStack& moveStack )
{
	WeichiBlock* mainBlock = moveStack.m_mainBlock;
	Vector<WeichiBlock*,4>& vCombineBlocks = moveStack.m_vCombineBlocks;

	m_status.m_bLastHasCombineBlocks = true;

	// backup stoneNbrMap
	moveStack.m_bmMainBlockStoneNbrMap = mainBlock->getStonenNbrMap();

	// merge neighbor block
	for( uint i=0; i<vCombineBlocks.size(); i++ ) {
		removeLibertyBlock(vCombineBlocks[i],vCombineBlocks[i]->getLiberty());
		mainBlock->combineBlockWithoutUpdateLib(vCombineBlocks[i]);
		if( !isPlayLightStatus() ) { mainBlock->getBoundingBox().combine(vCombineBlocks[i]->getBoundingBox()); }
		setGridToBlock(vCombineBlocks[i]->getStoneMap(),mainBlock);
		vCombineBlocks[i]->setIsUsed(false);
	}

	// update liberty and store into liberty block list
	uint org_lib = mainBlock->getLiberty();
	mainBlock->addGrid(grid,getBitBoard());
	if( !isPlayLightStatus() ) { mainBlock->getBoundingBox().combine(grid.getPosition()); }
	grid.setBlock(mainBlock);
	addAndRemoveLibertyBitBoard(mainBlock,org_lib);
}

void WeichiBoard::updateDeadBlocks( Vector<WeichiBlock*,4>& vDeadBlocks )
{
	m_status.m_bLastHasDeadBlocks = false;
	if( vDeadBlocks.size()==0 ) { return; }

	uint pos;
	Color myColor = vDeadBlocks[0]->getColor();
	Color oppColor = AgainstColor(myColor);
	m_bmDeadStones.Reset();
	m_status.m_bLastHasDeadBlocks = true;

	for( uint i=0; i<vDeadBlocks.size(); i++ ) {
		WeichiBlock *deadBlock = vDeadBlocks[i];

		WeichiBitBoard bmStone = deadBlock->getStoneMap();
		while( (pos=bmStone.bitScanForward())!=-1 ) {
			WeichiGrid& stoneGrid = getGrid(pos);
			m_bmUpdateGridLibPos |= stoneGrid.getStaticGrid().getStoneNbrsMap();
			//m_regionHandler.updateRegion(WeichiMove(deadBlock->getColor(),pos));

			stoneGrid.setBlock(NULL);
			stoneGrid.setColor(COLOR_NONE);
			updateDeadGridPatternIndex(stoneGrid, deadBlock->getColor());

			// update dead stone's neighbor block
			WeichiBitBoard bmCheckIndex;
			const int *iNeighbor = stoneGrid.getStaticGrid().getAdjacentNeighbors();
			for( ; *iNeighbor!=-1; iNeighbor++ ) {
				WeichiGrid& nbrGrid = getGrid(*iNeighbor);
				nbrGrid.incLiberty();

				if( nbrGrid.getColor()==oppColor ) {
					WeichiBlock *nbrBlock = nbrGrid.getBlock();
					if( bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) { continue; }

					bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
					nbrBlock->incLiberty();
					addAndRemoveLibertyBitBoard(nbrBlock,nbrBlock->getLiberty()-1);

					m_bmUpdateGridLibPos |= nbrBlock->getLibertyBitBoard(m_bmBoard);
				}
			}

			updateDeadRadiusPatternIndex(stoneGrid,myColor);
			if( !isPlayLightStatus() ) { m_candidates.addCandidate(pos); }
		}

		deadBlock->setIsUsed(false);
		m_hash ^= deadBlock->getHash();
		m_bmDeadStones |= deadBlock->getStoneMap();
		m_bmBoard -= deadBlock->getStoneMap();
		m_bmStone.get(myColor) -= deadBlock->getStoneMap();
	}
}

void WeichiBoard::updateClosedArea( WeichiGrid& grid, BoardMoveStack& moveStack )
{
	if( !WeichiConfigure::use_closed_area ) { return; }
	
	m_closedAreaHandler.updateClosedArea(grid,moveStack);
	m_closedAreaHandler.updateClosedAreaLifeAndDeath(grid);
}

void WeichiBoard::updateBlockModifyNumber( WeichiGrid& grid )
{
	assertToFile( grid.getBlock(), this );

	const uint modifyMoveNumber = m_moves.size();
	WeichiBlock* block = grid.getBlock();
	block->setModifyMoveNumber(modifyMoveNumber);

	for( const int* iNbr=grid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1; iNbr++ ) {
		WeichiGrid& nbrGrid = getGrid(*iNbr);
		if( nbrGrid.isEmpty() ) { continue; }

		WeichiBlock* nbrBlock = nbrGrid.getBlock();
		nbrBlock->setModifyMoveNumber(modifyMoveNumber);
	}
}

void WeichiBoard::undoFullBoardData( WeichiMove move, BoardMoveStack& moveStack )
{
	uint pos = move.getPosition();
	Color myColor = move.getColor();
	WeichiGrid& grid = getGrid(move);

	grid.setColor(COLOR_NONE);
	grid.setPlayedColor(myColor);
	m_hash ^= grid.getStaticGrid().getHash(myColor);
	m_hash ^= StaticBoard::getHashGenerator().getTurnKey();

	m_bmBoard.SetBitOff(pos);
	m_bmStone.get(myColor).SetBitOff(pos);

	//update boundingBox
	//m_boundingBox.get(myColor).combine(grid.getPosition());
}

void WeichiBoard::undoSiblings( WeichiMove move, BoardMoveStack& moveStack )
{
	WeichiBitBoard bmCheckIndex;
	WeichiGrid& grid = getGrid(move);
	WeichiBlock* block = grid.getBlock();

	undoDeadBlocks(moveStack.m_vDeadBlocks);
	undoCombineBlocks(grid,moveStack);

	const int *iNeighbor = grid.getStaticGrid().getAdjacentNeighbors();
	for( ; *iNeighbor!=-1; iNeighbor++ ) {
		WeichiGrid& nbrGrid = getGrid(*iNeighbor);
		nbrGrid.incLiberty();

		if( nbrGrid.isEmpty() ) { continue; }
		
		WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if( !bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) && nbrBlock->getColor()!=block->getColor() ) {
			bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
			nbrBlock->incLiberty();
			addAndRemoveLibertyBitBoard(nbrBlock,nbrBlock->getLiberty()-1);
			if( nbrBlock->getLiberty()==1 ) {
				nbrBlock->setLastLibertyPos(nbrBlock->getLastLiberty(m_bmBoard));
			}
		}
	}
}

void WeichiBoard::undoCombineBlocks( WeichiGrid& grid, BoardMoveStack& moveStack )
{
	WeichiBlock* mainBlock = grid.getBlock();
	Vector<WeichiBlock*,4>& vCombineBlocks = moveStack.m_vCombineBlocks;

	removeLibertyBlock(mainBlock,mainBlock->getLiberty());

	if( vCombineBlocks.size()==0 ) {
		if( moveStack.m_mainBlock==NULL ) {
			// remove new block
			m_blockList.FreeOne(mainBlock);
			grid.setBlock(NULL);
		} else {
			// recover stoneNbrMap
			mainBlock->setStoneNbrMap(moveStack.m_bmMainBlockStoneNbrMap);

			// remove grid from block
			mainBlock->removeGrid(grid,getBitBoard());
			grid.setBlock(NULL);
			addLibertyBlock(mainBlock,mainBlock->getLiberty());
		}
	} else {
		// recover stoneNbrMap
		mainBlock->setStoneNbrMap(moveStack.m_bmMainBlockStoneNbrMap);

		// undo combine block
		Vector<WeichiBlock*,4> vCombineBlocks = moveStack.m_vCombineBlocks;
		for( uint i=0; i<vCombineBlocks.size(); i++ ) {
			addLibertyBlock(vCombineBlocks[i],vCombineBlocks[i]->getLiberty());
			mainBlock->removeBlockWithoutUpdateLib(vCombineBlocks[i]);
			setGridToBlock(vCombineBlocks[i]->getStoneMap(),vCombineBlocks[i]);
			vCombineBlocks[i]->setIsUsed(true);
		}

		// update liberty and store into liberty block list & recover stone neighbor map
		mainBlock->removeGrid(grid,getBitBoard());
		grid.setBlock(NULL);
		addLibertyBlock(mainBlock,mainBlock->getLiberty());
	}

	if( mainBlock && mainBlock->getLiberty()==1 ) {
		mainBlock->setLastLibertyPos(mainBlock->getLastLiberty(m_bmBoard));
	}
}

void WeichiBoard::undoDeadBlocks( Vector<WeichiBlock*,4>& vDeadBlocks )
{
	if( vDeadBlocks.size()==0 ) { return; }

	uint pos;
	Color myColor = vDeadBlocks[0]->getColor();
	Color oppColor = AgainstColor(myColor);
	m_bmDeadStones.Reset();

	for( uint i=0; i<vDeadBlocks.size(); i++ ) {
		WeichiBlock* deadBlock = vDeadBlocks[i];

		m_bmDeadStones |= deadBlock->getStoneMap();
		m_hash ^= deadBlock->getHash();
		m_bmBoard |= deadBlock->getStoneMap();
		m_bmStone.get(myColor) |= deadBlock->getStoneMap();

		WeichiBitBoard bmStone = deadBlock->getStoneMap();
		while( (pos=bmStone.bitScanForward())!=-1 ) {
			WeichiGrid& stoneGrid = getGrid(pos);

			stoneGrid.setBlock(deadBlock);
			stoneGrid.setColor(myColor);

			// update dead stone's neighbor block
			WeichiBitBoard bmCheckIndex;
			const int *iNeighbor = stoneGrid.getStaticGrid().getAdjacentNeighbors();
			for( ; *iNeighbor!=-1; iNeighbor++ ) {
				WeichiGrid& nbrGrid = getGrid(*iNeighbor);
				nbrGrid.decLiberty();

				if( nbrGrid.getColor()==oppColor ) {
					WeichiBlock *nbrBlock = nbrGrid.getBlock();
					if( !bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) {
						bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
						nbrBlock->decLiberty();
						addAndRemoveLibertyBitBoard(nbrBlock,nbrBlock->getLiberty()+1);

						if( nbrBlock->getLiberty()==1 ) {
							nbrBlock->setLastLibertyPos(nbrBlock->getLastLiberty(m_bmBoard));
						}
					}
				}
			}
			updateNeighborPatternIndex(stoneGrid,myColor);
		}

		deadBlock->setIsUsed(true);
		if( deadBlock->getLiberty()==1 ) {
			deadBlock->setLastLibertyPos(deadBlock->getLastLiberty(m_bmBoard));
		}
	}
}

void WeichiBoard::setGridToBlock( WeichiBitBoard bmStone, WeichiBlock* block )
{
	uint pos;
	while( (pos=bmStone.bitScanForward())!=-1 ) {
		getGrid(pos).setBlock(block);
	}
}

void WeichiBoard::setBackupFlag()
{
	m_backupFlag.m_status = m_status;
	m_backupFlag.m_hash = m_hash;
	m_backupFlag.m_bmBoard = m_bmBoard;
	m_backupFlag.m_bmDeadStones = m_bmDeadStones;
	m_backupFlag.m_bmApproachLibBlockPos = m_bmApproachLibBlockPos;
	m_backupFlag.m_bmStone = m_bmStone;
	m_backupFlag.m_boundingBox = m_boundingBox;
	m_backupFlag.m_candidates = m_candidates;

	m_backupFlag.m_moveSize = m_moves.size();
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		m_backupFlag.m_grids[*it] = m_grids[*it];
	}

	m_backupFlag.m_bmOneLibBlocks = m_bmOneLibBlocks;
	m_backupFlag.m_bmTwoLibBlocks = m_bmTwoLibBlocks;
	m_backupFlag.m_blockList = m_blockList;
	
	if( WeichiConfigure::use_closed_area ) { m_closedAreaHandler.setBackupFlag(); }
	//m_regionHandler.setBackupFlag();
}

void WeichiBoard::restoreBackupFlag()
{
	m_status = m_backupFlag.m_status;
	m_hash = m_backupFlag.m_hash;
	m_bmBoard = m_backupFlag.m_bmBoard;
	m_bmDeadStones = m_backupFlag.m_bmDeadStones;
	m_bmApproachLibBlockPos = m_backupFlag.m_bmApproachLibBlockPos;
	m_bmStone = m_backupFlag.m_bmStone;
	m_boundingBox = m_backupFlag.m_boundingBox;
	m_candidates = m_backupFlag.m_candidates;

	m_moves.resize(m_backupFlag.m_moveSize);
	m_vbmStone.resize(m_backupFlag.m_moveSize);
	m_moveStacks.resize(m_backupFlag.m_moveSize+m_preset.size());
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		m_grids[*it] = m_backupFlag.m_grids[*it];
	}

	m_bmOneLibBlocks = m_backupFlag.m_bmOneLibBlocks;
	m_bmTwoLibBlocks = m_backupFlag.m_bmTwoLibBlocks;
	m_blockList = m_backupFlag.m_blockList;

	if( WeichiConfigure::use_closed_area ) { m_closedAreaHandler.restoreBackupFlag(); }
	if( WeichiConfigure::use_probability_playout ) { m_probabilityHandler.initailizeTable(); }
	//m_regionHandler.restoreBackupFlag();

	assertToFile( invariance(), this );
}

void WeichiBoard::storeMoveBackup() 
{
	BackupFlag backupFlag ;
	backupFlag.m_status = m_status;
	backupFlag.m_hash = m_hash;
	backupFlag.m_bmBoard = m_bmBoard;
	backupFlag.m_bmDeadStones = m_bmDeadStones;
	backupFlag.m_bmApproachLibBlockPos = m_bmApproachLibBlockPos;
	backupFlag.m_bmStone = m_bmStone;
	backupFlag.m_boundingBox = m_boundingBox;
	backupFlag.m_candidates = m_candidates;	

	backupFlag.m_moveSize = m_moves.size();
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		backupFlag.m_grids[*it] = m_grids[*it];
	}

	backupFlag.m_bmOneLibBlocks = m_bmOneLibBlocks;
	backupFlag.m_bmTwoLibBlocks = m_bmTwoLibBlocks;
	backupFlag.m_blockList = m_blockList;
	backupFlag.m_bmEverCaptured = m_bmEverCaptured;

	m_backupStacks.push_back(backupFlag);

	if (WeichiConfigure::use_closed_area) { m_closedAreaHandler.storeMoveBackup(); }
}

void WeichiBoard::resumeMoveBackup()
{
	BackupFlag& backupFlag = m_backupStacks.back() ;
	m_status = backupFlag.m_status;
	m_hash = backupFlag.m_hash;
	m_bmBoard = backupFlag.m_bmBoard;
	m_bmDeadStones = backupFlag.m_bmDeadStones;
	m_bmApproachLibBlockPos = backupFlag.m_bmApproachLibBlockPos;
	m_bmStone = backupFlag.m_bmStone;
	m_boundingBox = backupFlag.m_boundingBox;
	m_candidates = backupFlag.m_candidates;

	m_moves.resize(backupFlag.m_moveSize);
	m_vbmStone.resize(backupFlag.m_moveSize);
	m_moveStacks.resize(backupFlag.m_moveSize+m_preset.size());
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		m_grids[*it] = backupFlag.m_grids[*it];
	}

	m_bmOneLibBlocks = backupFlag.m_bmOneLibBlocks;
	m_bmTwoLibBlocks = backupFlag.m_bmTwoLibBlocks;
	m_blockList = backupFlag.m_blockList;
	m_bmEverCaptured = backupFlag.m_bmEverCaptured;

	if( WeichiConfigure::use_closed_area ) { m_closedAreaHandler.resumeMoveBackup(); }
	
	m_backupStacks.pop_back() ;	
	assertToFile( invariance(), this );	

	return ;
}

void WeichiBoard::clearBackupStacks()
{
	m_backupStacks.clear();
	m_closedAreaHandler.m_backupStacks.clear();
}

WeichiBoard::BackupFlag WeichiBoard::getCurrentBoardBackupFlag()
{
	BackupFlag backupFlag;

	backupFlag.m_status = m_status;
	backupFlag.m_hash = m_hash;
	backupFlag.m_bmBoard = m_bmBoard;
	backupFlag.m_bmDeadStones = m_bmDeadStones;
	backupFlag.m_bmApproachLibBlockPos = m_bmApproachLibBlockPos;
	backupFlag.m_bmStone = m_bmStone;
	backupFlag.m_boundingBox = m_boundingBox;
	backupFlag.m_candidates = m_candidates;

	backupFlag.m_moveSize = m_moves.size();
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		backupFlag.m_grids[*it] = m_grids[*it];
	}

	backupFlag.m_bmOneLibBlocks = m_bmOneLibBlocks;
	backupFlag.m_bmTwoLibBlocks = m_bmTwoLibBlocks;
	backupFlag.m_blockList = m_blockList;

	return backupFlag ;
}

bool WeichiBoard::checkSuicideRule( WeichiMove move ) const
{
	if ( move.isPass() ) return true ;
	const WeichiGrid& grid = getGrid(move) ;
	if ( !grid.isEmpty() ) return false ;

	const StaticGrid& sgrid = grid.getStaticGrid();
	Color color = move.getColor();

	const int* nbr = sgrid.getAdjacentNeighbors();
	for ( ; *nbr != -1 ; ++nbr ) {
		const WeichiGrid& nbrGrid = m_grids[*nbr] ;
		if ( nbrGrid.isEmpty() ) return true;

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if ( nbrBlock->getColor() == color ) {
			if ( nbrBlock->getLiberty() > 1 ) return true;
		} else /*if ( nbrBlock->getColor() == AgainstColor(color) ) */ {
			if ( nbrBlock->getLiberty() == 1 ) {
				// capture opponent
				return true;
			}
		}
	}
	return false ;
}

bool WeichiBoard::checkKoRule( WeichiMove move, const OpenAddrHashTable& ht ) const
{
	// simple ko rule
	if ( m_status.m_ko == move.getPosition() ) return false;

	assert ( checkSuicideRule(move) ) ; // ensure no obey suicide rule
	assert ( !move.isPass() ) ;
	assert ( getGrid(move).isEmpty() ) ;

	const WeichiGrid& grid = getGrid(move) ;
	Color color = move.getColor();

	// never put here, never ko
	if ( !grid.isPlayed(color) ) return true;

	const StaticGrid& sgrid = grid.getStaticGrid();
	HashKey64 hashkey = m_hash ^ sgrid.getHash(color);
	Color oppColor = AgainstColor(color) ;
	WeichiBitBoard bmCheckIndex;

	const int* nbr = sgrid.getAdjacentNeighbors();
	for ( ; *nbr != -1 ; ++nbr ) {
		const WeichiGrid& nbrGrid = m_grids[*nbr] ;
		if ( nbrGrid.getColor() != oppColor ) continue ;

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if ( nbrBlock->getLiberty() == 1 && !bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) {
			// capture opponent
			bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
			hashkey ^= nbrBlock->getHash();
		}
	}
	return !ht.lookup(hashkey) ; // should not found
}

WeichiBitBoard WeichiBoard::getCaptureRZone(WeichiMove captureMove) const
{	
	assert(isCaptureMove(captureMove));

	//if (m_status.m_ko == captureMove.getPosition()) { return WeichiBitBoard(); }

	// This function would get the R-Zone after playing the capturing move(Current not playing the capture move)
	Color ownColor = captureMove.getColor() ;
	Color oppColor = AgainstColor(ownColor) ;	
	const StaticGrid& sgrid = getGrid(captureMove).getStaticGrid();
	WeichiBitBoard bmCheckIndex;
	WeichiBitBoard bmDeadStone ;
	const int* nbr = sgrid.getAdjacentNeighbors();
	for ( ; *nbr != -1 ; ++nbr ) {
		const WeichiGrid& nbrGrid = m_grids[*nbr] ;
		if ( nbrGrid.getColor() != oppColor ) continue ;

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if ( nbrBlock->getLiberty() == 1 && !bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) {
			bmDeadStone |= nbrBlock->getStoneMap() ;
			bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
		} 
	}

	WeichiBitBoard bmNbrOwnBlocks = bmDeadStone.dilate() & StaticBoard::getMaskBorder() & getStoneBitBoard(ownColor) ;	
	WeichiBitBoard bmResult = bmDeadStone ;
	int pos = 0 ;
	while( (pos=bmNbrOwnBlocks.bitScanForward()) != -1 ) {
		const WeichiGrid& grid = getGrid(pos) ;
		const WeichiBlock* ownBlock = grid.getBlock() ;
		bmResult |= ownBlock->getStoneMap() ; 
		//bmNbrOwnBlocks -= ownBlock->getStoneMap() ; 
	}

	bmResult |= getStoneBitBoardAfterPlay(captureMove);

	return bmResult;
}

WeichiBitBoard WeichiBoard::getUpdatedRZone(Color ownColor, WeichiBitBoard bmStart) const
{
	Color oppColor = AgainstColor(ownColor) ;
	WeichiBitBoard bmInput = bmStart ;
	WeichiBitBoard bmResult = bmInput ;
	WeichiBitBoard bmSuicide;

	while (bmSuicide != bmResult) {
		bmInput = (bmResult | bmSuicide) ;
		bmResult = bmInput ;
		// Legalize RZone
		int pos = 0 ;
		while ((pos=bmInput.bitScanForward()) != -1) {
			const WeichiGrid& grid = getGrid(pos) ;
			if( grid.getColor() == COLOR_NONE ) { continue; }
			const WeichiBlock* block = grid.getBlock() ;		
			if (block->getColor() != ownColor ) { continue ; }
		
			bmResult |= block->getStoneMap();
			WeichiBitBoard bmLiberty = block->getLibertyBitBoard(getBitBoard());
			int numOverLappedLib = (bmLiberty & bmResult).bitCount();
			if (numOverLappedLib >= 2) { continue; }

			// numRequiredLib = 2 or 1
			int numRequiredLib = 2 - numOverLappedLib;
			WeichiBitBoard bmRemaingedLib = bmLiberty - bmResult;
			for (int i=0; i<numRequiredLib; i++) {			
				if (bmRemaingedLib.empty()) { break; }
				bmResult.SetBitOn(bmRemaingedLib.bitScanForward());			
			}

			bmInput -= block->getStoneMap();
		}

		// Suicide R-Zone
		WeichiBitBoard bmTest = bmResult ;
		bmSuicide = bmResult ;
		while ((pos=bmTest.bitScanForward()) != -1) {
			const WeichiGrid& grid = getGrid(pos);
			if (grid.getColor() != COLOR_NONE) {
				bmTest -= grid.getBlock()->getStoneMap();
				continue;
			}
			WeichiMove move(oppColor, pos);			
			if (getLibertyAfterPlay(move) > 0) { 
				bool bIsEatKo = isKoEatPlay(move);
				bool bOppForbidenKo = (oppColor == COLOR_BLACK && WeichiConfigure::black_ignore_ko) || (oppColor == COLOR_WHITE && WeichiConfigure::white_ignore_ko);
				if (!bIsEatKo || !bOppForbidenKo) { continue; }
			}

			WeichiBitBoard bmDeadStone = getStoneBitBoardAfterPlay(move);
			WeichiBitBoard bmNbrBlock = bmDeadStone.dilate() & StaticBoard::getMaskBorder() & getStoneBitBoard(ownColor);		
			bmSuicide |= bmNbrBlock;
			bmSuicide |= bmDeadStone;
		}		
	}

	return bmSuicide;
}

WeichiBitBoard WeichiBoard::getUpdatedRZoneFromOurTurn(WeichiMove ourMove, WeichiBitBoard bmNextMoveRZone) const
{
	WeichiBitBoard bmResult = bmNextMoveRZone;

	WeichiBitBoard bmInfluence = getInfluenceBlocks(ourMove);
	// find block no liberty in bmNextMoveRZone 
	vector<const WeichiBlock*> vBlocks;
	int pos = 0;
	while ( (pos=bmInfluence.bitScanForward()) != -1) {
		const WeichiGrid& grid = getGrid(pos);
		const WeichiBlock* block = grid.getBlock();
		WeichiBitBoard bmLiberty = block->getLibertyBitBoard(getBitBoard());
		// no intersection
		if (!bmLiberty.hasIntersection(bmNextMoveRZone)) { vBlocks.push_back(block); }

		bmInfluence -= block->getStoneMap();
		if (bmInfluence.empty()) { break; }
	}

	if (vBlocks.size() == 1) {
		const WeichiBlock* block = vBlocks[0];
		WeichiBitBoard bmLiberty = block->getLibertyBitBoard(getBitBoard());
		bmResult.SetBitOn(bmLiberty.bitScanForward());
	} else if (vBlocks.size() > 1) {
		WeichiBitBoard bmCommonLiberty;
		bmCommonLiberty = vBlocks[0]->getLibertyBitBoard(getBitBoard());
		for (int iBlock = 1; iBlock < vBlocks.size(); ++iBlock) {
			WeichiBitBoard bmLiberty = vBlocks[iBlock]->getLibertyBitBoard(getBitBoard());
			bmCommonLiberty &= bmLiberty;
		}

		bmResult |= bmCommonLiberty;

		WeichiBitBoard bmRemaining;
		for (int iBlock = 0; iBlock < vBlocks.size(); ++iBlock) {
			WeichiBitBoard bmLiberty = vBlocks[iBlock]->getLibertyBitBoard(getBitBoard());
			if (bmCommonLiberty.hasIntersection(bmLiberty)) { continue; }
			bmRemaining.SetBitOn(bmLiberty.bitScanForward());
		}

		bmResult |= bmRemaining;
	}

	return bmResult;
}

WeichiBitBoard WeichiBoard::getInfluenceBlocks(WeichiMove move) const
{
	Color ownColor = move.getColor();
	Color oppColor = AgainstColor(ownColor);

	WeichiBitBoard bmResult;
	if (isCaptureMove(move)) {
		const StaticGrid& sgrid = getGrid(move).getStaticGrid();
		WeichiBitBoard bmCheckIndex;
		WeichiBitBoard bmDeadStone;
		const int* nbr = sgrid.getAdjacentNeighbors();
		for (; *nbr != -1; ++nbr) {
			const WeichiGrid& nbrGrid = m_grids[*nbr];
			if (nbrGrid.getColor() != oppColor) continue;

			const WeichiBlock* nbrBlock = nbrGrid.getBlock();
			if (nbrBlock->getLiberty() == 1 && !bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid())) {
				bmDeadStone |= nbrBlock->getStoneMap();
				bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
			}
		}

		WeichiBitBoard bmNbrOwnBlocks = bmDeadStone.dilate() & StaticBoard::getMaskBorder() & getStoneBitBoard(ownColor);		
		int pos = 0;
		while ((pos = bmNbrOwnBlocks.bitScanForward()) != -1) {
			const WeichiGrid& grid = getGrid(pos);
			const WeichiBlock* ownBlock = grid.getBlock();
			bmResult |= ownBlock->getStoneMap();		
		}		
	} else {
		bmResult |= getStoneBitBoardAfterPlay(move);
		bmResult.SetBitOff(move.getPosition());
	}

	return bmResult;
}

WeichiBitBoard WeichiBoard::getInfluenceBitBoard(WeichiMove move) const
{
	Color ownColor = move.getColor();
	Color oppColor = AgainstColor(ownColor);

	WeichiBitBoard bmResult;
	if (isCaptureMove(move)) {
		const StaticGrid& sgrid = getGrid(move).getStaticGrid();
		WeichiBitBoard bmCheckIndex;
		WeichiBitBoard bmDeadStone;
		const int* nbr = sgrid.getAdjacentNeighbors();
		for (; *nbr != -1; ++nbr) {
			const WeichiGrid& nbrGrid = m_grids[*nbr];
			if (nbrGrid.getColor() != oppColor) continue;

			const WeichiBlock* nbrBlock = nbrGrid.getBlock();
			if (nbrBlock->getLiberty() == 1 && !bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid())) {
				bmDeadStone |= nbrBlock->getStoneMap();
				bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
			}
		}

		WeichiBitBoard bmNbrOwnBlocks = bmDeadStone.dilate() & StaticBoard::getMaskBorder() & getStoneBitBoard(ownColor);
		int pos = 0;
		while ((pos = bmNbrOwnBlocks.bitScanForward()) != -1) {
			const WeichiGrid& grid = getGrid(pos);
			const WeichiBlock* ownBlock = grid.getBlock();
			bmResult |= ownBlock->getStoneMap();
		}
		bmResult |= bmDeadStone;
	} else {
		if (move.getPosition() != WeichiMove::PASS_POSITION) {
			bmResult |= getStoneBitBoardAfterPlay(move);
			bmResult.SetBitOff(move.getPosition());
		}
	}

	return bmResult;
}

WeichiBitBoard WeichiBoard::getAndPlayerRZone(WeichiBitBoard bmRZone, Color ownColor) const
{
	WeichiBitBoard bmResult;
	bmResult = getSuicideRZone(bmRZone, ownColor);
	bmResult = getOpponentBorderRZone(bmResult, ownColor);
	bmResult = getLegalizeRZone(bmResult, ownColor);

	return bmResult;
}

WeichiBitBoard WeichiBoard::getLegalizeRZone(WeichiBitBoard bm, Color ownColor) const
{
	WeichiBitBoard bmResult = bm;
	// Legalize R-zone
	int pos = 0;
	while ((pos = bm.bitScanForward()) != -1) {
		const WeichiGrid& grid = getGrid(pos);
		if (grid.getColor() == COLOR_NONE) { continue; }
		const WeichiBlock* block = grid.getBlock();
		if (block->getColor() != ownColor) { continue; }

		bmResult |= block->getStoneMap();
		WeichiBitBoard bmLiberty = block->getLibertyBitBoard(m_bmBoard);
		if ((bmLiberty & bmResult).bitCount() >= 1) { continue; }

		// no z-liberty in RZ
		bmResult.SetBitOn(bmLiberty.bitScanForward());
		bm -= block->getStoneMap();
	}

	return bmResult;
}

WeichiBitBoard WeichiBoard::getSuicideRZone(WeichiBitBoard bm, Color ownColor) const
{	
	Color oppColor = AgainstColor(ownColor);
	WeichiBitBoard bmResult = bm;
	int pos = 0;
	while ((pos = bm.bitScanForward()) != -1) {
		const WeichiGrid& grid = getGrid(pos);
		if (grid.getColor() != COLOR_NONE) {
			bm -= grid.getBlock()->getStoneMap();
			continue;
		}
		WeichiMove move(oppColor, pos);
		if (getLibertyAfterPlay(move) > 0) { 
			bool bIsEatKo = isKoEatPlay(move);
			bool bOppForbiddenKo = (oppColor == COLOR_BLACK && WeichiConfigure::black_ignore_ko)
				|| (oppColor == COLOR_WHITE && WeichiConfigure::white_ignore_ko);
			if (!bIsEatKo || !bOppForbiddenKo) { continue; }
		}

		// Case 1: Suicide Move (Liberty after play = 0)
		// Case 2: Forbidden EatKo Move (Liberty after play = 1)
		WeichiBitBoard bmDeadStone = getStoneBitBoardAfterPlay(move);
		WeichiBitBoard bmNbrBlock = bmDeadStone.dilate() & getStoneBitBoard(ownColor) & StaticBoard::getMaskBorder();

		bmResult |= bmDeadStone;
		int nbrBlockPos = 0;
		while ((nbrBlockPos = bmNbrBlock.bitScanForward()) != -1) {
			const WeichiGrid& nbrBlockGrid = getGrid(nbrBlockPos);
			const WeichiBlock* nbrBlock = nbrBlockGrid.getBlock();
			bmNbrBlock -= nbrBlock->getStoneMap();
			bmResult |= nbrBlock->getStoneMap();
			// add at least one z-liberty for nbrBlock when dead-stone is not considered
			WeichiBitBoard bmLibertyAfterSuicide = nbrBlock->getLibertyBitBoard(m_bmBoard) - bmDeadStone;
			if ((bmLibertyAfterSuicide & bmResult).bitCount() == 0) { bmResult.SetBitOn(bmLibertyAfterSuicide.bitScanForward()); }
		}
	}

	return bmResult;
}

WeichiBitBoard WeichiBoard::getOpponentBorderRZone(WeichiBitBoard bm, Color ownColor) const
{	
	Color oppColor = AgainstColor(ownColor);
	WeichiBitBoard bmPrev = bm;
	WeichiBitBoard bmResult = bm;
	if (WeichiConfigure::use_opponent_border_rzone) {
		int pos = 0;
		while ((pos = bm.bitScanForward()) != -1) {
			const WeichiGrid& grid = getGrid(pos);
			if (grid.getColor() == oppColor) { 
				bm -= grid.getBlock()->getStoneMap();
				continue;
			}

			if (grid.getColor() == COLOR_NONE) {  // the capturing move would change the united zone pattern
				const WeichiMove oppMove(oppColor, pos);
				if (getCapturedStone(oppMove).hasIntersection(bmPrev)) { bmResult |= getStoneBitBoardAfterPlay(oppMove); }
			} else if (grid.getColor() == ownColor) { // if the own block (2-lib above) has 1 liberty in RZ, it might be captured to change the zone pattern 
				const WeichiBlock* ownBlock = grid.getBlock();
				int numLibInRZ = (ownBlock->getLibertyBitBoard(m_bmBoard) & bmPrev).bitCount();
				if (ownBlock->getLiberty() >= 2 && numLibInRZ == 1) {
					WeichiBitBoard bmRemainingLib = ownBlock->getLibertyBitBoard(m_bmBoard) - bmPrev;
					bmResult.SetBitOn(bmRemainingLib.bitScanForward());
				}
			}
		}
	}

	return bmResult;
}

void WeichiBoard::getMoveInfluence(const WeichiMove& move, WeichiBitBoard& bmInfluence, WeichiBitBoard& bmNbrOwn)
{
	// get the changed pattern on the board

	Color ownColor = move.getColor();
	Color oppColor = AgainstColor(ownColor);
	
	WeichiBitBoard bmAllInfluence;
	WeichiBitBoard bmOwnBlock;
	if (isCaptureMove(move)) {
		const WeichiGrid& grid = getGrid(move.getPosition());
		const StaticGrid& sgrid = grid.getStaticGrid();
		const int* nbr = sgrid.getAdjacentNeighbors();
		WeichiBitBoard bmDeadStone;
		for (; *nbr != -1; ++nbr) {
			const WeichiGrid& nbrGrid = getGrid(*nbr);
			if (nbrGrid.getColor() != oppColor) { continue; }
			const WeichiBlock* nbrBlock = nbrGrid.getBlock();
			if (nbrBlock->getLiberty() == 1) { bmDeadStone |= nbrBlock->getStoneMap(); }
		}		

		WeichiBitBoard bmNbrOwnBlocks = bmDeadStone.dilate() & getStoneBitBoard(ownColor) & StaticBoard::getMaskBorder();
		int pos = 0;
		while ((pos = bmNbrOwnBlocks.bitScanForward()) != -1) {
			const WeichiGrid& grid = getGrid(pos);
			const WeichiBlock* ownBlock = grid.getBlock();
			bmNbrOwnBlocks -= ownBlock->getStoneMap();
			bmAllInfluence |= ownBlock->getStoneMap();
			bmOwnBlock |= ownBlock->getStoneMap();
		}
		// note that we must add the stone block itself
		WeichiBitBoard bmStoneAfterPlay = getStoneBitBoardAfterPlay(move);
		bmOwnBlock |= bmStoneAfterPlay;
		bmAllInfluence |= bmDeadStone;
		bmAllInfluence |= bmStoneAfterPlay;
	}
	else {
		if (move.getPosition() != WeichiMove::PASS_POSITION) {
			WeichiBitBoard bmStoneAfterPlay = getStoneBitBoardAfterPlay(move);
			bmAllInfluence |= bmStoneAfterPlay;
			bmOwnBlock |= bmStoneAfterPlay;
		}
	}

	bmInfluence = bmAllInfluence;
	bmNbrOwn = bmOwnBlock;

	return;
}

WeichiBitBoard WeichiBoard::getMoveRZone(WeichiBitBoard bmRZone, WeichiBitBoard bmOwnInfluence) const
{
	WeichiBitBoard bmResult = bmRZone;
	// find block no liberty in bmNextMoveRZone
	vector<const WeichiBlock*> vBlocks;
	int pos = 0;
	while ((pos = bmOwnInfluence.bitScanForward()) != -1) {
		const WeichiGrid& grid = getGrid(pos);
		const WeichiBlock* block = grid.getBlock();
		if (!block) { continue; }
		WeichiBitBoard bmLiberty = block->getLibertyBitBoard(getBitBoard());
		// no intersection
		if (!bmLiberty.hasIntersection(bmRZone)) { vBlocks.push_back(block); }
		bmOwnInfluence -= block->getStoneMap();
		if (bmOwnInfluence.empty()) { break; }
	}

	if (vBlocks.size() == 1) {
		const WeichiBlock* block = vBlocks[0];
		WeichiBitBoard bmLiberty = block->getLibertyBitBoard(getBitBoard());
		bmResult.SetBitOn(bmLiberty.bitScanForward());
	}
	else if (vBlocks.size() > 1) {
		WeichiBitBoard bmCommonLiberty;
		bmCommonLiberty = vBlocks[0]->getLibertyBitBoard(getBitBoard());
		for (int iBlock = 1; iBlock < vBlocks.size(); ++iBlock) {
			WeichiBitBoard bmLiberty = vBlocks[iBlock]->getLibertyBitBoard(getBitBoard());
			bmCommonLiberty &= bmLiberty;
		}

		bmResult |= bmCommonLiberty;

		WeichiBitBoard bmRemaining;
		for (int iBlock = 0; iBlock < vBlocks.size(); ++iBlock) {
			WeichiBitBoard bmLiberty = vBlocks[iBlock]->getLibertyBitBoard(getBitBoard());
			if (bmCommonLiberty.hasIntersection(bmLiberty)) { continue; }
			bmRemaining.SetBitOn(bmLiberty.bitScanForward());
		}

		bmResult |= bmRemaining;
	}

	return bmResult;
}

bool WeichiBoard::isIllegalMove( WeichiMove move, const OpenAddrHashTable& ht ) const
{
	if (move.isPass()) { 
		if (WeichiConfigure::check_pass_superko) {			
			HashKey64 hashkey = m_hash ^ StaticBoard::getHashGenerator().getTurnKey();
			if (ht.lookup(hashkey)) { return true; }
		}
		return false; 
	}
	
	assertToFile( move.getColor()!=COLOR_NONE && move.getColor()!=COLOR_BORDER, this );

	const WeichiGrid& grid = getGrid(move) ;
	if ( !grid.isEmpty() ) return true ;

	if ( m_status.m_ko == move.getPosition() ) return true ;

	const StaticGrid& sgrid = grid.getStaticGrid();
	WeichiBitBoard bmCheckIndex;
	Color color = move.getColor();

	bool checkSuperKo = grid.isPlayed(color) ;
	if ( checkSuperKo ) {
		HashKey64 hashkey = 0 ;
		bool illegal = true ;
		if ( grid.getLiberty() > 0 ) illegal = false ;

		const int* nbr = sgrid.getAdjacentNeighbors();
		for ( ; *nbr != -1 ; ++nbr ) {
			const WeichiGrid& nbrGrid = m_grids[*nbr] ;
			if ( nbrGrid.isEmpty() ) continue ;

			const WeichiBlock* nbrBlock = nbrGrid.getBlock();
			if ( nbrBlock->getColor() == color ) {
				if ( nbrBlock->getLiberty() > 1 ) illegal = false ;
			} else /*if ( nbrBlock->getColor() == AgainstColor(color) ) */ {
				if ( nbrBlock->getLiberty() == 1 && !bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) {
					// capture opponent
					bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
					hashkey ^= nbrBlock->getHash();
					illegal = false ;
				}
			}
		}
		HashKey64 turnKey = StaticBoard::getHashGenerator().getTurnKey();
		return (illegal || ht.lookup(hashkey^m_hash^sgrid.getHash(color)^turnKey)) ; 
	} else {
		if ( grid.getLiberty() > 0 ) return false;

		const int* nbr = sgrid.getAdjacentNeighbors();
		for ( ; *nbr != -1 ; ++nbr ) {
			const WeichiGrid& nbrGrid = m_grids[*nbr] ;
			// if ( nbrGrid.isEmpty() ) return false ; // no such case
			assert(!nbrGrid.isEmpty());

			const WeichiBlock* nbrBlock = nbrGrid.getBlock();
			if ( nbrBlock->getColor() == color ) {
				if ( nbrBlock->getLiberty() > 1 ) return false;
			} else /*if ( nbrBlock->getColor() == AgainstColor(color) ) */ {
				if ( nbrBlock->getLiberty() == 1 && !bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid()) ) {
					// capture opponent
					return false;
				}
			}
		}
		return true;
	}
}

bool WeichiBoard::isTTIllegalMove(const WeichiMove& move, const OpenAddrHashTable& ht) const
{
	const WeichiGrid& grid = getGrid(move);

	if (move.isPass() || !grid.isEmpty() || (getLibertyAfterPlay(move) == 0)) { return false; }
	if (m_status.m_ko == move.getPosition()) { return true; }

	const StaticGrid& sgrid = grid.getStaticGrid();
	WeichiBitBoard bmCheckIndex;
	Color color = move.getColor();
	HashKey64 hashkey = 0;
	const int* nbr = sgrid.getAdjacentNeighbors();
	for (; *nbr != -1; ++nbr) {
		const WeichiGrid& nbrGrid = m_grids[*nbr];
		if (nbrGrid.isEmpty()) continue;

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if (nbrBlock->getColor() == AgainstColor(color)) {
			if (nbrBlock->getLiberty() == 1 && !bmCheckIndex.BitIsOn(nbrBlock->getiFirstGrid())) {
				// capture opponent
				bmCheckIndex.SetBitOn(nbrBlock->getiFirstGrid());
				hashkey ^= nbrBlock->getHash();
			}
		}
	}

	HashKey64 turnKey = StaticBoard::getHashGenerator().getTurnKey();
	return ht.lookup(hashkey^m_hash^sgrid.getHash(color) ^ turnKey);
}

float WeichiBoard::eval (float komi/* = WeichiDynamicKomi::Internal_komi*/) const
{
	if (WeichiConfigure::dcnn_use_ftl) {
		// Life (OR)
		int pos, moves = m_moves.size();
		WeichiBitBoard bmBlackLifeStone = getLADToLifeStones().get(COLOR_BLACK);
		while ((pos = bmBlackLifeStone.bitScanForward()) != -1) {
			const WeichiGrid& grid = getGrid(pos);
			if (grid.isEmpty()) {
				if (grid.getClosedArea(COLOR_BLACK) && grid.getClosedArea(COLOR_BLACK)->getStatus() == LAD_LIFE) { return moves; }
			} else {
				const WeichiBlock* block = grid.getBlock();
				if (block->getColor() == COLOR_BLACK && block->getStatus() == LAD_LIFE) { return moves; }
			}
		}
		WeichiBitBoard bmWhiteLifeStone = getLADToLifeStones().get(COLOR_WHITE);
		while ((pos = bmWhiteLifeStone.bitScanForward()) != -1) {
			const WeichiGrid& grid = getGrid(pos);
			if (grid.isEmpty()) {
				if (grid.getClosedArea(COLOR_WHITE) && grid.getClosedArea(COLOR_WHITE)->getStatus() == LAD_LIFE) { return -moves; }
			} else {
				const WeichiBlock* block = grid.getBlock();
				if (block->getColor() == COLOR_WHITE && block->getStatus() == LAD_LIFE) { return -moves; }
			}
		}

		// Kill (AND)
		WeichiBitBoard bmBlackKill = getLADToKillStones().get(COLOR_BLACK);
		WeichiBitBoard bmWhiteKill = getLADToKillStones().get(COLOR_WHITE);
		if (!bmBlackKill.empty() && (bmBlackKill&getStoneBitBoard(COLOR_WHITE)).empty()) { return moves; }
		if (!bmWhiteKill.empty() && (bmWhiteKill&getStoneBitBoard(COLOR_BLACK)).empty()) { return -moves; }

		return 0;
	} else {
		// Tromp¡VTaylor rule
		Dual<WeichiBitBoard> bmTerritory;
		for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
			const WeichiGrid& grid = getGrid(*it);
			//if (grid.getColor() != COLOR_NONE) { bmTerritory.get(grid.getColor()).SetBitOn(*it); }
			const WeichiClosedArea* blackCA = grid.getClosedArea(COLOR_BLACK);
			const WeichiClosedArea* whiteCA = grid.getClosedArea(COLOR_WHITE);
			if (blackCA && blackCA->getStatus() == LAD_LIFE) { bmTerritory.m_black.SetBitOn(*it); }
			else if (whiteCA && whiteCA->getStatus() == LAD_LIFE) { bmTerritory.m_white.SetBitOn(*it); }
			else if (grid.getColor() != COLOR_NONE) { bmTerritory.get(grid.getColor()).SetBitOn(*it); }
		}

		uint pos;
		WeichiBitBoard bmBorder = ~(bmTerritory.m_black | bmTerritory.m_white) & StaticBoard::getMaskBorder();
		while ((pos = bmBorder.bitScanForward()) != -1) {
			bmBorder.SetBitOn(pos);

			WeichiBitBoard bmFloodFill;
			bmBorder.floodfill(pos, bmFloodFill);
			WeichiBitBoard bmDilate = bmFloodFill.dilate() - bmFloodFill;
			bool bHasReachBlack = !(bmDilate&bmTerritory.m_black).empty();
			bool bHasReachWhite = !(bmDilate&bmTerritory.m_white).empty();
			if (bHasReachBlack && !bHasReachWhite) { bmTerritory.m_black |= bmFloodFill; }
			else if (!bHasReachBlack && bHasReachWhite) { bmTerritory.m_white |= bmFloodFill; }

			bmBorder -= bmFloodFill;
		}

		// add territory
		m_territory.clear();
		m_territory.resize(MAX_NUM_GRIDS);
		for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
			if (bmTerritory.m_black.BitIsOn(*it)) { m_territory[*it] = COLOR_BLACK; }
			else if (bmTerritory.m_white.BitIsOn(*it)) { m_territory[*it] = COLOR_WHITE; }
			else { m_territory[*it] = COLOR_NONE; }
		}

		float black = 0.0 + bmTerritory.m_black.bitCount();
		float white = komi + bmTerritory.m_white.bitCount();
		return black - white;
	}
}

std::string WeichiBoard::toString ( ) const
{
	Vector<uint,MAX_NUM_GRIDS> vCandidates ;
	uint size = m_candidates.getNumCandidate();

	const uint* cands = m_candidates.getCandidates();
	for( uint i=0; i<size; ++i ) {
		vCandidates.push_back(cands[i]);
	}

	return toString(vCandidates);
}

std::string WeichiBoard::toString( const Vector<uint,MAX_NUM_GRIDS>& vSpecial ) const
{
	std::set<uint> setSpecial ;
	for ( uint i=0;i<vSpecial.size();++i ) {
		setSpecial.insert(vSpecial[i]) ;
	}

	ostringstream oss ;
	oss << endl;
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		if( *it%MAX_BOARD_SIZE==0 ) { oss << std::setw(2) << (*it/MAX_BOARD_SIZE+1) << ' '; }

		const WeichiGrid& grid = getGrid(*it);
		if( grid.isEmpty() && setSpecial.count(*it) ) { oss << " *"; }
		else { oss << toGuiString(grid.getColor()); }

		if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }
	}

	oss << "   ";
	int coorBoardSize = (WeichiConfigure::BoardSize>=9) ? WeichiConfigure::BoardSize : (WeichiConfigure::BoardSize-1);
	for ( int x=0 ; x<=coorBoardSize ; ++x ) {
		if( 'A'+x=='I' )	x++;
		oss << ' ' << char('A'+x);
	}
	return oss.str();
}

void WeichiBoard::showColorBoard() const
{
	CERR() << toColorBoardString();
}

string WeichiBoard::toColorBoardString() const
{
	ostringstream oss;

	oss << toBoardCoordinateString(true) << endl;
	for (int rowNumber = WeichiConfigure::BoardSize; rowNumber > 0; rowNumber--) {
		oss << toOneRowBoardString(rowNumber, true) << endl;
	}
	oss << toBoardCoordinateString(true) << endl;
	oss << toChar(getToPlay()) << " to play" << endl;

	return oss.str();
}

string WeichiBoard::toBoardCoordinateString( bool bShowWithColor/*=false*/ ) const
{
	ostringstream oss;
	int boardSize = (WeichiConfigure::BoardSize>=9) ? WeichiConfigure::BoardSize : (WeichiConfigure::BoardSize-1);

	if( !bShowWithColor ) {
		oss << "  ";
		for( int x=0; x<=boardSize; ++x ) {
			if( 'A'+x=='I' ) { x++; }
			oss << ' ' << char('A'+x);
		}
		oss << "   ";
	} else {
#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WINNT)
		HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO *ConsoleInfo = new CONSOLE_SCREEN_BUFFER_INFO();
		GetConsoleScreenBufferInfo(hConsole, ConsoleInfo);
		WORD OriginalColors = ConsoleInfo->wAttributes;
		const int BLACK = 96,WHITE = 111,DEFAULT = 103;

		SetConsoleTextAttribute(hConsole,DEFAULT);
		oss << "  ";
		for ( int x=0; x<=boardSize ; ++x ) {
			if( 'A'+x=='I' )	x++;
			oss << ' ' << char('A'+x);
		}
		oss << "   ";
		SetConsoleTextAttribute(hConsole,OriginalColors);
#else
		oss << getColorMessage("  ",ANSITYPE_BOLD,ANSICOLOR_BLACK,ANSICOLOR_YELLOW);
		for ( int x=0; x<=boardSize ; ++x ) {
			if( 'A'+x=='I' ) { x++; }
			oss << getColorMessage(" "+ToString(char('A'+x)),ANSITYPE_BOLD,ANSICOLOR_BLACK,ANSICOLOR_YELLOW);
		}
		oss << getColorMessage("  ",ANSITYPE_BOLD,ANSICOLOR_BLACK,ANSICOLOR_YELLOW);
#endif
	}

	return oss.str();
}

string WeichiBoard::toOneRowBoardString( int rowNumber, Vector<uint,MAX_NUM_GRIDS> vNumberSequecne, Color numberColor, bool bChangeColorWithEvenNumber, bool bShowWithColor/*=false*/ ) const
{
	ostringstream oss;
	if( !bShowWithColor ) {
		oss << std::setw(2) << rowNumber;
		for( uint x=0; x<WeichiConfigure::BoardSize; x++ ) {
			int position = (rowNumber-1)*MAX_BOARD_SIZE + x;
			const WeichiGrid& grid = getGrid(position);

			if( grid.isEmpty() && m_candidates.contains(position) ) { oss << " *"; }
			else { oss << toGuiString(grid.getColor()); }
		}
		oss << ' ' << std::setw(2) << rowNumber;
	} else {
#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WINNT)
		HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO *ConsoleInfo = new CONSOLE_SCREEN_BUFFER_INFO();
		GetConsoleScreenBufferInfo(hConsole, ConsoleInfo);
		WORD OriginalColors = ConsoleInfo->wAttributes;
		const int BLACK = 96,WHITE = 111,DEFAULT = 103;

		SetConsoleTextAttribute(hConsole,DEFAULT);
		oss << std::setw(2) << rowNumber;
		for( uint x=0; x<WeichiConfigure::BoardSize; x++ ) {
			int position = (rowNumber-1)*MAX_BOARD_SIZE + x;
			const WeichiGrid& grid = getGrid(position);
			
			SetConsoleTextAttribute(hConsole,DEFAULT);
			if( grid.isBlack() ) { SetConsoleTextAttribute(hConsole,BLACK); oss << "¡´"; }
			else if( grid.isWhite() ) { SetConsoleTextAttribute(hConsole,WHITE); oss << "¡³"; }
			else if( m_candidates.contains(position) ) { oss << " *"; }
			else { oss << " ."; }
		}
		oss << ' ' << std::setw(2) << rowNumber;
		SetConsoleTextAttribute(hConsole,OriginalColors);
#else
		ostringstream ossTemp;
		ossTemp << std::setw(2) << rowNumber;
		string sRowNumber = getColorMessage(ossTemp.str(),ANSITYPE_BOLD,ANSICOLOR_BLACK,ANSICOLOR_YELLOW);
		oss << sRowNumber;
		for( uint x=0; x<WeichiConfigure::BoardSize; x++ ) {
			int position = (rowNumber-1)*MAX_BOARD_SIZE + x;
			const WeichiGrid& grid = getGrid(position);

			if( vNumberSequecne[position]!=-1 && vNumberSequecne[position]<=99 ) {
				ossTemp.str("");
				ossTemp << setw(2) << vNumberSequecne[position];
				Color displayColor = numberColor;
				if( bChangeColorWithEvenNumber ) { displayColor = (vNumberSequecne[position]%2==1)? numberColor: AgainstColor(numberColor); }
				if( displayColor==COLOR_BLACK ) { oss << getColorMessage(ossTemp.str(),ANSITYPE_NORMAL,ANSICOLOR_BLACK,ANSICOLOR_YELLOW); }
				else { oss << getColorMessage(ossTemp.str(),ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_YELLOW); }
			} else {
				if( grid.isEmpty() ) { oss << getColorMessage(" .",ANSITYPE_BOLD,ANSICOLOR_BLACK,ANSICOLOR_YELLOW); }
				else {
					if( hasPrevMove() && getPrevMove().getPosition()==position ) { oss << getColorMessage("(",ANSITYPE_BOLD,ANSICOLOR_RED,ANSICOLOR_YELLOW); }
					else if( hasPrevOwnMove() && getPrevOwnMove().getPosition()==position ) { oss << getColorMessage("(",ANSITYPE_NORMAL,ANSICOLOR_RED,ANSICOLOR_YELLOW); }
					else { oss << getColorMessage(" ",ANSITYPE_NORMAL,ANSICOLOR_BLACK,ANSICOLOR_YELLOW); }

					if( grid.getColor()==COLOR_BLACK ) { oss << getColorMessage("O",ANSITYPE_NORMAL,ANSICOLOR_BLACK,ANSICOLOR_YELLOW); }
					else { oss << getColorMessage("O",ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_YELLOW); }
				}
			}
		}
		oss << sRowNumber;
#endif
	}

	return oss.str();
}

string WeichiBoard::toOneRowTerritoryString( int rowNumber, bool bShowWithColor/*=false*/, Color onlyShowColor/*=COLOR_NONE*/ ) const
{
	ostringstream oss;
	const Territory& territory = WeichiGlobalInfo::getSearchInfo().m_territory;
	
	if( !bShowWithColor ) {
		oss << std::setw(2) << rowNumber;
		for( uint x=0; x<WeichiConfigure::BoardSize; x++ ) {
			int position = (rowNumber-1)*MAX_BOARD_SIZE + x;
			const double dTerritory = territory.getTerritory(position);
			const double dCriticality = territory.getCriticality(position);

			if( dCriticality>WeichiConfigure::CriticalityThreshold ) { oss << " !"; }
			else if( dTerritory>0 ) {
				if( onlyShowColor==COLOR_BLACK ) { oss << " ?"; }
				else if( dTerritory>WeichiConfigure::SureTerrityThreshold ) { oss << " X"; }
				else if( dTerritory>WeichiConfigure::PossibleTerrityThreshold ) { oss << " x"; }
				else { oss << " ."; }
			} else {
				if( onlyShowColor==COLOR_WHITE ) { oss << " ?"; }
				else if( dTerritory<-WeichiConfigure::SureTerrityThreshold ) { oss << " X"; }
				else if( dTerritory<-WeichiConfigure::PossibleTerrityThreshold ) { oss << " x"; }
				else { oss << " ."; }
			}
		}
		oss << ' ' << std::setw(2) << rowNumber;
	} else {
#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WINNT)
		HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO *ConsoleInfo = new CONSOLE_SCREEN_BUFFER_INFO();
		GetConsoleScreenBufferInfo(hConsole, ConsoleInfo);
		WORD OriginalColors = ConsoleInfo->wAttributes;
		const int BLACK = 96,WHITE = 111,DEFAULT = 103,RED = 108;

		SetConsoleTextAttribute(hConsole,DEFAULT);
		oss << std::setw(2) << rowNumber;
		for( uint x=0; x<WeichiConfigure::BoardSize; x++ ) {
			int position = (rowNumber-1)*MAX_BOARD_SIZE + x;
			const double dTerritory = territory.getTerritory(position);
			const double dCriticality = territory.getCriticality(position);

			SetConsoleTextAttribute(hConsole,DEFAULT);
			if( dCriticality>WeichiConfigure::CriticalityThreshold ) {
				SetConsoleTextAttribute(hConsole,RED);
				oss << "¡I";
			} else if( dTerritory>0 ) {
				if( dTerritory>WeichiConfigure::SureTerrityThreshold ) { SetConsoleTextAttribute(hConsole,BLACK); oss << "¢æ"; }
				else if( dTerritory>WeichiConfigure::PossibleTerrityThreshold ) { SetConsoleTextAttribute(hConsole,BLACK); oss << "£A"; }
				else { oss << " ."; }
			} else {
				if( dTerritory<-WeichiConfigure::SureTerrityThreshold ) { SetConsoleTextAttribute(hConsole,WHITE); oss << "¢æ"; }
				else if( dTerritory<-WeichiConfigure::PossibleTerrityThreshold ) { SetConsoleTextAttribute(hConsole,WHITE); oss << "£A"; }
				else { oss << " ."; }
			}
		}
		oss << ' ' << std::setw(2) << rowNumber;
		SetConsoleTextAttribute(hConsole,OriginalColors);
	#else
		const string sCirticality = "\33[0;31;43m !\33[0m";
		const string sSureBlack = "\33[0;30;43m X\33[0m";
		const string sPossibleBlack = "\33[0;30;43m x\33[0m";
		const string sSureWhite = "\33[1;37;43m X\33[0m";
		const string sPossibleWhite = "\33[1;37;43m x\33[0m";

		ostringstream ossTemp;
		ossTemp << std::setw(2) << rowNumber;
		string sRowNumber = getColorMessage(ossTemp.str(),ANSITYPE_BOLD,ANSICOLOR_BLACK,ANSICOLOR_YELLOW);
		oss << sRowNumber;
		for( uint x=0; x<WeichiConfigure::BoardSize; x++ ) {
			int position = (rowNumber-1)*MAX_BOARD_SIZE + x;
			const double dTerritory = territory.getTerritory(position);
			const double dCriticality = territory.getCriticality(position);

			// criticality (background)
			ANSICOLOR background = ANSICOLOR_YELLOW;
			if( dCriticality>WeichiConfigure::CriticalityThreshold ) { background = ANSICOLOR_RED; }

			// territory
			if( dTerritory>0 ) {
				if( dTerritory>WeichiConfigure::SureTerrityThreshold ) { oss << getColorMessage(" X",ANSITYPE_NORMAL,ANSICOLOR_BLACK,background); }
				else if( dTerritory>WeichiConfigure::PossibleTerrityThreshold ) { oss << getColorMessage(" x",ANSITYPE_NORMAL,ANSICOLOR_BLACK,background); }
				else { oss << getColorMessage(" .",ANSITYPE_NORMAL,ANSICOLOR_BLACK,background); }
			} else if( dTerritory<0 ) {
				if( dTerritory<-WeichiConfigure::SureTerrityThreshold ) { oss << getColorMessage(" X",ANSITYPE_BOLD,ANSICOLOR_WHITE,background); }
				else if( dTerritory<-WeichiConfigure::PossibleTerrityThreshold ) { oss << getColorMessage(" x",ANSITYPE_BOLD,ANSICOLOR_WHITE,background); }
				else { oss << getColorMessage(" .",ANSITYPE_BOLD,ANSICOLOR_WHITE,background); }
			} else {
				assertToFile( dTerritory==0, this );
				oss << getColorMessage(" .",ANSITYPE_BOLD,ANSICOLOR_BLACK,background);
			}
		}
		oss << sRowNumber;
#endif
	}

	return oss.str();
}

HashKey64 WeichiBoard::getMinimumHash() const
{
	HashKey64 minBoardHash(ULLONG_MAX);
	for (int i=0 ; i<8 ; i++) {
		HashKey64 tmpKey;
		for (StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it) {			
			const WeichiGrid& grid = getGrid(*it);
			if (grid.isEmpty()) { continue; }
			
			int boardSize = WeichiConfigure::BoardSize;
			Color color = grid.getColor();
			int x = grid.getStaticGrid().x();
			int y = grid.getStaticGrid().y();
			
			int tmp = 0;
			switch(i) {
				case 0: x = x; y = y  ; break;
				case 1: tmp = x; x = y; y = tmp; break;
				case 2: tmp = x; x = y; y = boardSize - tmp; break;
				case 3:	x = x; y = boardSize - y; break;
				case 4:	x = boardSize - x; y=boardSize-y ; break;
				case 5:	tmp = x; x = boardSize - y; y = boardSize - x; break;
				case 6:	tmp = x; x = boardSize - y; y=tmp ; break;
				case 7:	x = boardSize - y; y=y ; break;
			}
			const WeichiGrid& transGrid = getGrid(x,y);
			tmpKey ^= transGrid.getStaticGrid().getHash(color);
		}
		if (tmpKey < minBoardHash) { minBoardHash = tmpKey; }
	}

	cerr << minBoardHash << endl;

	return minBoardHash;
}

void WeichiBoard::updateCandidateRange( uint pos )
{
	const StaticGrid& sgrid = getGrid(pos).getStaticGrid() ;
	uint size = sgrid.getNumCandidate();
	const uint * candidates = sgrid.getCandidates() ;

	m_candidates.removeCandidate(pos) ;

	for ( uint i=0; i<size; ++i ) {
		uint candidate = candidates[i] ;
		if ( !m_grids[candidate].isEmpty() ) { continue; }

		m_candidates.addCandidate(candidate) ;
	}
}

void WeichiBoard::undoClosedArea(WeichiGrid& grid, BoardMoveStack& moveStack)
{

}

void WeichiBoard::undoBlockModifyNumber(WeichiGrid& grid)
{

}

void WeichiBoard::undoCandidateRange(uint pos)
{
	// To Do: The logic?
}

void WeichiBoard::updatePatternLibIndex( WeichiGrid& grid )
{
	uint iNbrLibIndex = 0;

	for( int iNbr=0; iNbr<RADIUS_NBR_LIB_SIZE; iNbr++ ) {
		int dir = WeichiRadiusPatternTable::vRadiusNbrLibOrder[iNbr];
		if( grid.getStaticGrid().getNeighbor(dir)==-1 ) { continue; }

		const WeichiGrid& nbrGrid = getGrid(grid,dir);
		if( nbrGrid.isEmpty() ) { continue; }

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if( nbrBlock->getLiberty()>2 ) { continue; }
		else if( nbrBlock->getLiberty()==1 ) { iNbrLibIndex |= (1<<(dir*RADIUS_NBR_LIB_BITS)); }
		else if( nbrBlock->getLiberty()==2 ) { iNbrLibIndex |= (2<<(dir*RADIUS_NBR_LIB_BITS)); }

		assertToFile( dir>=0 && dir<4, this );
	}

	grid.setNbrLibIndex(iNbrLibIndex);
	if( WeichiConfigure::use_probability_playout ) { m_probabilityHandler.setUpdatePattern(grid.getPosition()); }
}

void WeichiBoard::updateNeighborPatternIndex( const WeichiGrid& grid, Color color )
{
	const int * nbr33 = grid.getStaticGrid().get3x3Neighbors();
	const int * rel33 = grid.getStaticGrid().get3x3Relations();

	const int* indexDiff = grid.getStaticGrid().getPatternIndexDifference(color);
	for( ; *nbr33!=-1 ; nbr33++, rel33++ ) {
		WeichiGrid& nbrGrid = getGrid(*nbr33) ;
		nbrGrid.addPatternOffset(indexDiff[*rel33]) ;
	}
}

void WeichiBoard::updateDeadGridPatternIndex( const WeichiGrid& grid, Color color )
{
	const int * nbr33 = grid.getStaticGrid().get3x3Neighbors();
	const int * rel33 = grid.getStaticGrid().get3x3Relations();

	const int* indexDiff = grid.getStaticGrid().getPatternIndexDifference(color);
	for( ; *nbr33!=-1 ; nbr33++, rel33++ ) {
		WeichiGrid& nbrGrid = getGrid(*nbr33) ;
		nbrGrid.subPatternOffset(indexDiff[*rel33]) ;
	}
}

void WeichiBoard::updateRadiusPatternIndex( const WeichiGrid& grid, Color color )
{
	if( isPlayLightStatus() ) { return; }

	const StaticGrid::RadiusGridRelation *radiusGridRelation = grid.getStaticGrid().getRadiusGridRelations();

	for( ; radiusGridRelation->m_iRadius!=-1; radiusGridRelation++ ) {
		if( isPlayoutStatus() && radiusGridRelation->m_iRadius>3 ) { break; }

		WeichiGrid& radiusGrid = getGrid(radiusGridRelation->m_neighborPos);
		uint iRadius = radiusGridRelation->m_iRadius;
		uint offset = WeichiRadiusPatternTable::getRadiusPatternIndexOffset(radiusGridRelation->m_relativeIndex,color);

		radiusGrid.addPatternOffset(iRadius,offset);
		if( WeichiConfigure::use_probability_playout ) { m_probabilityHandler.setUpdatePattern(radiusGrid.getPosition()); }
	}
}

void WeichiBoard::updateDeadRadiusPatternIndex( const WeichiGrid& grid, Color color )
{
	if( isPlayLightStatus() ) { return; }

	const StaticGrid::RadiusGridRelation *radiusGridRelation = grid.getStaticGrid().getRadiusGridRelations();

	for( ; radiusGridRelation->m_iRadius!=-1; radiusGridRelation++ ) {
		if( isPlayoutStatus() && radiusGridRelation->m_iRadius>3 ) { break; }

		WeichiGrid& radiusGrid = getGrid(radiusGridRelation->m_neighborPos);
		uint iRadius = radiusGridRelation->m_iRadius;
		uint offset = WeichiRadiusPatternTable::getRadiusPatternIndexOffset(radiusGridRelation->m_relativeIndex,color);

		radiusGrid.subPatternOffset(iRadius,offset);
		if( WeichiConfigure::use_probability_playout ) { m_probabilityHandler.setUpdatePattern(radiusGrid.getPosition()); }
	}

	if( WeichiConfigure::use_probability_playout ) { m_probabilityHandler.setUpdatePattern(grid.getPosition()); }
}

void WeichiBoard::removeLibertyBlock( WeichiBlock* block, uint liberty )
{
	if ( liberty==1 ) { m_bmOneLibBlocks.get(block->getColor()) -= block->getStoneMap(); }
	else if ( liberty==2 ) { m_bmTwoLibBlocks.get(block->getColor()) -= block->getStoneMap(); }
}

void WeichiBoard::addLibertyBlock( WeichiBlock* block, uint liberty )
{
	if( liberty==1 ) { m_bmOneLibBlocks.get(block->getColor()) |= block->getStoneMap(); }
	else if( liberty==2 ) { m_bmTwoLibBlocks.get(block->getColor()) |= block->getStoneMap(); }
}

void WeichiBoard::addLibertyStone( WeichiBlock* block, uint stonePos, uint org_lib )
{
	uint new_lib = block->getLiberty();
	if( org_lib!=new_lib ) {
		addAndRemoveLibertyBitBoard(block,org_lib);
		return;
	}

	if( new_lib==1 ) { m_bmOneLibBlocks.get(block->getColor()).SetBitOn(stonePos); }
	else if( new_lib==2 ) { m_bmTwoLibBlocks.get(block->getColor()).SetBitOn(stonePos); }
}

void WeichiBoard::removeLibertyStone( WeichiBlock* block, uint stonePos, uint org_lib )
{
	uint new_lib = block->getLiberty();
	if( org_lib!=new_lib ) {
		addAndRemoveLibertyBitBoard(block,org_lib);
		return;
	}

	if( new_lib==1 ) { m_bmOneLibBlocks.get(block->getColor()).SetBitOff(stonePos); }
	else if( new_lib==2 ) { m_bmTwoLibBlocks.get(block->getColor()).SetBitOff(stonePos); }
}

void WeichiBoard::addAndRemoveLibertyBitBoard( WeichiBlock* block, uint org_lib )
{
	addLibertyBlock(block,block->getLiberty());
	if( org_lib!=block->getLiberty() ) { removeLibertyBlock(block,org_lib); }
}

uint WeichiBoard::calculateRadiusLibertyPatternIndex( const WeichiGrid& center_grid ) const
{
	uint iIndex = 0;
	const StaticGrid& sgrid = center_grid.getStaticGrid();

	if( sgrid.getNeighbor(UPPER_IDX)!=-1 ) { iIndex += (getOneRadiusLibertyIndex(getGrid(center_grid,UPPER_IDX))); }
	if( sgrid.getNeighbor(RIGHT_IDX)!=-1 ) { iIndex += (getOneRadiusLibertyIndex(getGrid(center_grid,RIGHT_IDX)) << 1); }
	if( sgrid.getNeighbor(DOWN_IDX)!=-1 ) { iIndex += (getOneRadiusLibertyIndex(getGrid(center_grid,DOWN_IDX)) << 2); }
	if( sgrid.getNeighbor(LEFT_IDX)!=-1 ) { iIndex += (getOneRadiusLibertyIndex(getGrid(center_grid,LEFT_IDX)) << 3); }
	if( sgrid.getNeighbor(LEFT_UPPER_IDX)!=-1 ) { iIndex += (getOneRadiusLibertyIndex(getGrid(center_grid,LEFT_UPPER_IDX)) << 4); }
	if( sgrid.getNeighbor(LEFT_DOWN_IDX)!=-1 ) { iIndex += (getOneRadiusLibertyIndex(getGrid(center_grid,LEFT_DOWN_IDX)) << 5); }
	if( sgrid.getNeighbor(RIGHT_DOWN_IDX)!=-1 ) { iIndex += (getOneRadiusLibertyIndex(getGrid(center_grid,RIGHT_DOWN_IDX)) << 6); }
	if( sgrid.getNeighbor(RIGHT_UPPER_IDX)!=-1 ) { iIndex += (getOneRadiusLibertyIndex(getGrid(center_grid,RIGHT_UPPER_IDX)) << 7); }

	return iIndex;
}

uint WeichiBoard::getOneRadiusLibertyIndex( const WeichiGrid& grid ) const
{
	if( grid.isEmpty() ) { return 0; }

	const WeichiBlock* block = grid.getBlock();
	if( block->getLiberty()==1 ) { return 1; }
	
	return 0;
}

std::string WeichiBoard::getBoardStatus() const
{
	ostringstream oss;
    
	// blocks
	oss << "Block[" << setw(2) << m_blockList.getSize() << "]:" << endl;
	for( uint i=0; i<m_blockList.getCapacity(); i++ ) {
		if( !m_blockList.isValidIdx(i) ) { continue; }

		const WeichiBlock* block = m_blockList.getAt(i);
		if( !block->isUsed() ) { continue; }

		oss << "\t" << block->getBlockStatusString() << endl;
	}

	return oss.str();
}

std::string WeichiBoard::getEdgeDrawingString() const
{
	uint pos;
	ostringstream oss;
	WeichiBitBoard bmStone = m_bmBoard;
	while( (pos=bmStone.bitScanForward())!=-1 ) {
		const WeichiGrid& grid = getGrid(pos);
		if( grid.isEmpty() || !grid.hasEdge() ) { continue; }

		oss << grid.getEdgeLinesString();
	}

	return oss.str();
}

string WeichiBoard::getMemberSizeString()
{
	ostringstream oss;

	oss << setiosflags(ios::left) << setw(25) << "Total size:" << sizeof(*this) << endl;
	oss << setiosflags(ios::left) << setw(25) << "status:" << sizeof(BoardStatus) << endl;
	oss << setiosflags(ios::left) << setw(25) << "hash key:" << sizeof(m_hash) << endl;
	oss << setiosflags(ios::left) << setw(25) << "board bitBoard:" << sizeof(m_bmBoard) << endl;
	oss << setiosflags(ios::left) << setw(25) << "board dead bitboard:" << sizeof(m_bmDeadStones) << endl;
	oss << setiosflags(ios::left) << setw(25) << "color bitboard:" << sizeof(m_bmStone) << endl;
	oss << setiosflags(ios::left) << setw(25) << "bounding box:" << sizeof(m_boundingBox) << endl;
	oss << setiosflags(ios::left) << setw(25) << "preset:" << sizeof(m_preset) << endl;
	oss << setiosflags(ios::left) << setw(25) << "move stack:" << sizeof(m_moveStacks) << endl;
	oss << setiosflags(ios::left) << setw(25) << "territory:" << sizeof(m_territory) << endl;
	oss << setiosflags(ios::left) << setw(25) << "candidate list:" << sizeof(m_candidates) << endl;
	oss << setiosflags(ios::left) << setw(25) << "grids:" << sizeof(m_grids) << endl;
	oss << setiosflags(ios::left) << setw(25) << "block list:" << sizeof(m_blockList) << endl;
#if DO_WINDOWS_TIMER
	oss << setiosflags(ios::left) << setw(25) << "microsecond timer:" << sizeof(m_microSecondTimer) << endl;
#endif
	oss << endl;

	return oss.str();
}

std::string WeichiBoard::toSgfFilePrefix() const
{
	return "(;FF[4]CA[UTF-8]";
}

std::string WeichiBoard::toSgfFileString(SgfTag sgfTag/* = SgfTag()*/, vector<string> vMoveComments/* = vector<string>()*/) const
{
	ostringstream oss;
	oss << toSgfFilePrefix() << sgfTag.getSgfTagString() << toMoveString(true, vMoveComments) << ")";

	return oss.str();
}

std::string WeichiBoard::toMoveString(bool with_semicolon/* = false*/, vector<string> vMoveComments/* = vector<string>()*/) const
{
	ostringstream oss;

	string black, white;
	for (uint i = 0; i < m_preset.size(); ++i) {
		if (m_preset[i].getColor() == COLOR_BLACK)
			black += "[" + m_preset[i].toSgfString(false) + "]";
		else if (m_preset[i].getColor() == COLOR_WHITE)
			white += "[" + m_preset[i].toSgfString(false) + "]";
	}
	if (!black.empty() || !white.empty()) {
		if (with_semicolon) oss << ";";
		if (!black.empty()) oss << "AB" << black;
		if (!white.empty()) oss << "AW" << white;
	}

	for (uint i = 0; i < m_moves.size(); ++i) {
		if (with_semicolon) oss << ";";
		oss << m_moves[i].toSgfString();
		if (i < vMoveComments.size() && vMoveComments[i] != "") { oss << "C[" << vMoveComments[i] << "]"; }
	}
	return oss.str();
}

bool WeichiBoard::isAtariMove( WeichiMove move ) const 
{	
	if (move.getPosition()==WeichiMove::PASS_POSITION) { return false; }
	if( getLibertyAfterPlay(move) == 0 ) { return false ; } // forbidden move

	Color ownColor = move.getColor() ;
	Color oppColor = AgainstColor(move.getColor()) ;

	const WeichiGrid& grid = getGrid(move);
	const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors();
	for( ; *iNbr!=-1; iNbr++ ) {
		const WeichiBlock* nbrBlk = getGrid(*iNbr).getBlock() ;
		if( nbrBlk == NULL ) continue ;
		if( nbrBlk->getColor() == ownColor ) continue ;
		if( nbrBlk->getLiberty() == 2 ) { return true ; }
	}

	return false ;
}

bool WeichiBoard::isCaptureMove( WeichiMove move ) const 
{	
	if (move.getPosition()==WeichiMove::PASS_POSITION) { return false; }
	if (getLibertyAfterPlay(move) == 0) { return false ; } // forbidden move

	Color ownColor = move.getColor() ;
	Color oppColor = AgainstColor(move.getColor()) ;

	const WeichiGrid& grid = getGrid(move);
	const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors();
	for( ; *iNbr!=-1; iNbr++ ) {
		const WeichiBlock* nbrBlk = getGrid(*iNbr).getBlock() ;
		if( nbrBlk == NULL ) continue ;
		if( nbrBlk->getColor() == ownColor ) continue ;
		if( nbrBlk->getLiberty() == 1 ) { return true ; }
	}

	return false ;
}

bool WeichiBoard::is2libMoveImmediateMoveForbidden( const WeichiMove& move ) const 
{
	if (getLibertyAfterPlay(move) != 2) { return false; }

	Color escapeColor = move.getColor();
	Color blockingColor = AgainstColor(escapeColor);

	WeichiBitBoard bm2Lib = getLibertyBitBoardAfterPlay(move);
	Vector<uint, 2> v2Libs ; bm2Lib.bitScanAll(v2Libs);	
	bool bHasBlockingMove = false ;
	for( uint iLib = 0 ; iLib < v2Libs.size() ; ++iLib )  {
		WeichiMove blockingMove(blockingColor, v2Libs[iLib]) ;
		WeichiBitBoard bmlibAfterBlocking = getLibertyBitBoardAfterPlay(blockingMove) ;
		bmlibAfterBlocking.SetBitOff(move.getPosition()) ;
		if( bmlibAfterBlocking.bitCount() >=2 ) {
			WeichiMove anotherMove(escapeColor, v2Libs[1-iLib]) ;
			if( getLibertyAfterPlay(anotherMove) <= 1 ) {
				bHasBlockingMove = true ;
				break ;
			}
		}
	}

	return bHasBlockingMove ;
}

WeichiBitBoard WeichiBoard::getCapturedStone(const WeichiMove move) const
{
	if (move.getPosition() == WeichiMove::PASS_POSITION) { return WeichiBitBoard(); }
	if (getLibertyAfterPlay(move) == 0) { return WeichiBitBoard(); } // forbidden move

	Color ownColor = move.getColor();
	Color oppColor = AgainstColor(move.getColor());

	WeichiBitBoard bmCaptured;
	const WeichiGrid& grid = getGrid(move);
	const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors();
	for (; *iNbr != -1; iNbr++) {
		const WeichiBlock* nbrBlk = getGrid(*iNbr).getBlock();
		if (nbrBlk == NULL) continue;
		if (nbrBlk->getColor() == ownColor) continue;
		if (nbrBlk->getLiberty() == 1) { bmCaptured |= nbrBlk->getStoneMap(); }
	}

	return bmCaptured;
}

bool WeichiBoard::isKoEatPlay( WeichiMove move ) const
{
	if( move.getPosition() == WeichiMove::PASS_POSITION ) { return false; }

	const WeichiGrid& grid = getGrid(move);
	Color oppColor = AgainstColor(move.getColor());
	if( !grid.getPattern().getFalseEye(oppColor) ) { return false; }

	bool bIsKoPlay = false;
	const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors();
	for( ; *iNbr!=-1; iNbr++ ) {
		const WeichiBlock* nbrBlk = getGrid(*iNbr).getBlock();

		assertToFile( nbrBlk->getColor()==oppColor, this );

		if( nbrBlk->getLiberty()>1 ) { continue; }
		if( nbrBlk->getNumStone()>1 ) { return false; }

		// lib==1 && num stone==1
		if( bIsKoPlay ) { return false; }
		else { bIsKoPlay = true; }
	}

	return bIsKoPlay;
}

bool WeichiBoard::isKoMakePlay( WeichiMove move ) const
{
	WeichiBitBoard bmNewLib;
	uint newLib = getLibertyBitBoardAndLibertyAfterPlay(move,bmNewLib);
	if( newLib!=1 ) { return false; }

	uint ko_pos = bmNewLib.bitScanForward();
	const WeichiGrid& center_grid = getGrid(ko_pos);
	if( !center_grid.getPattern().getMakeKo(move.getColor()) ) { return false; }

	for( const int* iNbr=center_grid.getStaticGrid().getAdjacentNeighbors(); *iNbr!=-1; iNbr++ ) {
		const WeichiGrid& nbrGrid = getGrid(*iNbr);
		if( nbrGrid.isEmpty() ) { continue; }

		const WeichiBlock* nbrBlock = nbrGrid.getBlock();
		if( nbrBlock->getLiberty()==1 ) { return false; }
	}

	return true;
}

WeichiBitBoard WeichiBoard::getAliveArea(Color color) const {	
	return getBensonBitboard(color) | getDeadArea(color);
}

WeichiBitBoard WeichiBoard::getDeadArea(Color color) const {
	if (!WeichiConfigure::use_independent_territory_detection) { return WeichiBitBoard(); }

	WeichiBitBoard bmLifeZone = getBensonBitboard(color); // Pass-live area.
	WeichiBitBoard bmDeadArea; // Set true if the point is in pass-dead area.
	WeichiBitBoard bmBorder; // Set true if the point is at side or corner.
	WeichiBitBoard bmEmptys; // Set true if the point is not in pass-alive area.
	WeichiBitBoard bmInnerRegions; // Set true if the point is in inner area.

	vector<WeichiBitBoard> emptyRegions;
	vector<WeichiBitBoard> lifeStrings;

	WeichiBitBoard bmSurroundPoint;
	array<int, MAX_NUM_GRIDS> surroundCount;

	// Spilt the bitboard area.
	const auto buildRegions = [this](WeichiBitBoard &bmEmptys, vector<WeichiBitBoard> &regions)->void {
		WeichiBitBoard bmMarked;
		for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
			const WeichiGrid& grid = getGrid(*it);			
			for (const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors(); *iNbr != -1; iNbr++) {
				const int pos = *iNbr;
				if (bmEmptys.BitIsOn(pos) && !bmMarked.BitIsOn(pos)) {
					WeichiBitBoard bmResult;
					bmEmptys.floodfill(pos, bmResult);
					bmMarked |= bmResult;
					regions.emplace_back(bmResult);
				}
			}
		}
	};

	// We call an area as the inner area if the each point in the area are not
	// side point or cornor point.
	const auto getInnerArea = [buildRegions](WeichiBitBoard &bmRegion, WeichiBitBoard &bmBorder)->WeichiBitBoard {
		WeichiBitBoard bmResult;
		WeichiBitBoard bmBuf;
		for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
			const int pos = *it;
			if (!bmRegion.BitIsOn(pos)) {
				bmBuf.SetBitOn(pos);
			}
		}

		vector<WeichiBitBoard> buffer;
		buildRegions(bmBuf, buffer);
		for (WeichiBitBoard& bmArea : buffer) {
			if ((bmArea & bmBorder).empty()) {
				bmResult |= bmArea;
			}
		}
		return bmResult;
	};

	const auto isPotentialEye = [this](int pos, Color color, WeichiBitBoard &bmEmptys, WeichiBitBoard &bmInnerArea, array<int, MAX_NUM_GRIDS> &surroundCount)->bool {
		const WeichiGrid& grid = getGrid(pos);

		if (grid.getColor() == color) {
			return false;
		}

		for (const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors(); *iNbr != -1; iNbr++) {
			if (!bmEmptys.BitIsOn(*iNbr)) {
				return false;
			}
		}

		array<int, 4> cornerCount = { 0, 0, 0, 0 };
		for (const int *iDbr = grid.getStaticGrid().getDiagonalNeighbors(); *iDbr != -1; iDbr++) {
			if (bmInnerArea.BitIsOn(*iDbr) || bmEmptys.BitIsOn(*iDbr)) {
				cornerCount[color]++;
			}
			else {
				cornerCount[AgainstColor(color)]++;
			}
		}

		if (cornerCount[COLOR_BORDER] == 0) {
			// The eye is at the center.
			if (cornerCount[AgainstColor(color)] > 1) {
				return false;
			}
		}
		else {
			// The eye is at the board side or corner.
			if (cornerCount[AgainstColor(color)] > 0) {
				return false;
			}
		}

		if (surroundCount[pos] == 0) {
			vector<pair<int, int>> relation;
			for (const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors(); *iNbr != -1; iNbr++) {
				const int val = surroundCount[*iNbr];
				if (val != 0) {
					relation.emplace_back(val, *iNbr);
				}
			}
			if (relation.size() >= 2) {
				// Remove first two surrond points.
				sort(begin(relation), end(relation));
				for (int k = 0; k < 2; ++k) {
					surroundCount[relation[k].second] = 0;
				}
				return false;
			}
		}

		return true;
	};

	const auto isNeighbor = [this](int pos1, int pos2)->bool {
		const WeichiGrid& grid = getGrid(pos1);
		for (const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors(); *iNbr != -1; iNbr++) {
			if (*iNbr == pos2) return true;
		}
		return false;
	};

	// Prepare the basic data.
	surroundCount.fill(0);

	for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
		const int pos = *it;
		if (pos%MAX_BOARD_SIZE == 0 || pos%MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1 ||
			pos / MAX_BOARD_SIZE == 0 || pos / MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) {
			bmBorder.SetBitOn(pos);
		}
		if (!bmLifeZone.BitIsOn(pos)) {
			// Don't need to conside the color. It is not important. We always
			// think it is empty point.
			bmEmptys.SetBitOn(pos);
		}
		else {
			const WeichiGrid& grid = getGrid(pos);
			for (const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors(); *iNbr != -1; iNbr++) {
				if (getGrid(*iNbr).isEmpty()) {
					// We call the  point as surround point if the it is adjacent the
					// pass-alive strings.
					bmSurroundPoint.SetBitOn(*iNbr);
				}
			}
		}
	}

	// Count the priority.
	for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
		const int pos = *it;
		if (bmSurroundPoint.BitIsOn(pos)) {
			int val = 0;
			const WeichiGrid& grid = getGrid(pos);
			for (const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors(); *iNbr != -1; iNbr++) {
				if (getGrid(*iNbr).isEmpty() && !bmSurroundPoint.BitIsOn(*iNbr)) {
					val++;
				}
			}
			surroundCount[pos] = max(1, val); // At least one if the point is surround point.
		}
	}

	// Build all potential pass-dead areas. 
	buildRegions(bmEmptys, emptyRegions);

	// Now compute each region.
	for (WeichiBitBoard &bmRegion : emptyRegions) {
		vector<int> potentiaEyes;
		WeichiBitBoard bmBuf = bmRegion;
		WeichiBitBoard bmInnerArea = getInnerArea(bmRegion, bmBorder);
		int pos;
		while ((pos = bmBuf.bitScanForward()) != -1) {
			// Search the whole region.
			if (isPotentialEye(pos, AgainstColor(color), bmEmptys, bmInnerArea, surroundCount)) {
				potentiaEyes.emplace_back(pos);
			}
		}

		size_t eyesCount = potentiaEyes.size();
		if (eyesCount == 2) {
			// Only one eye if two eyes are adjacent each other.
			if (isNeighbor(potentiaEyes[0], potentiaEyes[1])) {
				eyesCount--;
			}
		}
		if (eyesCount < 2) {
			// Not enough eyes, the region is pass-dead.
			bmDeadArea |= bmRegion;
		}
	}

	return bmDeadArea;
}

 
