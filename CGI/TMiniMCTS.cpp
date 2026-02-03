#include "TMiniMCTS.h"
#include "SgfLoader.h"

void TMiniMCTS::newGame()
{
	newTree();
	m_vMoveComments.clear();
	m_state.resetThreadState();
	m_fKomi = WeichiConfigure::komi;
	m_bEnableResign = (Random::nextReal() > WeichiConfigure::zero_disable_resign_prob);
	m_minWinRate.m_black = m_minWinRate.m_white = 1.0f;

	loadOpenings();
}

void TMiniMCTS::play(const WeichiMove &move)
{
	m_state.play(move, true);
	newTree();
}

void TMiniMCTS::runMCTSSimulationBeforeForward()
{
	// if maximum simulation is 1, return immediately
	if (MAX_NUM_SIMULATION == 1) { return; }

	if (m_simulation == 0) {
		newTree();
		m_state.backup();
	}
	selection();

	WeichiBitBoard bmLegal;
	float inputFeatures[MAX_CNN_CHANNEL_SIZE*MAX_NUM_GRIDS];
	WeichiCNNNet* cnnNet = getCNNNet(m_state.getRootTurn());
	WeichiCNNFeatureType cnnFeatureType = cnnNet->getCNNFeatureType();
	WeichiCNNFeatureGenerator::calculateDCNNFeatures(getState(), cnnFeatureType, inputFeatures, bmLegal);
	cnnNet->set_data(getBatchID(), inputFeatures, getState().m_board.getToPlay(), bmLegal);
}

void TMiniMCTS::runMCTSSimulationAfterForward()
{
	// if maximum simulation is 1, random play to end game
	if (MAX_NUM_SIMULATION == 1) {
		while (true) { randomPlayGame(); }
	}

	if (m_vPath.size() == 0) { return; }

	DCNNResult dcnnResult = getDCNNResult();
	if (isLearned(dcnnResult)) { newGame(); return; }

	adjustKomi(dcnnResult);
	expandAndEvaluate(dcnnResult);
	update(dcnnResult);
	m_state.rollback();

	++m_simulation;

	if (isSimulationEnd()) { handle_simulationEnd(); }
}

WeichiMove TMiniMCTS::selectMCTSMove()
{
	uint childNum = 0;
	double dSum = 0.0f;
	WeichiMove bestMove = PASS_MOVE;
	TMiniNode* pBest = nullptr;
	TMiniNode* pRoot = getRootNode();
	
	childNum = 0;
	m_MCTSSearchDistribution = "";
	bool bAddComma = false;
	for (TMiniNode *pChild = pRoot->getFirstChild(); childNum < pRoot->getNumChild(); ++pChild, ++childNum) {
		const StatisticData& uctData = pChild->getUctData();
		if (uctData.getCount() == 0) { continue; }
		//if (maxCount > 1 && uctData.getCount() == 1) { continue; }
		//if (m_state.m_board.getMoveList().size() >= 30 && uctData.getCount() < maxCount * WeichiConfigure::zero_threshold_ratio) { continue; }

		double dCount = uctData.getCount();
		dSum += dCount;
		double dRand = Random::nextReal(dSum);
		if (dRand < dCount) {
			bestMove = pChild->getMove();
			pBest = pChild;
		}

		if (uctData.getCount() > 0) {
			m_MCTSSearchDistribution += (bAddComma ? "," : "");
			m_MCTSSearchDistribution += ToString(WeichiMove::toCompactPosition(pChild->getMove().getPosition())) + ":";
			m_MCTSSearchDistribution += ToString(dCount);
			bAddComma = true;
		}
	}

	m_pBest = pBest;
	m_simulation = 0;

	// debug information
	WeichiCNNNet* cnnNet = getCNNNet(m_state.getRootTurn());
	m_MCTSSearchDistribution += getSearchDebugInformation();

	return bestMove;
}

