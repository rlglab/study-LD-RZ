#include "WeichiJLMctsHandler.h"
#include "concepts.h"

WeichiJLMctsHandler::WeichiJLMctsHandler(MCTS& instance) : m_mcts(instance)
{
	reset();
}

void WeichiJLMctsHandler::reset()
{	
	m_best_move = PASS_MOVE;
	m_bestSD.reset();
}

void WeichiJLMctsHandler::playSgfPathString(string path)
{
	string normalizedPath = normalizePathStringFromJLMCTS(path);
	for (int pos = 0; normalizedPath.find_first_of("BW", pos) != string::npos; pos += 5) {
		Color c = toColor(normalizedPath[pos]);
		string pos_str = normalizedPath.substr(pos+2, 2);
		WeichiMove m(c, pos_str);
		m_mcts.play(m);
	}
}

void WeichiJLMctsHandler::set7x7JobLevelConfigurations()
{
	StaticBoard::initializeBoardSizeArgument(7);
	playSgfPathString(WeichiConfigure::Path);
	WeichiConfigure::komi = 30;
	WeichiConfigure::dcnn_use_ftl = true;
	WeichiConfigure::win_color = 2; // win for white
	WeichiGlobalInfo::getTreeInfo().m_problemSgfFilename = "JLMCTS.sgf";
	WeichiGlobalInfo::getTreeInfo().m_winColor = Color(WeichiConfigure::win_color);
	WeichiConfigure::black_ignore_ko = false;
	WeichiConfigure::white_ignore_ko = false;
	WeichiGlobalInfo::getEndGameCondition().setSearchGoal(COLOR_BLACK, "TOKILL_ALL");
	WeichiGlobalInfo::getEndGameCondition().setSearchGoal(COLOR_WHITE, "TOLIVE_ANY");	
}

void WeichiJLMctsHandler::solve()
{
	set7x7JobLevelConfigurations();
	m_mcts.genmove(false);
	computeOutputInformation();
}

string WeichiJLMctsHandler::normalizePathStringFromJLMCTS(string path)
{
	string normalizedString = path;
	for (int pos = 0; normalizedString.find_first_of("BW", pos) != string::npos; pos += 5) {
		if (normalizedString[pos+3] != '@')
			normalizedString[pos+3] = normalizedString[pos+3] - 'A' + '0' + 1;
	}

	return normalizedString;
}

void WeichiJLMctsHandler::computeOutputInformation()
{
	UctNodePtr pRoot = WeichiGlobalInfo::getTreeInfo().m_pRoot;	
	Color toPlay = AgainstColor(pRoot->getColor());
	Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
	Color killColor = AgainstColor(winColor);
	for (UctChildIterator<WeichiUctNode> it(pRoot); it; ++it) {
		if (it->getColor() == winColor) {
			if (it->isProved()) {
				m_bestNode = it;
				m_bestSD = it->getUctDataWithoutVirtualLoss();
				m_best_move = it->getMove();
				break;
			}
			else if (it->getUctDataWithoutVirtualLoss().getCount() > m_bestSD.getCount()) {
				m_bestNode = it;
				m_bestSD = it->getUctDataWithoutVirtualLoss();
				m_best_move = it->getMove();
			}
		}
		else if (it->getColor() == killColor) {
			if (it->getUctDataWithoutVirtualLoss().getCount() > m_bestSD.getCount() && !it->isProved()) {
				m_bestNode = it;
				m_bestSD = it->getUctDataWithoutVirtualLoss();
				m_best_move = it->getMove();
			}
		}
	}

	// bestNode might be null if every child is solved
	if (m_bestNode.isNull()) { m_best_move.setColor(toPlay); }
}

string WeichiJLMctsHandler::getResult() const
{
	ostringstream oss;
	oss << getGeneratedMoveResult();
	oss << getWinRateResult();
	oss << getICResult();
	oss << getVCResult();
	oss << getRootRZoneResult();
	oss << getCurrentRZoneResult();
	oss << getRootWinStatusResult();
	oss << getCurrentWinStatusResult();
	oss << getRootLegalBitMap();
	oss << getCurrentLegalBitMap();
	oss << getLastResult();
	oss << getAIEndMessage();	

	return oss.str();
}

string WeichiJLMctsHandler::getGeneratedMoveResult() const
{
	return ToString(toChar(m_best_move.getColor())) + "[" + m_best_move.toEditorMoveJLMCTSString() + "]";
}

string WeichiJLMctsHandler::getWinRateResult() const
{
	Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
	Color killColor = AgainstColor(winColor);
	UctNodePtr pRoot = WeichiGlobalInfo::getTreeInfo().m_pRoot;
	ostringstream oss;
	if (pRoot->getColor() == killColor && pRoot->getUctNodeStatus() == UCT_STATUS_WIN) { oss << "WR[1.00]"; }
	else { oss << "WR[" << fixed << setprecision(2) << double(m_bestSD.getMean()) << "]"; }

	return oss.str();
}

string WeichiJLMctsHandler::getICResult() const
{
	ostringstream oss;
	oss << "IC[" << ToString(WeichiGlobalInfo::getTreeInfo().m_nSimulation) << "]";
	return oss.str();
}

