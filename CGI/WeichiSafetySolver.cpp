#include "WeichiSafetySolver.h"
#include "WeichiThreadState.h"

void WeichiSafetySolver::reset()
{
	m_regionColor = COLOR_NONE;
	m_bmSafeBlock.Reset();
	m_bmEyeSpace.Reset();
	m_bmEyePoint.Reset();
}

bool WeichiSafetySolver::isRegionSurroundedBySafeBlocks(const WeichiClosedArea* region, WeichiThreadState& state)
{
	FeatureList<WeichiBlock, MAX_GAME_LENGTH>& blocklist = state.m_board.getBlockList();
	for (int iBlock = 0; iBlock < blocklist.getCapacity(); ++iBlock) {
		if (!state.m_board.getBlockList().isValidIdx(iBlock)) { continue; }
		WeichiBlock* block = state.m_board.getBlockList().getAt(iBlock);
		if (!block->isUsed()) { continue; }		
		// skip dangling blocks
		if (block->getLibertyBitBoard(state.m_board.getBitBoard()).isSubsetOf(region->getStoneMap())) { continue; }

		if (block->getStatus() != LAD_LIFE) { return false; }
	}

	return true;
}

bool WeichiSafetySolver::isRegionSafe(const WeichiClosedArea* region, WeichiThreadState& state)
{
	assert(region->getStatus() != LAD_LIFE);

	if (!isRegionSurroundedBySafeBlocks(region, state)) { return false; }

	reset();
	m_regionColor = region->getColor();
	computeFullBoardSafeBlocks(state);
	computeRegionSafeBlocks(region, state);
	computeEyeSpace(region,state);

	// no two nonadjacent potential eye points remain in the region
	if (m_bmEyeSpace.bitCount() < 2) { return true; }
	else if (m_bmEyeSpace.bitCount() == 2) {
		Vector<uint, 2> vEyeSpace;
		m_bmEyeSpace.bitScanAll(vEyeSpace);
		int eye1 = vEyeSpace[0], eye2 = vEyeSpace[1];
		const WeichiGrid& eyegrid = state.m_board.getGrid(eye1);
		for (const int *iNbr = eyegrid.getStaticGrid().getAdjacentNeighbors(); *iNbr != -1; iNbr++) {
			if (*iNbr == eye2) { return true; } 
		}
	}

	return false;
}

void WeichiSafetySolver::computeFullBoardSafeBlocks(WeichiThreadState& state)
{
	FeatureList<WeichiBlock, MAX_GAME_LENGTH>& blocklist = state.m_board.getBlockList();
	for (int iBlock = 0; iBlock < blocklist.getCapacity(); ++iBlock) {
		if (!state.m_board.getBlockList().isValidIdx(iBlock)) { continue; }
		WeichiBlock* block = state.m_board.getBlockList().getAt(iBlock);
		if (!block->isUsed()) { continue; }
		if (block->getStatus() == LAD_LIFE) { m_bmSafeBlock |= block->getStoneMap(); }
	}

	return; 
}

void WeichiSafetySolver::computeRegionSafeBlocks(const WeichiClosedArea* region, WeichiThreadState& state)
{
	WeichiBitBoard bmSafeBlockLiberty;
	for (uint iNumBlock = 0; iNumBlock < region->getNumBlock(); iNumBlock++) {
		const WeichiBlock* caNbrBlock = state.m_board.getBlock(region->getBlockID(iNumBlock));
		if (caNbrBlock->getStatus() == LAD_LIFE) { bmSafeBlockLiberty |= caNbrBlock->getLibertyBitBoard(state.m_board.getBitBoard()); }
	}

	for (uint iNumBlock = 0; iNumBlock < region->getNumBlock(); iNumBlock++) {
		const WeichiBlock* caNbrBlock = state.m_board.getBlock(region->getBlockID(iNumBlock));
		if (caNbrBlock->getStatus() == LAD_LIFE) { continue; }

		// For the block that is not benson
		WeichiBitBoard bmLibery = caNbrBlock->getLibertyBitBoard(state.m_board.getBitBoard());
		if ((bmLibery & bmSafeBlockLiberty).bitCount() >= 2) {
			m_bmSafeBlock |= caNbrBlock->getStoneMap();
		}
	}
	
	return;
}