WeichiPlayoutResult TMiniMCTS::getGameResult(bool bResign)
{
	WeichiPlayoutResult result(0);
	if (bResign) {
		Color turnColor = m_state.m_board.getToPlay();
		result = (turnColor == COLOR_BLACK) ? WeichiPlayoutResult(-MAX_NUM_GRIDS) : WeichiPlayoutResult(MAX_NUM_GRIDS);
	} else { result = m_state.eval(m_fKomi); }

	return result;
}

bool TMiniMCTS::isResign()
{
	const bool TRAIN_LAD_PROBLEM = true;
	if (TRAIN_LAD_PROBLEM) {
		// always play to end game when training LAD problems
		return false;
	} else {
		const StatisticData& rootData = getRootNode()->getUctData();
		const StatisticData& bestData = m_pBest->getUctData();
		const float RESIGN_THRESHOLD = WeichiConfigure::zero_resign_threshold;
		return (rootData.getMean() > -RESIGN_THRESHOLD && bestData.getMean() < RESIGN_THRESHOLD);
	}
}

bool TMiniMCTS::isSimulationEnd()
{
	return m_simulation >= MAX_NUM_SIMULATION;
}

// private function
void TMiniMCTS::newTree()
{
	m_nodeIndex = 1;
	m_simulation = 0;

	// initialize root node
	Color rootColor = AgainstColor(m_state.m_board.getToPlay());
	m_nodes->reset(WeichiMove(rootColor));
}

void TMiniMCTS::loadOpenings()
{
	string sSgf;
	int random_shift = Random::nextInt(1000);	// random shift 1~3 when reading sgf
	while (random_shift >= 0) { sSgf = getOneOpening(); --random_shift; }

	m_sgfLoader.parseFromString(sSgf);
	Color turnColor = m_sgfLoader.hasSgfTag("PL") ? toColor(m_sgfLoader.getSgfTag("PL")[0]) : COLOR_BLACK;
	for (int i = 0; i < m_sgfLoader.getPreset().size(); ++i) { m_state.m_board.preset(m_sgfLoader.getPreset()[i]); }
	m_state.m_rootFilter.startFilter();
	m_state.setRootTurn(turnColor);
	m_state.m_board.setToPlay(turnColor);

	m_LADSgfTags = m_sgfLoader.getSgfTag("EV");
	vector<string> vSplit;
	vSplit.push_back("");
	for (auto c : m_LADSgfTags) {
		if (c == ';') { vSplit.push_back(""); }
		else { vSplit.back().append(1, c); }
	}

	// format: BL ; BK ; WL ; WK ; type
	if (vSplit.size() != 5) { CERR() << "LAD format error!! " << m_LADSgfTags << endl; }
	setLADBitBoard(vSplit[0], m_state.m_board.getLADToLifeStones().get(COLOR_BLACK));
	setLADBitBoard(vSplit[1], m_state.m_board.getLADToKillStones().get(COLOR_BLACK));
	setLADBitBoard(vSplit[2], m_state.m_board.getLADToLifeStones().get(COLOR_WHITE));
	setLADBitBoard(vSplit[3], m_state.m_board.getLADToKillStones().get(COLOR_WHITE));
	if (vSplit[4] == "UCL" || vSplit[4] == "KbK") {
		if (vSplit[0] == "") {
			// B to kill, W to live
			m_state.m_board.getLADKoAvailable().m_black = true;
			m_state.m_board.getLADKoAvailable().m_white = false;
		} else if (vSplit[1] == "") {
			// B to live, W to kill
			m_state.m_board.getLADKoAvailable().m_black = false;
			m_state.m_board.getLADKoAvailable().m_white = true;
		}
	} else if (vSplit[4] == "UCD" || vSplit[4] == "LbK") {
		if (vSplit[0] == "") {
			// B to kill, W to live
			m_state.m_board.getLADKoAvailable().m_black = false;
			m_state.m_board.getLADKoAvailable().m_white = true;
		} else if (vSplit[1] == "") {
			// B to live, W to kill
			m_state.m_board.getLADKoAvailable().m_black = true;
			m_state.m_board.getLADKoAvailable().m_white = false;
		}
	}
}

