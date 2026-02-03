#include "ZeroOptimization.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>

// ZeroOptimizationSlave (public function)
void ZeroOptimizationSlave::doSlaveJob()
{
	while (!m_sharedData.m_trainNet->isFull()) {
		calculateFeatures();
	}
}

// ZeroOptimizationSlave (private function)
void ZeroOptimizationSlave::initialize()
{
	m_label = new float[m_sharedData.m_trainNet->getLabelSize()];

	TBaseSlave::initialize();
}

void ZeroOptimizationSlave::calculateFeatures()
{
	// random a position from all training positions
	int min_index = 0;
	int max_index = m_sharedData.m_selfplayGameRecords.back().second;
	int rand_moves = Random::nextInt(min_index, max_index);
	int min_gameIndex = -1, max_gameIndex = m_sharedData.m_selfplayGameRecords.size(), gameIndex;
	while (true) {
		gameIndex = (min_gameIndex + max_gameIndex) / 2;
		gameIndex = (gameIndex >= m_sharedData.m_selfplayGameRecords.size() ? m_sharedData.m_selfplayGameRecords.size() : gameIndex);

		int upperBound = m_sharedData.m_selfplayGameRecords[gameIndex].second;
		int lowerBound = (gameIndex - 1 < min_index ? min_index : m_sharedData.m_selfplayGameRecords[gameIndex - 1].second);

		if (rand_moves < lowerBound) { max_gameIndex = gameIndex; }
		else if (rand_moves >= upperBound) { min_gameIndex = gameIndex; }
		else if (rand_moves >= lowerBound && rand_moves < upperBound) {
			rand_moves = rand_moves - lowerBound;
			break;
		}
	}

	SgfLoader sgfLoader;
	string sSgfString = m_sharedData.m_selfplayGameRecords[gameIndex].first;
	if (!sgfLoader.parseFromString(sSgfString)) { return; }

	// random a position and play
	const Color winner = sgfLoader.getWinner();
	const vector<WeichiMove>& vPresets = sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = sgfLoader.getPlayMove();
	m_state.resetThreadState();
	for (int i = 0; i < vPresets.size(); i++) { m_state.m_board.preset(vPresets[i]); }
	for (int i = 0; i < rand_moves; i++) { m_state.play(vMoves[i]); }
	setLADSgfTags(sgfLoader);

	// calculate features
	WeichiCNNTrainNet* cnnTrainNet = m_sharedData.m_trainNet;
	WeichiBitBoard bmLegal;
	m_state.m_rootFilter.startFilter();
	float features[MAX_CNN_CHANNEL_SIZE*MAX_NUM_GRIDS];
	WeichiCNNFeatureGenerator::calculateDCNNFeatures(m_state, cnnTrainNet->getCNNFeatureType(), features, bmLegal);

	//---------------------------label---------------------------//
	float* fLabel = m_label;
	Color turnColor = vMoves[rand_moves].getColor();

	// SL
	if (cnnTrainNet->hasSLOutput()) {
		const WeichiMove& move = vMoves[rand_moves];
		*fLabel = static_cast<float>(WeichiMove::toCompactPosition(move.getPosition()));
		fLabel++;
		if (cnnTrainNet->hasSLDistribution()) {
			string sMCTSDistribution = sgfLoader.getSgfNode()[rand_moves + 1].m_comment;
			if (sMCTSDistribution.find("*") != string::npos) { sMCTSDistribution = sMCTSDistribution.substr(0, sMCTSDistribution.find("*")); }
			map<uint, float> mDistribution = calculateMCTSSearchDistribution(sMCTSDistribution, move);
			for (uint pos = 0; pos < WeichiConfigure::TotalGrids + 1; pos++) {
				if (mDistribution.find(pos) != mDistribution.end()) { *fLabel = mDistribution[pos]; }
				else { *fLabel = 0.0f; }
				fLabel++;
			}
		}
	}

	// VN
	if (cnnTrainNet->hasVNOutput()) {
		int label_size = cnnTrainNet->getVNLabelSize();
		if (label_size == 1) {
			*fLabel = ((winner == COLOR_NONE) ? 0.0f : ((winner == turnColor) ? 1.0f : -1.0f));
		} else {
			if (WeichiConfigure::dcnn_use_ftl) {
				int max_moves = sgfLoader.getPlayMove().size() - 1;
				for (int i = 0; i < label_size - 1; ++i) {
					if (rand_moves + i < max_moves) { *fLabel = -1.0f; }
					//else { *fLabel = ((winner == COLOR_NONE) ? 0.0f : ((winner == turnColor) ? 1.0f : -1.0f)); }
					else { *fLabel = ((winner == turnColor) ? 1.0f : -1.0f); }
					fLabel++;
				}
				// the result for this game (without considering move number)
				*fLabel = ((winner == turnColor) ? 1.0f : -1.0f);
				//*fLabel = ((winner == COLOR_NONE) ? 0.0f : ((winner == turnColor) ? 1.0f : -1.0f));
				fLabel++;
			} else {
				string sResultInfo = sgfLoader.getSgfTag("RE");
				float fBLeadCount = (sResultInfo == "0" ? 0 : static_cast<float>(atof(sResultInfo.substr(2).c_str())));
				fBLeadCount = (winner == COLOR_BLACK) ? fBLeadCount + sgfLoader.getKomi() : -fBLeadCount + sgfLoader.getKomi();

				int label_size = cnnTrainNet->getVNLabelSize();
				float min_komi = WeichiConfigure::komi - (label_size / 2);
				for (int i = 0; i < label_size; i++) {
					float fResult = (fBLeadCount - (min_komi + i));

					Color newWinner = COLOR_NONE;
					if (fResult > 0) { newWinner = COLOR_BLACK; }
					else if (fResult < 0) { newWinner = COLOR_WHITE; }

					*fLabel = ((newWinner == COLOR_NONE) ? 0.0f : ((newWinner == turnColor) ? 1.0f : -1.0f));
					fLabel++;
				}
			}
		}
	}

	// BV
	if (cnnTrainNet->hasBVOutput()) {
		for (int i = rand_moves; i < vMoves.size(); i++) { m_state.play(vMoves[i]); }
		
		m_state.eval();
		const Vector<Color, MAX_NUM_GRIDS>& vTerritory = m_state.m_board.getTerritory();
		for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
			*fLabel = ((vTerritory[*it] == COLOR_NONE) ? 0.5f : ((vTerritory[*it] == turnColor) ? 1.0f : 0.0f));
			fLabel++;
		}
	}

	// GL
	if (cnnTrainNet->hasGLOutput()) {
		int max_moves = sgfLoader.getPlayMove().size() - 1;
		int vn_label_size = cnnTrainNet->getVNLabelSize();
		//*fLabel = floor((max_moves - rand_moves) / 2);
		*fLabel = max_moves - rand_moves;
		*fLabel = (*fLabel >= cnnTrainNet->getVNLabelSize()) ? cnnTrainNet->getVNLabelSize() - 1 : *fLabel;
		fLabel++;
	}
	//---------------------------label---------------------------//

	// store features & log info
	boost::lock_guard<boost::mutex> lock(m_sharedData.m_mutex);
	if (cnnTrainNet->isFull()) { return; }
	cnnTrainNet->push_back(m_sharedData.m_bGPUEvaluateFirstBatch, features, turnColor, m_label, static_cast<float>(rand_moves));
}

