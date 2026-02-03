#include "WeichiEndGameCondition.h"
#include "WeichiThreadState.h"
#include <sstream>

void WeichiEndGameCondition::reset()
{
	m_crucialGroups.reset(); 		
	m_searchgoal.reset();
	m_condition.reset();
	m_captureHistory.reset();
	m_bmCrucialBlock.Reset();
	m_bmReigon.Reset();

	return;
}

bool WeichiEndGameCondition::isEndGame(WeichiThreadState& state, Color ownColor)
{	
	WeichiSearchGoal searchGoal = m_searchgoal.get(ownColor);	
	// for 7x7 killall
	if (searchGoal == GOAL_LIVE_ANY) {
		Color liveColor = ownColor;
		FeatureList<WeichiBlock,MAX_GAME_LENGTH>& blockList = state.m_board.getBlockList();	
		for (int iBlock =0; iBlock < blockList.getCapacity(); ++iBlock) {
			WeichiBlock* block = blockList.getAt(iBlock);
			if (!block->valid() || !block->isUsed()) { continue; }
			if (block->getColor() != liveColor) { continue; }
			if (isAlive(state, block)) { return true; }					
		}
		return false;
	} else if (searchGoal == GOAL_KILL_ALL) {
		Color killColor = ownColor;
		if (state.m_board.getAliveArea(killColor) == StaticBoard::getMaskBorder()) { return true; }

		return false;
	}

	// For normal tsumego
	Color crucialColor = (searchGoal == GOAL_LIVE) ? ownColor : AgainstColor(ownColor);
	Vector<WeichiBitBoard, MAX_NUM_BLOCKS> vBmCrucialGroups = m_crucialGroups.get(crucialColor); 
	WeichiGroupCondition groupCondition = m_condition.get(crucialColor);

	WeichiBoard& board = state.m_board;
	bool bAllGroupAlive = true;	
	for (int iGroup = 0; iGroup < vBmCrucialGroups.size(); ++iGroup) {
		WeichiBitBoard bmCrucialStone = vBmCrucialGroups[iGroup];

		if (searchGoal == GOAL_LIVE) {
			bool bHasAtLeastOneAlive = false;
			int pos = 0;
			while ((pos=bmCrucialStone.bitScanForward()) != -1) {				
				const WeichiBlock* block = board.getGrid(pos).getBlock();				
				if (!isAlive(state, block)) { continue; }
				if (block->getColor() != ownColor) { continue; }

				bHasAtLeastOneAlive = true;
				if (groupCondition == CONDITION_OR) { return true; }
			}
			if (!bHasAtLeastOneAlive) { bAllGroupAlive = false; }
		} else if (searchGoal == GOAL_KILL) {
			int pos = 0;
			while ((pos=bmCrucialStone.bitScanForward()) != -1) {
				const WeichiGrid& grid = board.getGrid(pos);
				const WeichiBlock* block = grid.getBlock();
				if (!block) { continue; }
				else { 									
					// has block
					if (block->getColor() != crucialColor) { continue; }

					// only care about crucial block
					const WeichiClosedArea* surCA = grid.getClosedArea(AgainstColor(block->getColor()));					
					if (!surCA) { return false; }					
					if (surCA->getStatus() != LAD_LIFE) { 
						if (WeichiConfigure::use_dead_shape_detection) {
							if (!m_safetySolver.isRegionSafe(surCA, state)) { return false; }							
						} else {
							return false; 
						}
					}
				}
			}				
		}
	}
	
	if (searchGoal == GOAL_LIVE && groupCondition == CONDITION_AND) { return bAllGroupAlive; }
	// For kill, need to capture all crucial stones without regarding to the condition
	if (searchGoal == GOAL_KILL) { return true; }

	return false;
}