string TMiniMCTS::getOneOpening()
{
	if (!m_fOpenings.is_open()) {
		m_fOpenings.open(WeichiConfigure::zero_selfplay_openings_file, ios::in);
	}

	char cLine[65536];
	if (m_fOpenings.getline(cLine, 65536)) { return ToString(cLine); }
	else { m_fOpenings.close(); return getOneOpening(); }
}

void TMiniMCTS::setLADBitBoard(string sSgfPos, WeichiBitBoard& bmStone)
{
	vector<string> vPoints = splitToVector(sSgfPos, ',');
	for (auto s : vPoints) {
		if (s.empty()) { continue; }
		WeichiMove m(COLOR_BLACK, s);
		bmStone.SetBitOn(m.getPosition());
	}
}

void TMiniMCTS::selection()
{
	TMiniNode* pNode = getRootNode();

	m_vPath.clear();
	m_vPath.push_back(pNode);
	while (pNode->hasChildren()) {
		pNode = selectChild(pNode);
		m_state.play(pNode->getMove());
		m_vPath.push_back(pNode);
	}
}

void TMiniMCTS::expandAndEvaluate(const DCNNResult& dcnnResult)
{
	if (m_state.m_board.hasTwoPass() || m_state.m_board.getMoveList().size() > 2 * WeichiConfigure::TotalGrids) { return; }
	if (WeichiConfigure::dcnn_use_ftl && (m_fKomi != m_vPath.size() - 1) && (m_fKomi - (m_vPath.size() - 1) < 0)) { return; }

	Color turnColor = m_state.m_board.getToPlay();
	Vector<CandidateEntry, MAX_NUM_GRIDS> vCandidates = dcnnResult.m_vProbability;

	// When training on 7x7 killall, remove pass for Black & remain only pass for White for the first 3 moves
	if (WeichiConfigure::zero_training_7x7_killall) {
		if (m_state.m_board.getMoveList().size() == 0 || m_state.m_board.getMoveList().size() == 2) {
			vCandidates = dcnnResult.getProbabilityWithoutPass();
		} else if (m_state.m_board.getMoveList().size() == 1) {
			vCandidates.clear();
			vCandidates.push_back(CandidateEntry(PASS_MOVE.getPosition(), 1.0f));
		}
	}

	uint size = vCandidates.size();
	TMiniNode* pFirstChild = allocateNewNodes(size);
	TMiniNode* pParent = m_vPath.back();
	TMiniNode* pChild = pFirstChild;
	for (uint i = 0; i < size; i++) {
		WeichiMove move(turnColor, vCandidates[i].getPosition());
		float fScore = static_cast<float>(vCandidates[i].getScore());

		pChild->reset(move);
		pChild->setProbability(fScore);
		pChild->setOriginalProbability(fScore);
		pChild++;
	}

	pParent->setNumChild(size);
	pParent->setFirstChild(pFirstChild);

	// add noise to root node
	if (m_bAddNoiseToRoot) {
		TMiniNode* pRootNode = getRootNode();
		if (pParent == pRootNode) {
			const float fEpsilon = WeichiConfigure::zero_noise_epsilon;
			vector<float> vDirichletNoise = calculateDirichletNoise(pRootNode->getNumChild(), WeichiConfigure::zero_noise_alpha);
			uint childNum = 0;
			for (TMiniNode *pChild = pRootNode->getFirstChild(); childNum < pRootNode->getNumChild(); ++pChild, ++childNum) {
				pChild->setProbability((1 - fEpsilon) * pChild->getProbability() + fEpsilon * vDirichletNoise[childNum]);
			}
		}
	}
}