void ZeroOptimizationSlave::setLADSgfTags(const SgfLoader& sgfLoader)
{
	// set LAD info for feature
	string sLADTags = sgfLoader.getSgfTag("EV").substr(sgfLoader.getSgfTag("EV").find("LAD: ") + 5);
	vector<string> vSplit;
	vSplit.push_back("");
	for (auto c : sLADTags) {
		if (c == ';') { vSplit.push_back(""); }
		else { vSplit.back().append(1, c); }
	}

	// format: BL ; BK ; WL ; WK
	if (vSplit.size() != 5) { CERR() << "LAD format error!! " << sLADTags << endl; }
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

void ZeroOptimizationSlave::setLADBitBoard(string sSgfPos, WeichiBitBoard & bmStone)
{
	vector<string> vPoints = splitToVector(sSgfPos, ',');
	for (auto s : vPoints) {
		if (s.empty()) { continue; }
		WeichiMove m(COLOR_BLACK, s);
		bmStone.SetBitOn(m.getPosition());
	}
}

map<uint, float> ZeroOptimizationSlave::calculateMCTSSearchDistribution(string sMCTSSearchDistribution, const WeichiMove& selectMove)
{
	map<uint, float> mDistribution;

	if (sMCTSSearchDistribution != "") {
		vector<string> vMoveCount;
		boost::split(vMoveCount, sMCTSSearchDistribution, boost::is_any_of(","), boost::token_compress_on);
		float total_count = 0;
		for (uint i = 0; i < vMoveCount.size(); i++) {
			if (vMoveCount[i].find(":") == string::npos) { continue; }

			int split_index = static_cast<int>(vMoveCount[i].find(":"));
			uint pos = atoi(vMoveCount[i].substr(0, split_index).c_str());
			float count = atof(vMoveCount[i].substr(split_index + 1).c_str());
			mDistribution[pos] = count;
			total_count += count;
		}

		// normalization
		for (map<uint, float>::iterator it = mDistribution.begin(); it != mDistribution.end(); ++it) { it->second = it->second * 1.0f / total_count; }
	} else { mDistribution[WeichiMove::toCompactPosition(selectMove.getPosition())] = 1.0f; }

	return mDistribution;
}

// ZeroOptimizationMaster (public function)
void ZeroOptimizationMaster::run()
{
	initialize();
	CERR() << "Optimization initialize over." << endl;

	runOneOptimization();
	string sFileName = WeichiConfigure::zero_optimization_save_name_prefix + "iter_" + ToString(m_sharedData.m_trainNet->getIteration()) + ".pb";
	CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "Optimization " << sFileName << endl;
	COUT() << "Optimization " << sFileName << endl;

	/*//while (true) {
		runOneOptimization();
		if (!WeichiConfigure::zero_optimization_quiet) {
			CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "Optimization Job_Done" << endl;
			COUT() << "Optimization Job_Done" << endl;
		}
		/ *if (m_bHasCommand) {
			CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "Receive Command: " << m_command << endl;
			if (m_command.find("ReadSgf") != string::npos) {
				WeichiConfigure::zero_optimization_sgf_max_iteration = atoi(m_command.substr(m_command.find(" ") + 1).c_str());
				loadSgf();
				CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "Optimization ReadSgf_Done" << endl;
				COUT() << "Optimization ReadSgf_Done" << endl;
			} else if (m_command == "Optimization") {
				runOneOptimization();
				if (!WeichiConfigure::zero_optimization_quiet) {
					CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "Optimization Job_Done" << endl;
					COUT() << "Optimization Job_Done" << endl;
				}
			}
			m_bHasCommand = false;
		} else {
			boost::this_thread::sleep(boost::posix_time::seconds(1));
		}* /
	//}*/
}

bool ZeroOptimizationMaster::initialize()
{
	CNNNetParam param(WeichiConfigure::zero_optimization_net);
	m_sharedData.m_trainNet = new WeichiCNNTrainNet(param, WeichiConfigure::zero_optimization_solver);
	m_sharedData.m_trainNet->init(false);
	CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << m_sharedData.m_trainNet->getNetInfomation();
	
	m_optimizaionIteration = WeichiConfigure::zero_optimization_sgf_max_iteration - WeichiConfigure::zero_replay_buffer_size + 1;
	m_optimizaionIteration = (m_optimizaionIteration >= 1) ? m_optimizaionIteration : 1;
	CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "Optimization Iteration: " << m_optimizaionIteration << endl;

	loadSgf();
	TBaseMaster::initialize();

	// run another thread to read standard input
	m_input_thread = boost::thread(boost::bind(&ZeroOptimizationMaster::readInput, this));

	return true;
}

void ZeroOptimizationMaster::runOneOptimization()
{
	if (m_sharedData.m_selfplayGameRecords.size() == 0 || m_sharedData.m_selfplayGameRecords.back().second == 0) { return; }

	//m_sharedData.m_trainNet->setBaseLearningRate(WeichiConfigure::zero_optimization_learning_rate);
	m_sharedData.m_bGPUEvaluateFirstBatch = false;
	for (int i = 0; i < m_nThread; i++) { m_vSlaves[i]->startRun(); }
	for (int i = 0; i < m_nThread; i++) { m_vSlaves[i]->finishRun(); }

	int train_net_batch_size = m_sharedData.m_trainNet->getBatchSize() * WeichiConfigure::dcnn_train_gpu_list.length();
	const int repeat_times = 3;
	int nMaxIteration = m_sharedData.m_selfplayGameRecords.back().second * repeat_times / (WeichiConfigure::zero_replay_buffer_size * train_net_batch_size);
	for (int iteration = 0; iteration < nMaxIteration; iteration++) {
		m_sharedData.m_bGPUEvaluateFirstBatch = !m_sharedData.m_bGPUEvaluateFirstBatch;
		m_sharedData.m_trainNet->resetCurrentBatchSize();
		for (int i = 0; i < m_nThread; i++) { m_vSlaves[i]->startRun(); }
		m_sharedData.m_trainNet->update(m_sharedData.m_bGPUEvaluateFirstBatch);
		for (int i = 0; i < m_nThread; i++) { m_vSlaves[i]->finishRun(); }

		if (m_sharedData.m_trainNet->getIteration() % WeichiConfigure::zero_optimization_snapshot_iter == 0) {
			CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << m_sharedData.m_trainNet->getTrainingInfo() << endl;
		}
	}

	if (nMaxIteration > 0) { CERR() << m_sharedData.m_trainNet->getTrainingInfo() << endl; }

	// always save the latest model
	//string sFileName = WeichiConfigure::zero_optimization_save_name_prefix + "iter_" + ToString(m_sharedData.m_trainNet->getIteration()) + ".pb";
	string sFileName = WeichiConfigure::zero_optimization_save_name_prefix + "iter_" + ToString(m_sharedData.m_trainNet->getIteration()) + ".caffemodel";
	m_sharedData.m_trainNet->saveNetWork(WeichiConfigure::zero_train_directory + "/model/" + sFileName);
}

void ZeroOptimizationMaster::readInput()
{
	while (true) {
		if (!m_bHasCommand) {
			getline(cin, m_command);
			m_bHasCommand = true;
		}
	}
}

void ZeroOptimizationMaster::loadSgf()
{
	if (WeichiConfigure::zero_train_directory == "") { return; }

	for (; m_optimizaionIteration <= WeichiConfigure::zero_optimization_sgf_max_iteration; m_optimizaionIteration++) {
		string sFileName = WeichiConfigure::zero_train_directory + "/sgf/" + ToString(m_optimizaionIteration) + ".sgf";
		loadSgf(sFileName);
	}
}

void ZeroOptimizationMaster::loadSgf(string sFileName)
{
	CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "load " << sFileName << endl;

	int nGame = 0, total_position = 0;
	string sLine;
	string sWorkerName = sFileName;
	fstream fin(sFileName, ios::in);
	while (getline(fin, sLine)) {
		std::vector<std::string> vArgs;
		boost::split(vArgs, sLine, boost::is_any_of(" "), boost::token_compress_on);

		int move_number = atoi(vArgs[1].c_str());
		string sSgfString = sLine.substr(sLine.find(vArgs[1]) + vArgs[1].length() + 1);
		int accumulate_position = (m_sharedData.m_selfplayGameRecords.size() > 0 ? m_sharedData.m_selfplayGameRecords.back().second : 0) + move_number;
		m_sharedData.m_selfplayGameRecords.push_back(make_pair(sSgfString, accumulate_position));
		total_position += move_number;
		nGame++;
	}

	m_sharedData.m_selfplayGameIndex.push_back(nGame);

	shiftWindow();
	CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "Total read " << nGame << " games, " << total_position << " positions." << endl;
	CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "Total has " << m_sharedData.m_selfplayGameRecords.size() << " games, "
		<< (m_sharedData.m_selfplayGameRecords.size() > 0 ? m_sharedData.m_selfplayGameRecords.back().second : 0) << " positions." << endl;
}

void ZeroOptimizationMaster::shiftWindow()
{
	// shift window
	if (m_sharedData.m_selfplayGameIndex.size() <= WeichiConfigure::zero_replay_buffer_size) { return; }
	int iRemoveSize = m_sharedData.m_selfplayGameIndex.front();
	int minPosition = (m_sharedData.m_selfplayGameRecords.begin() + iRemoveSize - 1)->second;

	m_sharedData.m_selfplayGameRecords.erase(m_sharedData.m_selfplayGameRecords.begin(), m_sharedData.m_selfplayGameRecords.begin() + iRemoveSize);
	m_sharedData.m_selfplayGameIndex.pop_front();
	for (int i = 0; i < m_sharedData.m_selfplayGameRecords.size(); i++) {
		m_sharedData.m_selfplayGameRecords[i].second -= minPosition;
	}
} 