WeichiBitBoard WeichiEndGameCondition::getEndGameRZone(WeichiThreadState& state, Color ownColor)
{	
	WeichiSearchGoal searchGoal = m_searchgoal.get(ownColor);
	if (searchGoal == GOAL_LIVE_ANY) {
		Color liveColor = ownColor;
		FeatureList<WeichiBlock,MAX_GAME_LENGTH>& blockList = state.m_board.getBlockList();	
		for (int iBlock = 0; iBlock < blockList.getCapacity(); ++iBlock) {
			WeichiBlock* block = blockList.getAt(iBlock);
			if (!block->valid() || !block->isUsed()) { continue; }
			if (block->getColor() != liveColor) { continue; }
			if (isAlive(state, block)) { return getAliveBlockRZone(state, block); }	
		}
		return WeichiBitBoard();
	} else if (searchGoal == GOAL_KILL_ALL) {
		WeichiBitBoard bmFullBoard = StaticBoard::getMaskBorder();
		return bmFullBoard;
	}

	Color crucialColor = (searchGoal == GOAL_LIVE) ? ownColor : AgainstColor(ownColor);
	Vector<WeichiBitBoard, MAX_NUM_BLOCKS> vBmCrucialGroups = m_crucialGroups.get(crucialColor); 
	WeichiGroupCondition groupCondition = m_condition.get(crucialColor);
	
	WeichiBoard& board = state.m_board;
	bool bAllGroupAlive = true;
	WeichiBitBoard bmRZone;	
	for (int iGroup = 0; iGroup < vBmCrucialGroups.size(); ++iGroup) {
		WeichiBitBoard bmCrucialStone = vBmCrucialGroups[iGroup];

		if (searchGoal == GOAL_LIVE) {
			bool bHasAtLeastOneAlive = false;
			int pos = 0;
			while ((pos=bmCrucialStone.bitScanForward()) != -1) {				
				const WeichiBlock* block = board.getGrid(pos).getBlock();				
				if (!isAlive(state, block)) { continue; }
				if (block->getColor() != ownColor) { continue; }

				bHasAtLeastOneAlive = true;
				bmRZone |= getAliveBlockRZone(state, block);
				if (groupCondition == CONDITION_OR) { return bmRZone; }
			}
			if (!bHasAtLeastOneAlive) { bAllGroupAlive = false; }
		} else if (searchGoal == GOAL_KILL) {			
			int pos = 0;
			while ((pos=bmCrucialStone.bitScanForward()) != -1) {
				const WeichiGrid& grid = board.getGrid(pos);
				const WeichiBlock* block = grid.getBlock();				
				if (block) { 
					if (block->getColor() != crucialColor) { continue; }
					bmRZone |= getDeadBlockRZone(state, block);
				} 
				else { bmRZone.SetBitOn(pos);  }
			}				
		}
	}
	
	if (searchGoal == GOAL_LIVE && groupCondition == CONDITION_AND) { 
		return bmRZone;		
	}
	// For kill, need to capture all crucial stones without regarding to the condition
	if (searchGoal == GOAL_KILL) { return bmRZone; }

	return WeichiBitBoard();
}

void WeichiEndGameCondition::setCrucialStones(WeichiThreadState& state, Color color, string sCrucialStone)
{
	// ex: bs,es&da,cd,be
	// ex: bs,es|da,cd,be
	char cSeparator = ' ';
	for (int i = 0; i < sCrucialStone.length(); ++i) {
		if (sCrucialStone[i] == '&') { 
			m_condition.get(color) = CONDITION_AND; 
			cSeparator = '&';
			break;
		} else if (sCrucialStone[i] == '|') { 
			m_condition.get(color) = CONDITION_OR;
			cSeparator = '|';
			break;
		}
	}
	
	const WeichiBoard& board = state.m_board;

	Vector<WeichiBitBoard, MAX_NUM_BLOCKS>& vBmCrucialGroups = m_crucialGroups.get(color);
	istringstream iss(sCrucialStone);
	string group;
	while (std::getline(iss, group, cSeparator)) {
		WeichiBitBoard bmGroup;
		istringstream issGroup(group); 
		string coor;
		while (std::getline(issGroup, coor, ',')) {			
			WeichiMove move(COLOR_NONE, coor);
			bmGroup.SetBitOn(move.getPosition());			
			// TO DO: refactor this?
			const WeichiGrid& grid = board.getGrid(move.getPosition());
			const WeichiBlock* block = grid.getBlock();
			if (block) { m_bmCrucialBlock |= block->getStoneMap(); }
			else {
				CERR() << "[WARNING] No crucial stone at " << move.toGtpString() << endl;
			}
		}
		vBmCrucialGroups.push_back(bmGroup);
	}

	return;
}

WeichiBitBoard WeichiEndGameCondition::getCrucialStonesBitBoard()
{
	return (getCrucialStones(COLOR_BLACK) | getCrucialStones(COLOR_WHITE));
}

WeichiBitBoard WeichiEndGameCondition::getCrucialBlocksBitBoard()
{
	return m_bmCrucialBlock;
}

WeichiBitBoard WeichiEndGameCondition::getCrucialStones(Color color)
{
	WeichiBitBoard bmCrucialStone;
	Vector<WeichiBitBoard, MAX_NUM_BLOCKS> vBmCrucialGroups = m_crucialGroups.get(color);
	for (int iGroup = 0; iGroup < vBmCrucialGroups.size(); ++iGroup) {
		bmCrucialStone |= vBmCrucialGroups[iGroup];
	}	

	return bmCrucialStone;
}