void TMiniMCTS::update(const DCNNResult& dcnnResult)
{
	// decrease komi after playing some moves
	int komi_index = m_fKomi;
	if (komi_index != dcnnResult.m_vGLOutput.size() - 1) { komi_index -= (m_vPath.size() - 1); }

	float fValue;
	if (m_state.isTerminal() || komi_index < 0) {
		WeichiPlayoutResult result = m_state.eval();
		Color winner = result.getWinner();
		fValue = (winner == COLOR_NONE) ? 0.0f : ((winner == m_state.m_board.getToPlay()) ? 1.0f : -1.0f);
		//fValue = ((winner == m_state.m_board.getToPlay()) ? 1.0f : -1.0f);
	} else { fValue = dcnnResult.m_vValues[komi_index]; }

	for (int i = static_cast<int>(m_vPath.size()) - 1; i >= 0; i--) {
		TMiniNode* pNode = m_vPath[i];

		// since the first update node is parent node, we should reverse the value first
		fValue = -fValue;
		pNode->getUctData().add(fValue);

		// baseline value for all non-expand children
		if (i == static_cast<int>(m_vPath.size()) - 1) { pNode->setValue(fValue); }
	}
}

string TMiniMCTS::getGameSgf(float komi/* = WeichiConfigure::komi*/)
{
	string sBlackModel = getCNNNet(COLOR_BLACK)->getNetParam().m_sCaffeModel;
	string sWhiteModel = getCNNNet(COLOR_WHITE)->getNetParam().m_sCaffeModel;
	sBlackModel = sBlackModel.substr(sBlackModel.find_last_of("/") + 1);
	sWhiteModel = sWhiteModel.substr(sWhiteModel.find_last_of("/") + 1);

	ostringstream oss;
	WeichiCNNNet* cnnNet = getCNNNet(m_state.getRootTurn());
	bool bResign = (cnnNet->getVNLabelSize() == 1 && !cnnNet->hasBVOutput() && isResign());
	WeichiPlayoutResult result = getGameResult(bResign);
	oss << (isEnableResign() ? "" : ("Mini-winrate: " + ToString(m_minWinRate.get(result.getWinner()))) + "; ");
	oss << (isEnableResign() ? "" : "disable resign; ");
	oss << "Machine: " << WeichiConfigure::zero_selfplay_machine_name << "; ";
	oss << "LAD: " << m_LADSgfTags;
	
	SgfTag sgfTag;
	sgfTag.setSgfTag("KM", ToString(komi));
	sgfTag.setSgfTag("EV", oss.str());
	sgfTag.setSgfTag("DT", TimeSystem::getTimeString("Y/m/d_H:i:s.f"));
	sgfTag.setSgfTag("RE", result.toString());
	sgfTag.setSgfTag("PB", sBlackModel);
	sgfTag.setSgfTag("PW", sWhiteModel);

	return m_state.m_board.toSgfFileString(sgfTag, m_vMoveComments);
}

void TMiniMCTS::handle_simulationEnd()
{
	WeichiMove move = selectMCTSMove();

	// record min winrate
	const StatisticData& rootData = getRootNode()->getUctData();
	const StatisticData& bestData = m_pBest->getUctData();
	float fMinWinRate = (bestData.getMean() > -rootData.getMean() ? bestData.getMean() : -rootData.getMean());
	if (fMinWinRate < m_minWinRate.get(m_state.getRootTurn())) { m_minWinRate.get(m_state.getRootTurn()) = fMinWinRate; }

	WeichiCNNNet* cnnNet = getCNNNet(m_state.getRootTurn());
	bool bResign = (cnnNet->getVNLabelSize() == 1 && !cnnNet->hasBVOutput() && isResign());
	if (!bResign) {
		play(move);
		int move_number = m_state.m_board.getMoveList().size();
		addMoveComment(move_number, getMCTSSearchDistribution());
	}
	displayBoard(move);

	bool bEndGame = (bResign || isEndGame());
	if (bEndGame) { handle_endGame(); }
}