void WeichiSafetySolver::computeEyeSpace(const WeichiClosedArea* region, WeichiThreadState& state)
{
	WeichiBitBoard bmRegion = region->getStoneMap();	
	// add the dangling stones into the candidate of eye space
	for (uint iNumBlock = 0; iNumBlock < region->getNumBlock(); iNumBlock++) {
		const WeichiBlock* caNbrBlock = state.m_board.getBlock(region->getBlockID(iNumBlock));
		if (caNbrBlock->getStatus() == LAD_LIFE) { continue; }

		WeichiBitBoard bmLibery = caNbrBlock->getLibertyBitBoard(state.m_board.getBitBoard());
		if (bmLibery.isSubsetOf(region->getStoneMap())) {
			bmRegion |= caNbrBlock->getStoneMap();
		}
	}

	int pos = 0;
	while ((pos = bmRegion.bitScanForward()) != -1) {
		const WeichiGrid& grid = state.m_board.getGrid(pos);
		checkNbr(grid, state);
	}

	return;
}

void WeichiSafetySolver::checkNbr(const WeichiGrid& grid, WeichiThreadState& state)
{	
	Color color = grid.getColor();
	Color killColor = m_regionColor;
	Color liveColor = AgainstColor(killColor);
	
	if (color == liveColor) {
		return; 
	} else if (color == COLOR_NONE) {
		bool bEyePoint = true;
		int numEmptyAllowed = 1;
		for (const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors(); *iNbr != -1; iNbr++) {
			const WeichiGrid& nbrGrid = state.m_board.getGrid(*iNbr);
			if (nbrGrid.getColor() == killColor) { return; }
			else if (nbrGrid.getColor() == COLOR_NONE) {
				--numEmptyAllowed;
				bEyePoint = (numEmptyAllowed >= 0);
			}
		}
		int numDiagIgnored, numDiagonalOccupied = 0;
		if (grid.getStaticGrid().getLineNo() > 1) { numDiagIgnored = 2; }
		else { numDiagIgnored = 1; }

		for (const int *iDiag = grid.getStaticGrid().getDiagonalNeighbors(); *iDiag != -1; iDiag++) {
			const WeichiGrid& diagGrid = state.m_board.getGrid(*iDiag);
			if (m_bmSafeBlock.BitIsOn(*iDiag)) { ++numDiagonalOccupied; }			
		}

		if (numDiagonalOccupied >= numDiagIgnored) { return; }

		m_bmEyeSpace.SetBitOn(grid.getPosition());
		
		// TO DO: eye point?
	} else if (color == killColor) {
		if (m_bmSafeBlock.BitIsOn(grid.getPosition())) { return; }

		// special case
		const WeichiBlock* block = grid.getBlock();
		if (block->getNumStone() == 1 && block->getLiberty() == 1) {
			int numDiagIgnored, numDiagonalOccupied = 0;
			if (grid.getStaticGrid().getLineNo() > 1) { numDiagIgnored = 2; }
			else { numDiagIgnored = 1; }

			for (const int *iDiag = grid.getStaticGrid().getDiagonalNeighbors(); *iDiag != -1; iDiag++) {
				const WeichiGrid& diagGrid = state.m_board.getGrid(*iDiag);
				if (m_bmSafeBlock.BitIsOn(*iDiag)) { ++numDiagonalOccupied; }
			}
			if (numDiagonalOccupied >= numDiagIgnored) { return; }

			m_bmEyeSpace.SetBitOn(grid.getPosition());
		} else {
			WeichiBitBoard bmStone = block->getStoneMap();
			m_bmEyeSpace |= bmStone;
		}
	}

	return;
}