string WeichiEndGameCondition::getCruicalStoneSgfString(Color color)
{
	ostringstream oss;
	WeichiBitBoard bmCrucialStone = getCrucialStones(color);
	int pos=0; 
	while ( (pos=bmCrucialStone.bitScanForward()) != -1) {
		oss << WeichiMove(COLOR_NONE,pos).toSgfString(false) << " ";
	}

	return oss.str();
}

string WeichiEndGameCondition::getCruicalStoneEditorString(Color color)
{
	ostringstream oss;
	WeichiBitBoard bmCrucialStone = getCrucialStones(color);
	int pos=0; 
	while ( (pos=bmCrucialStone.bitScanForward()) != -1) {
		oss << WeichiMove(COLOR_NONE,pos).toEditorGtpString() << " ";
	}

	return oss.str();
}

string WeichiEndGameCondition::getCruicalGroupInfo(Color color)
{
	ostringstream oss;		
	oss << endl;
	oss << toChar(color) << " " << toGroupConditionString(m_condition.get(color)) << endl;
	Vector<WeichiBitBoard, MAX_NUM_BLOCKS> vBmCrucialGroups = m_crucialGroups.get(color);
	for (int iGroup=0; iGroup < vBmCrucialGroups.size(); ++iGroup) {
		WeichiBitBoard bmCrucialStone = vBmCrucialGroups[iGroup];
		oss << "G" << iGroup << ": ";
		int pos=0; 
		while ( (pos=bmCrucialStone.bitScanForward()) != -1) {
			oss << WeichiMove(COLOR_NONE,pos).toGtpString() << " ";
		}
		oss << endl;
	}	

	return oss.str();
}

void WeichiEndGameCondition::setBoardCrucialStone(WeichiThreadState& state)
{	
	WeichiBitBoard bmCrucialMap = getCrucialBlocksBitBoard();
	WeichiBitBoard bmBoardMask = StaticBoard::getMaskBorder();
	Dual<WeichiBitBoard>& vLifeStones = state.m_board.getLADToLifeStones();
	Dual<WeichiBitBoard>& vKillStones = state.m_board.getLADToKillStones();
	vLifeStones.reset();
	vKillStones.reset();
	WeichiSearchGoal blackGoal = m_searchgoal.get(COLOR_BLACK);
	WeichiSearchGoal whiteGoal = m_searchgoal.get(COLOR_WHITE);
	switch (blackGoal) {
		case GOAL_KILL: vKillStones.get(COLOR_BLACK) = bmCrucialMap; break;
		case GOAL_KILL_ALL: vKillStones.get(COLOR_BLACK) = bmBoardMask; break;
		case GOAL_LIVE: vLifeStones.get(COLOR_BLACK) = bmCrucialMap; break;		
		case GOAL_LIVE_ANY: vLifeStones.get(COLOR_BLACK) = bmBoardMask; break;
		default: CERR() << "Unsupported Black Search Goal." << endl;
	}

	switch (whiteGoal) {
		case GOAL_KILL: vKillStones.get(COLOR_WHITE) = bmCrucialMap; break;
		case GOAL_KILL_ALL: vKillStones.get(COLOR_WHITE) = bmBoardMask; break;
		case GOAL_LIVE: vLifeStones.get(COLOR_WHITE) = bmCrucialMap; break;
		case GOAL_LIVE_ANY: vLifeStones.get(COLOR_WHITE) = bmBoardMask; break;
		default: CERR() << "Unsupported White Search Goal." << endl;
	}

	Dual<bool>& koAvailable = state.m_board.getLADKoAvailable();		
	if (WeichiConfigure::black_ignore_ko) { koAvailable.get(COLOR_BLACK) = false; }
	else { koAvailable.get(COLOR_BLACK) = true; }
	if (WeichiConfigure::white_ignore_ko) { koAvailable.get(COLOR_WHITE) = false; }
	else { koAvailable.get(COLOR_WHITE) = true; }

	return;
}

void WeichiEndGameCondition::setSearchGoal(Color color, string sSearchGoal)
{	
	if (sSearchGoal == "TOKILL") { m_searchgoal.get(color) = WeichiSearchGoal::GOAL_KILL; }
	else if (sSearchGoal == "TOKILL_ALL") { m_searchgoal.get(color) = WeichiSearchGoal::GOAL_KILL_ALL; }
	else if (sSearchGoal == "TOLIVE") { m_searchgoal.get(color) = WeichiSearchGoal::GOAL_LIVE; }
	else if (sSearchGoal == "TOLIVE_ANY") { m_searchgoal.get(color) = WeichiSearchGoal::GOAL_LIVE_ANY; }

	return;
}