void TMiniMCTS::handle_endGame()
{
	boost::lock_guard<boost::mutex> lock(m_mutex);
	int total_moves = m_state.m_board.getMoveList().size();
	COUT() << "Self-play " << total_moves << " " << getGameSgf(m_fKomi) << endl;
	newGame();
}

void TMiniMCTS::randomPlayGame()
{
	while (!isEndGame()) {
		Vector<uint, MAX_NUM_GRIDS> vCandidates;
		WeichiBitBoard bmEmpty = ~m_state.m_board.getBitBoard() & StaticBoard::getMaskBorder();
		bmEmpty.bitScanAll(vCandidates);

		int index, size = vCandidates.size();
		Color turnColor = m_state.m_board.getToPlay();
		while (size > 0) {
			index = Random::nextInt(size);
			
			WeichiMove m(turnColor, vCandidates[index]);
			const WeichiGrid& grid = m_state.m_board.getGrid(m);
			if (!m_state.m_board.isIllegalMove(m, m_state.m_ht) && !grid.getPattern().getTrueEye(turnColor)) { break; }

			swap(vCandidates[index], vCandidates[size - 1]);
			--size;
		}

		int position = (size == 0) ? PASS_MOVE.getPosition() : vCandidates[index];
		if (WeichiConfigure::zero_training_7x7_killall && m_state.m_board.getMoveList().size() == 1) {
			// force White to play PASS on the second move in 7x7 killall
			position = PASS_MOVE.getPosition();
		}
		WeichiMove move(turnColor, position);
		play(move);
	}

	handle_endGame();
}

bool TMiniMCTS::isLearned(const DCNNResult& dcnnResult)
{
	if (m_simulation != 0) { return false; }

	set<int> sMark;
	const vector<WeichiMove>& vMarks = m_sgfLoader.getSgfNode()[0].m_vMark;
	if (vMarks.size() == 0) { return false; }
	for (int i = 0; i < vMarks.size(); ++i) { sMark.insert(vMarks[i].getPosition()); }

	float fSum = 0.0f;
	int rank = MAX_NUM_GRIDS;
	for (int i = 0; i < dcnnResult.m_vProbability.size(); ++i) {
		if (sMark.find(dcnnResult.m_vProbability[i].getPosition()) == sMark.end()) { continue; }
		fSum += dcnnResult.m_vProbability[i].getScore();
		rank = rank < i ? rank : i;
	}

	return rank <= 10 && Random::nextReal() < fSum;
}

void TMiniMCTS::displayBoard(const WeichiMove& move)
{
	if (!m_bDisplayBoard) { return; }

	string sBlackModel = getCNNNet(COLOR_BLACK)->getNetParam().m_sCaffeModel;
	string sWhiteModel = getCNNNet(COLOR_WHITE)->getNetParam().m_sCaffeModel;
	sBlackModel = sBlackModel.substr(sBlackModel.find_last_of("/") + 1);
	sWhiteModel = sWhiteModel.substr(sWhiteModel.find_last_of("/") + 1);

	m_state.m_board.showColorBoard();
	CERR() << "Black Model: " << sBlackModel << endl;
	CERR() << "White Model: " << sWhiteModel << endl;
	CERR() << "Komi: " << m_fKomi << endl;
	CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ")
		<< "[" << m_state.m_board.getMoveList().size() << "] "
		<< "Best Move: " << move.toGtpString() << " ";
	CERR() << m_pBest->getUctData().toString() << endl << endl;
}