string WeichiJLMctsHandler::getVCResult() const
{
	ostringstream oss;	
	oss << "VC[" << ToString(m_bestSD.getCount()) << "]";
	return oss.str();
}

string WeichiJLMctsHandler::getRootRZoneResult() const
{	
	UctNodePtr pRoot = WeichiGlobalInfo::getTreeInfo().m_pRoot;
	WeichiBitBoard bmRootRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(pRoot->getProofRZoneID());
	ostringstream oss;	
	oss << "RRZONEBITMAP[";
	if (pRoot->getUctNodeStatus() == UCT_STATUS_WIN) { oss << toEditorBitBoardValueString(bmRootRZone); }
	else { oss << "0"; }
	oss << "]";

	return oss.str();
}

string WeichiJLMctsHandler::getCurrentRZoneResult() const
{		
	ostringstream oss;
	oss << "CRZONEBITMAP[";
	if (m_bestNode.isNull()) { oss << "0"; }
	else if (m_bestNode->getUctNodeStatus() == UCT_STATUS_WIN) {
		// TO DO:
		WeichiBitBoard bmBestMoveRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(m_bestNode->getProofRZoneID());
		oss << toEditorBitBoardValueString(bmBestMoveRZone);
	} else { oss << "0"; }
	oss << "]";

	return oss.str();
}

string WeichiJLMctsHandler::getRootWinStatusResult() const
{		
	Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
	Color killColor = AgainstColor(winColor);
	UctNodePtr pRoot = WeichiGlobalInfo::getTreeInfo().m_pRoot;	

	ostringstream oss;
	if (pRoot->getUctNodeStatus() == UCT_STATUS_WIN) {
		if (pRoot->getColor() == winColor) { oss << "RWIN[1]"; }
		else if (pRoot->getColor() == killColor) { oss << "RWIN[2]"; }
	}
	else oss << "RWIN[0]";

	return oss.str();
}

string WeichiJLMctsHandler::getCurrentWinStatusResult() const
{
	Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
	Color killColor = AgainstColor(winColor);	
	Color moveColor = m_best_move.getColor();
	ostringstream oss;

	if (m_bestNode.isNull()) { oss << "CWIN[2]"; }
	else {
		if (m_bestNode->getColor() == winColor && m_bestNode->getUctNodeStatus() == UCT_STATUS_WIN) {
			oss << "CWIN[1]";
		} else { oss << "CWIN[0]"; }
	}

	return oss.str();
}

string WeichiJLMctsHandler::getRootLegalBitMap() const
{
	UctNodePtr pRoot = WeichiGlobalInfo::getTreeInfo().m_pRoot;
	WeichiBitBoard bmRootLegal;
	for (UctChildIterator<WeichiUctNode> it(pRoot); it; ++it) {
		if (!it->isProved()) { bmRootLegal.SetBitOn(it->getPosition()); }
	}

	ostringstream oss;
	oss << "RLEGALBITMAP[" << toEditorBitBoardValueString(bmRootLegal) << "]";
	return oss.str();
}

string WeichiJLMctsHandler::getCurrentLegalBitMap() const
{
	WeichiBitBoard bmCurrentLegal;
	if (!m_bestNode.isNull()) {
		for (UctChildIterator<WeichiUctNode> it(m_bestNode); it; ++it) {
			if (!it->isProved()) { bmCurrentLegal.SetBitOn(it->getPosition()); }
		}
	}

	ostringstream oss;
	oss << "CLEGALBITMAP[" << toEditorBitBoardValueString(bmCurrentLegal) << "]";
	return oss.str();
}

string WeichiJLMctsHandler::getLastResult() const
{
	ostringstream oss;
	oss << "LAST";
	if (WeichiGlobalInfo::get()->m_bIsLastCandidate) { oss << "[1]"; }
	else { oss << "[0]"; }

	return oss.str();
}

string WeichiJLMctsHandler::getAIEndMessage() const
{
	return "AI_END\n";
}

string WeichiJLMctsHandler::toEditorBitBoardValueString(WeichiBitBoard bm) const
{
	// use 64bit to represent 7x7 board size
	// The mapping from bitboard position to the real board position is in the following:
	// 0   1   2       6  | 20  21  22 ..  |...   125 126   (The position 7,8, .... is not used in the 7x7 board size) 
	// AA, BA, CA, ....GA,  AB, BB, CB, ....FG, GG.
	// And in the hex coding mapping

	bool DO_CHECK_PASS_BIT = true;
	int pos = 0;
	ull result_value = 0;	
	for (int i = 0; i < WeichiConfigure::BoardSize; i++) {
		for (int j = 0; j < WeichiConfigure::BoardSize; j++) {
			int position = MAX_BOARD_SIZE * i + j;
			int index = 7 * i + j;			
			if (bm.BitIsOn(position)) { result_value |= (1ULL << index); }
		}
	}

	// Check pass bit
	if (DO_CHECK_PASS_BIT && bm.BitIsOn(WeichiMove::PASS_POSITION)) {
		result_value |= (1ULL << 49);
	}

	ostringstream oss;
	oss << std::hex << result_value;

	return oss.str();
}