void WeichiEndGameCondition::setRegion(string sRegion)
{
	string coor;
	istringstream issRegion(sRegion);
	while (std::getline(issRegion, coor, ',')) {
		WeichiMove move(COLOR_NONE, coor);
		m_bmReigon.SetBitOn(move.getPosition());
	}

	return;
}

string WeichiEndGameCondition::toString()
{
	ostringstream oss;
	Vector<WeichiBitBoard, MAX_NUM_BLOCKS> vBlackCrucial = m_crucialGroups.get(COLOR_BLACK);
	Vector<WeichiBitBoard, MAX_NUM_BLOCKS> vWhiteCrucial = m_crucialGroups.get(COLOR_WHITE);

	oss << "[Black Cruical Stone]" << endl;
	oss << "Black Search Goal: " << getWeichiSearchGoalString(m_searchgoal.get(COLOR_BLACK)) << endl;
	oss << "Condition: " << toGroupConditionString(m_condition.get(COLOR_BLACK)) << endl;
	for (int iGroup=0; iGroup < vBlackCrucial.size(); ++iGroup) {
		WeichiBitBoard bmCrucial = vBlackCrucial[iGroup];
		oss << "\tGroup " << iGroup << ": ";
		int pos = 0;
		while ((pos=bmCrucial.bitScanForward()) != -1) {
			WeichiMove move(COLOR_NONE, pos);
			oss << move.toGtpString() << " ";
		}
		oss << endl;
	}

	oss << "[White Cruical Stone]" << endl;
	oss << "White Search Goal: " << getWeichiSearchGoalString(m_searchgoal.get(COLOR_WHITE)) << endl;
	oss << "Condition: " << toGroupConditionString(m_condition.get(COLOR_WHITE)) << endl;
	for (int iGroup=0; iGroup < vWhiteCrucial.size(); ++iGroup) {
		WeichiBitBoard bmCrucial = vWhiteCrucial[iGroup];
		oss << "\tGroup " << iGroup << ": ";
		int pos = 0;
		while ((pos=bmCrucial.bitScanForward()) != -1) {
			WeichiMove move(COLOR_NONE, pos);
			oss << move.toGtpString() << " ";
		}
		oss << endl;
	}	

	if (!m_bmReigon.empty()) {
		oss << "[Search Region]" << endl;
		oss << m_bmReigon.toString(WeichiConfigure::BoardSize, WeichiConfigure::BoardSize) << endl;
	}

	return oss.str();
}

bool WeichiEndGameCondition::isAlive(WeichiThreadState& state, const WeichiBlock* block)
{
	if (!block) { return false; }
	if (block->getStatus() == LAD_LIFE) { return true; }
	if (WeichiConfigure::use_early_life) { return state.m_lifedeathHandler.isBlockEarlyLife(block); }

	return false; 
}

WeichiBitBoard WeichiEndGameCondition::getDeadBlockRZone(WeichiThreadState& state, const WeichiBlock* block)
{
	if (!block) { return WeichiBitBoard(); }

	WeichiBitBoard bmRZone;
	Color oppColor = AgainstColor(block->getColor());
	const WeichiClosedArea* oppCA = state.m_board.getGrid(block->getiFirstGrid()).getClosedArea(oppColor);

	if (!oppCA) { return bmRZone; }

	if (oppCA->getStatus() == LAD_LIFE) {		
		// the zone would contain the oppCA if it is LAD_LIFE
		bmRZone = WeichiRZoneHandler::calculateDeadRZone(state.m_board, WeichiMove(COLOR_NONE, block->getiFirstGrid()));
	} else if (WeichiConfigure::use_dead_shape_detection && oppCA->getStatus() != LAD_LIFE) {
		bmRZone = WeichiRZoneHandler::calculateDeadRZone(state.m_board, WeichiMove(COLOR_NONE, block->getiFirstGrid()));
		bmRZone |= oppCA->getStoneMap();
	}

	return bmRZone;
}

WeichiBitBoard WeichiEndGameCondition::getAliveBlockRZone(WeichiThreadState& state, const WeichiBlock * block)
{
	if (!block) { return WeichiBitBoard(); }

	WeichiBitBoard bmRZone;
	if (block->getStatus() == LAD_LIFE) { 
		bmRZone = WeichiRZoneHandler::calculateBensonRZone(state.m_board, WeichiMove(COLOR_NONE, block->getiFirstGrid())); 
	} else {
		bmRZone = state.m_lifedeathHandler.getEarlyLifeBlockRZone(block);
	}

	return bmRZone;
}

 