DCNNResult TMiniMCTS::getDCNNResult()
{
	DCNNResult dcnnResult;
	WeichiCNNNet* cnnNet = getCNNNet(m_state.getRootTurn());
	
	dcnnResult.clear();
	dcnnResult.storeProbability(cnnNet->getSLCandidates(getBatchID()));
	dcnnResult.m_fValue = cnnNet->getVNResult(getBatchID(), m_fKomi);

	if (cnnNet->getVNLabelSize() > 1) {
		dcnnResult.m_vValues.clear();
		for(int i=0; i<cnnNet->getVNLabelSize(); ++i) {
			dcnnResult.m_vValues.push_back(cnnNet->getVNResult(getBatchID(), i));
		}
	}

	if (cnnNet->hasGLOutput()) {
		dcnnResult.m_vGLOutput = cnnNet->getGLResult(getBatchID());

		float fSum = 0.0f;
		for (int i = 0; i < dcnnResult.m_vGLOutput.size(); ++i) {
			fSum += dcnnResult.m_vGLOutput[i];
			dcnnResult.m_vValues[i] = (dcnnResult.m_vValues[i] + 1) / 2;
			dcnnResult.m_vValues[i] += (1 - fSum) / 2;
			dcnnResult.m_vValues[i] = dcnnResult.m_vValues[i] * 2 - 1;
		}
	}
	
	return dcnnResult;
}

string TMiniMCTS::getSearchDebugInformation()
{
	TMiniNode* pRoot = getRootNode();

	ostringstream debugLog;
	debugLog << "*@";
	debugLog << "Komi: " << m_fKomi << "@";
	debugLog << "Root: " << getRootNode()->getUctData().toString() << "@";
	debugLog << "Best: " << getBestNode()->getUctData().toString() << "@";
	debugLog << "Sequence: ";
	TMiniNode* pNode = pRoot;
	while (pNode->hasChildren()) {
		int childNum = 0;
		TMiniNode* pBest = pNode->getFirstChild();
		for (TMiniNode *child = pNode->getFirstChild(); childNum < pNode->getNumChild(); ++child, ++childNum) {
			if (child->getUctData().getCount() <= pBest->getUctData().getCount()) { continue; }
			pBest = child;
		}
		pNode = pBest;
		debugLog << pBest->getMove().toGtpString() << "(" << pBest->getUctData().toString() << ")";
		if (pNode->hasChildren()) { debugLog << " => "; }
	}
	debugLog << "@";

	debugLog << "move rank:  action    Q     U     P   P-Dir    N  soft-N@";
	uint childNum = 0;
	uint nParentSimulation = static_cast<uint>(pRoot->getUctData().getCount());
	for (TMiniNode *pChild = pRoot->getFirstChild(); childNum < pRoot->getNumChild(); ++pChild, ++childNum) {
		if (pChild->getUctData().getCount() == 0 && childNum >= 3) { continue; }

		const StatisticData& childUctData = pChild->getUctData();
		double dValuePa = pChild->getProbability();
		double dPUCTBias = log((1 + nParentSimulation + 19652) / 19652) + WeichiConfigure::mcts_puct_bias;
		double dValueU = dPUCTBias * dValuePa * sqrt(nParentSimulation) / (1 + childUctData.getCount());
		double dValueQ = (childUctData.getCount() > 0) ? childUctData.getMean() : pNode->getAdjustChildVNValue();
		double move_score = dValueQ + dValueU;

		debugLog << left << setw(4) << pChild->getMove().toGtpString() << " "
			<< right << setw(4) << (childNum + 1) << ": "
			<< fixed << setprecision(3) << setw(7) << move_score << " "
			<< setw(6) << dValueQ << " "
			<< setw(5) << dValueU << " "
			<< setw(5) << pChild->getOriginalProbability() << " "
			<< setw(5) << dValuePa << " ";
		debugLog.unsetf(ios::fixed);
		debugLog << setw(5) << childUctData.getCount() << " "
			<< fixed << setprecision(3) << setw(5) << childUctData.getCount() / (nParentSimulation - 1)
			<< (pChild == m_pBest ? " *" : "") << "@";
	}

	return debugLog.str();
}

TMiniNode* TMiniMCTS::selectChild(TMiniNode* pNode)
{
	double dBestScore = -DBL_MAX;
	TMiniNode* pBest = nullptr;

	uint childNum = 0;
	float fSumOfChildWins = 0.0f;
	float fSumOfChildSims = 0.0f;
	uint nParentSimulation = static_cast<uint>(pNode->getUctData().getCount());
	for (TMiniNode *child = pNode->getFirstChild(); childNum < pNode->getNumChild(); ++child, ++childNum) {
		const StatisticData& childUctData = child->getUctData();
		double dValuePa = child->getProbability();
		double dPUCTBias = log((1 + nParentSimulation + 19652) / 19652) + WeichiConfigure::mcts_puct_bias;
		double dValueU = dPUCTBias * dValuePa * sqrt(nParentSimulation) / (1 + childUctData.getCount());
		double dValueQ = (childUctData.getCount() > 0) ? childUctData.getMean() : pNode->getAdjustChildVNValue();

		if (child->hasChildren()) {
			fSumOfChildWins += child->getUctData().getMean();
			fSumOfChildSims += 1;
		}

		double move_score = dValueQ + dValueU;
		if (move_score <= dBestScore) { continue; }

		dBestScore = move_score;
		pBest = child;
	}

	float fAdjustChildVNValue = fSumOfChildWins / (fSumOfChildSims + 1);
	pNode->setAdjustChildVNValue(fAdjustChildVNValue);

	return pBest;
}

void TMiniMCTS::adjustKomi(const DCNNResult& dcnnResult)
{
	// only adjust komi at the first time for each move
	if (m_simulation != 0) { return; }

	if (WeichiConfigure::dcnn_use_ftl) {
		const vector<float>& vGL = dcnnResult.m_vGLOutput;
		const float fThreshold = 0.6f + Random::nextReal(0.2);
		float fSum = 0.0f;
		m_fKomi = vGL.size() - 1;
		for (uint i = 0; i < vGL.size(); ++i) {
			fSum += vGL[i];
			if (fSum < fThreshold) { continue; }
			m_fKomi = i;
			break;
		}
		/*int max_size = dcnnResult.m_vValues.size() - 1;
		m_fKomi = max_size - 1;	// the last one is the winrate for the game without considering move number
		for (int i = 0; i < max_size; ++i) {
			float fScore = dcnnResult.m_vValues[i];
			if (fScore <= 0) { continue; }
			m_fKomi = i;
		}*/
	} else {
		int min_index = 0, max_index = dcnnResult.m_vValues.size() - 1;
		if (m_state.getRootTurn() == COLOR_BLACK) {
			m_fKomi = min_index;
			for (int i = max_index; i >= min_index; --i) {
				float fScore = dcnnResult.m_vValues[i];
				if (fScore < 0) { continue; }
				m_fKomi = i;
				break;
			}
		} else if (m_state.getRootTurn() == COLOR_WHITE) {
			m_fKomi = max_index;
			for (int i = min_index; i <= max_index; ++i) {
				float fScore = dcnnResult.m_vValues[i];
				if (fScore < 0) { continue; }
				m_fKomi = i;
				break;
			}
		}
	}
}

vector<float> TMiniMCTS::calculateDirichletNoise(int size, float fAlpha)
{
	boost::random::gamma_distribution<> pdf(fAlpha);
	boost::variate_generator<Random::IntegerGenerator&, boost::gamma_distribution<> >
		generator(*Random::integer_generator, pdf);

	vector<float> vDirichlet;
	for (int i = 0; i < size; i++) { vDirichlet.push_back(static_cast<float>(generator())); }
	float fSum = accumulate(vDirichlet.begin(), vDirichlet.end(), 0.0f);
	if (fSum < boost::numeric::bounds<float>::smallest()) { return vDirichlet; }
	for (int i = 0; i < vDirichlet.size(); i++) { vDirichlet[i] /= fSum; }

	return vDirichlet;
} 
